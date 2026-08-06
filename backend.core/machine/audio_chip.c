// Copyright 2016-2024 Timo Kloss
// Copyright 2021-2026 Martin Mauchauffée

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "audio_chip.h"
#include "core.h"
#include <math.h>
#include <string.h>

const double envRates[16] =
{
	256.0 / 0.002,
	256.0 / 0.03,
	256.0 / 0.06,
	256.0 / 0.09,
	256.0 / 0.14,
	256.0 / 0.21,
	256.0 / 0.31,
	256.0 / 0.47,
	256.0 / 0.70,
	256.0 / 1.0,
	256.0 / 1.6,
	256.0 / 2.4,
	256.0 / 3.5,
	256.0 / 5.0,
	256.0 / 8.0,
	256.0 / 12.0
};

static const double lfoRates[16] = {
	0.12 * 256.0,
	0.16 * 256.0,
	0.23 * 256.0,
	0.32 * 256.0,
	0.44 * 256.0,
	0.62 * 256.0,
	0.87 * 256.0,
	1.2 * 256.0,
	1.7 * 256.0,
	2.4 * 256.0,
	3.3 * 256.0,
	4.7 * 256.0,
	6.6 * 256.0,
	9.2 * 256.0,
	12.9 * 256.0,
	18.0 * 256.0
};

static const int lfoAmounts[16] = {
	0,
	1,
	2,
	4,
	6,
	9,
	12,
	17,
	24,
	34,
	48,
	67,
	93,
	131,
	183,
	256
};

static const int pulseWidths[16] = {
	239,
	237,
	234,
	231,
	227,
	223,
	218,
	213,
	207,
	199,
	191,
	181,
	171,
	158,
	144,
	127
};

static const int triangleRises[16] = {
	16,
	22,
	29,
	39,
	53,
	71,
	95,
	128,
	128,
	161,
	185,
	203,
	217,
	227,
	234,
	240
};

// sauce: Claude Opus 5
// Spectral tilt of the noise waveform, indexed by the same pulse width nibble.
// 128 is flat, below is a lowpass, above a highpass, at 16 units per octave.
static const int noiseTilts[16] = {
	16,
	32,
	48,
	64,
	80,
	96,
	112,
	128,
	142,
	156,
	170,
	184,
	198,
	212,
	226,
	240
};

// sauce: Claude Opus 5
#define NOISE_TILT_NEUTRAL 128
#define NOISE_TILT_MIN 16
#define NOISE_TILT_MAX 240
#define NOISE_TILT_LP_REF 16000.0f // lowpass corner in Hz at the neutral tilt
#define NOISE_TILT_HP_REF 80.0f    // highpass corner in Hz at the neutral tilt
#define NOISE_TILT_OCTAVE 16.0f    // tilt units per octave of corner frequency
#define NOISE_TILT_HP_CLOCK 0.25f  // highpass corner stays below this * LFSR clock
#define NOISE_TILT_TARGET 0.55f    // output RMS relative to the raw LFSR RMS
#define NOISE_TILT_MAX_GAIN 3.0f   // ceiling on the makeup gain

// sauce: Claude Opus 5
// The raw LFSR output is uniform (crest factor sqrt(3)) but anything filtered out
// of it is gaussian (crest factor ~3.15), so at a common peak ceiling the filtered
// signal can only carry 0.55 of the RMS. NOISE_TILT_TARGET applies to the neutral
// tilt as well, which costs 5.2 dB but keeps every width equally loud, so the LFO
// can sweep across the centre without a step.
static void audio_updateNoiseTilt(struct VoiceInternals *voiceIn, int tilt, int freq, int outputFrequency)
{
	float fs = (float)outputFrequency;
	float rho = (float)freq / fs;
	float fc = (tilt < NOISE_TILT_NEUTRAL ? NOISE_TILT_LP_REF : NOISE_TILT_HP_REF)
		* exp2f((float)(tilt - NOISE_TILT_NEUTRAL) / NOISE_TILT_OCTAVE);

	if(tilt > NOISE_TILT_NEUTRAL)
	{
		// a highpass reaching far past the LFSR clock leaves a train of decaying
		// spikes whose crest factor no makeup gain can be normalised against
		if(fc > 0.25f * fs)
			fc = 0.25f * fs;
		if(fc > NOISE_TILT_HP_CLOCK * (float)freq)
			fc = NOISE_TILT_HP_CLOCK * (float)freq;
	}

	float k = 1.0f - expf(-6.2831853f * fc / fs);
	if(k > 1.0f)
		k = 1.0f;
	if(k < 1.0e-6f)
		k = 1.0e-6f;

	// The LFSR output is an AR(1) process with a = 0.5 at its own clock rate, but
	// it is sample-and-held at the output rate, so the correlation the filter
	// actually sees depends on the note being played. Assuming a flat 0.5 here
	// overestimates the makeup gain by up to 15 dB at low pitch.
	float a = (rho < 1.0f) ? (1.0f - 0.5f * rho) : exp2f(-rho);
	float p = 1.0f - k;
	float ompa = (1.0f - a) + k * a; // 1 - p*a, grouped to avoid cancellation in float
	float den = (2.0f - k) * ompa;
	float v = (tilt < NOISE_TILT_NEUTRAL)
		? k * (2.0f - ompa) / den    // Var(lowpass) / Var(in)
		: 2.0f * (1.0f - a) * p * p / den; // Var(highpass) / Var(in)
	if(v < 1.0e-9f)
		v = 1.0e-9f;

	float gain = NOISE_TILT_TARGET / sqrtf(v);
	if(gain > NOISE_TILT_MAX_GAIN)
		gain = NOISE_TILT_MAX_GAIN;

	voiceIn->noiseTiltK = k;
	voiceIn->noiseTiltGain = gain;
	voiceIn->noiseTiltIndex = tilt;
	voiceIn->noiseTiltFreq = freq;
}

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency, int volume);

void audio_reset(struct Core *core)
{
	struct AudioInternals *internals = &core->machineInternals->audioInternals;

	for(int i = 0; i < NUM_VOICES; i++)
	{
		struct VoiceInternals *voiceIn = &internals->voices[i];
		voiceIn->noiseRandom = 0xABCD;
		voiceIn->lfoRandom = 0xABCD;
		// sauce: Claude Opus 5
		voiceIn->noiseTiltState = 0.0f;
		// 0 is never a legal tilt, so this forces a recompute before first use
		voiceIn->noiseTiltIndex = 0;
		voiceIn->noiseTiltFreq = 0;
	}
	internals->writeBufferIndex = -1;
}

void audio_bufferRegisters(struct Core *core)
{
	struct AudioRegisters *registers = &core->machine->audioRegisters;
	struct AudioInternals *internals = &core->machineInternals->audioInternals;

	// next buffer
	int writeBufferIndex = internals->writeBufferIndex;
	if(writeBufferIndex >= 0)
	{
		writeBufferIndex = (writeBufferIndex + 1) % NUM_AUDIO_BUFFERS;
	}
	else
	{
		writeBufferIndex = NUM_AUDIO_BUFFERS / 2;
	}

	// copy registers to buffer
	memcpy(&internals->buffers[writeBufferIndex], registers, sizeof(struct AudioRegisters));

	// reset "init" flags
	for(int v = 0; v < NUM_VOICES; v++)
	{
		struct Voice *voice = &registers->voices[v];
		voice->status.init = 0;
	}

	internals->writeBufferIndex = writeBufferIndex;
}

void audio_renderAudio(struct Core *core, int16_t *stereoOutput, int numSamples, int outputFrequency, int volume)
{
	struct AudioInternals *internals = &core->machineInternals->audioInternals;
	struct AudioRegisters *lifeRegisters = &core->machine->audioRegisters;

	int numSamplesPerUpdate = outputFrequency / 60 * NUM_CHANNELS;
	int offset = 0;

	while(offset < numSamples)
	{
		if(offset + numSamplesPerUpdate > numSamples)
		{
			numSamplesPerUpdate = numSamples - offset;
		}
		int readBufferIndex = internals->readBufferIndex;
		audio_renderAudioBuffer(lifeRegisters,
			&internals->buffers[readBufferIndex],
			internals,
			&stereoOutput[offset],
			numSamplesPerUpdate,
			outputFrequency,
			volume);
		if(internals->writeBufferIndex != -1 && internals->writeBufferIndex != readBufferIndex)
		{
			internals->readBufferIndex = (readBufferIndex + 1) % NUM_AUDIO_BUFFERS;
		}

		offset += numSamplesPerUpdate;
	}
}

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency, int volume)
{
	double overflow = 0x1000000;

	for(int v = 0; v < NUM_VOICES; v++)
	{
		struct Voice *voice = &registers->voices[v];
		if(voice->status.init)
		{
			voice->status.init = 0;

			struct VoiceInternals *voiceIn = &internals->voices[v];
			voiceIn->envState = EnvStateAttack;
			voiceIn->lfoHold = false;
			voiceIn->timeoutCounter = voice->length;
			if(voice->lfoAttr.envMode || voice->lfoAttr.trigger)
			{
				voiceIn->lfoAccumulator = 0.0;
			}
		}
	}

	int i = 0;
	while(i < numSamples)
	{
		int16_t leftOutput = 0;
		int16_t rightOutput = 0;

		if(internals->audioEnabled)
		{
			for(int v = 0; v < NUM_VOICES; v++)
			{
				struct Voice *voice = &registers->voices[v];
				struct VoiceInternals *voiceIn = &internals->voices[v];

				int freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
				if(freq == 0)
					continue;

				int volume = voice->status.volume << 4;
				int pulseWidth = pulseWidths[voice->attr.pulseWidth];
				int triangleRise = triangleRises[voice->attr.pulseWidth];
				enum WaveType waveType = voice->attr.wave;

				// --- LFO ---

				uint8_t lfoAccu8Last = voiceIn->lfoAccumulator;
				if(!voiceIn->lfoHold)
				{
					double lfoRate = lfoRates[voice->lfoFrequency];
					double lfoAccumulator = voiceIn->lfoAccumulator + lfoRate / (double)outputFrequency;
					if(voice->lfoAttr.envMode && lfoAccumulator >= 255.0)
					{
						lfoAccumulator = 255.0;
						voiceIn->lfoHold = true;
					}
					else if(lfoAccumulator >= 256.0)
					{
						// avoid overflow and loss of precision
						lfoAccumulator -= 256.0;
					}
					voiceIn->lfoAccumulator = lfoAccumulator;
				}
				uint8_t lfoAccu8 = voiceIn->lfoAccumulator;
				uint8_t lfoSample = 0;

				enum LFOWaveType lfoWaveType = voice->lfoAttr.wave;
				switch(lfoWaveType)
				{
				case LFOWaveTypeTriangle: {
					lfoSample = ((lfoAccu8 & 0x80) ? ~(lfoAccu8 << 1) : (lfoAccu8 << 1));
					break;
				}
				case LFOWaveTypeSawtooth: {
					lfoSample = ~lfoAccu8;
					break;
				}
				case LFOWaveTypeSquare: {
					lfoSample = (lfoAccu8 & 0x80) ? 0x00 : 0xFF;
					break;
				}
				case LFOWaveTypeRandom: {
					if((lfoAccu8 & 0x80) != (lfoAccu8Last & 0x80))
					{
						uint16_t r = voiceIn->lfoRandom;
						uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5)) & 1;
						voiceIn->lfoRandom = (r >> 1) | (bit << 15);
					}
					lfoSample = voiceIn->lfoRandom & 0xFF;
					break;
				}
				}

				int freqAmount = lfoAmounts[voice->lfoOscAmount];
				int volAmount = voice->lfoVolAmount;
				int pwAmount = voice->lfoPWAmount;

				int freqMod = freq * lfoSample * freqAmount >> 16;
				if(voice->lfoAttr.invert)
					freq -= freqMod;
				else
					freq += freqMod;
				if(freq < 1)
					freq = 1;
				if(freq > 65535)
					freq = 65535;

				if(voice->lfoAttr.invert)
				{
					volume -= volume * lfoSample * volAmount >> 12;
				}
				else
				{
					volume -= volume * (~lfoSample & 0xFF) * volAmount >> 12;
				}
				if(volume < 0)
					volume = 0;
				if(volume > 255)
					volume = 255;

				int pwMod = lfoSample * pwAmount >> 4;
				if(voice->lfoAttr.invert)
				{
					pulseWidth -= pwMod;
					triangleRise -= pwMod;
				}
				else
				{
					pulseWidth += pwMod;
					triangleRise += pwMod;
				}
				if(pulseWidth < 16)
					pulseWidth = 16;
				if(pulseWidth > 239)
					pulseWidth = 239;
				if(triangleRise < 16)
					triangleRise = 16;
				if(triangleRise > 240)
					triangleRise = 240;

				// sauce: Claude Opus 5
				// the noise tilt shares the width nibble and the same modulation,
				// so an LFO sweeps it at the full 256 step resolution
				int noiseTilt = NOISE_TILT_NEUTRAL;
				if(waveType == WaveTypeNoise)
				{
					noiseTilt = noiseTilts[voice->attr.pulseWidth];
					if(voice->lfoAttr.invert)
						noiseTilt -= pwMod;
					else
						noiseTilt += pwMod;
					if(noiseTilt < NOISE_TILT_MIN)
						noiseTilt = NOISE_TILT_MIN;
					if(noiseTilt > NOISE_TILT_MAX)
						noiseTilt = NOISE_TILT_MAX;
				}

				//  if (i == 0 && v == 0) printf("pulseWidth %d\n", pulseWidth);

				// --- WAVEFORM GENERATOR ---

				uint16_t accu16Last = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;
				double accumulator = voiceIn->accumulator + (double)freq * 65536.0 / (double)outputFrequency;
				if(accumulator >= overflow)
				{
					// avoid overflow and loss of precision
					accumulator -= overflow;
				}
				voiceIn->accumulator = accumulator;
				uint16_t accu16 = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;

				uint16_t sample = 0x7FFF; // silence

				switch(waveType)
				{
				case WaveTypeSawtooth: {
					sample = accu16;
					break;
				}
				case WaveTypePulse: {
					sample = ((accu16 >> 8) > pulseWidth) ? 0xFFFF : 0x0000;
					break;
				}
				case WaveTypeTriangle: {
					// up, down, up
					uint32_t rise = (uint32_t)triangleRise << 8;
					uint16_t phase = accu16 + (uint16_t)(rise >> 1);
					uint32_t s = (phase < rise)
						? (((uint32_t)phase << 16) / rise)
						: (((uint32_t)(0x10000 - phase) << 16) / (0x10000 - rise));
					sample = (s > 0xFFFF) ? 0xFFFF : (uint16_t)s;
					break;
				}
				case WaveTypeNoise: {
					// sauce: Claude Opus 5
					// bit 12 of accu16 toggles 16 times per cycle, so the LFSR clock
					// rate in Hz equals the raw frequency register. Count the
					// crossings instead of detecting one, or the clock saturates at
					// one step per output sample above freq == outputFrequency.
					int steps = (((uint32_t)accu16 >> 12) - ((uint32_t)accu16Last >> 12)) & 0x0F;
					while(steps--)
					{
						uint16_t r = voiceIn->noiseRandom;
						uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5)) & 1;
						voiceIn->noiseRandom = (r >> 1) | (bit << 15);
					}

					// sauce: Claude Opus 5
					float x = (float)((int32_t)voiceIn->noiseRandom - 0x7FFF);
					if(noiseTilt == NOISE_TILT_NEUTRAL)
					{
						// flat: no filter, only the trim that keeps the widths level
						sample = (uint16_t)((int32_t)(x * NOISE_TILT_TARGET) + 0x7FFF);
					}
					else
					{
						if(noiseTilt != voiceIn->noiseTiltIndex || freq != voiceIn->noiseTiltFreq)
						{
							audio_updateNoiseTilt(voiceIn, noiseTilt, freq, outputFrequency);
						}

						float y = voiceIn->noiseTiltState + voiceIn->noiseTiltK * (x - voiceIn->noiseTiltState);
						voiceIn->noiseTiltState = y;

						float o = voiceIn->noiseTiltGain * ((noiseTilt < NOISE_TILT_NEUTRAL) ? y : (x - y));
						// clamp before the cast, both to keep the 4 voice sum inside
						// int16 and to keep the rounding below unbiased
						if(o > 32768.0f)
							o = 32768.0f;
						else if(o < -32767.0f)
							o = -32767.0f;
						sample = (uint16_t)(((int32_t)(o + 32768.5f) - 32768) + 0x7FFF);
					}
					break;
				}
				}

				// --- TIMEOUT ---

				if(voice->attr.timeout)
				{
					voiceIn->timeoutCounter -= 60.0 / outputFrequency;
					if(voiceIn->timeoutCounter <= 0.0)
					{
						voiceIn->timeoutCounter = 0.0;
						voice->status.gate = 0;
					}
				}

				// --- ENVELOPE GENERATOR ---

				if(!voice->status.gate)
				{
					voiceIn->envState = EnvStateRelease;
				}

				switch(voiceIn->envState)
				{
				case EnvStateAttack:
					voiceIn->envCounter += envRates[voice->envA] / outputFrequency;
					if(voiceIn->envCounter >= 255.0)
					{
						voiceIn->envCounter = 255.0;
						voiceIn->envState = EnvStateDecay;
					}
					break;

				case EnvStateDecay:
					if(voiceIn->envCounter > voice->envS * 16.0)
					{
						voiceIn->envCounter -= envRates[voice->envD] / outputFrequency;
					}
					break;

				case EnvStateRelease:
					if(voiceIn->envCounter > 0.0)
					{
						voiceIn->envCounter -= envRates[voice->envR] / outputFrequency;
						if(voiceIn->envCounter < 0.0)
						{
							voiceIn->envCounter = 0.0;
						}
					}
					break;
				}

				// --- OUTPUT ---

				volume = volume * (int)voiceIn->envCounter >> 8;

				// output peak to system registers
				lifeRegisters->voices[v].peak = volume;

				int16_t voiceSample =
					(((int32_t)(sample - 0x7FFF)) * volume) >> 10; // 8 bit for volume, 2 bit for global
				if(voice->status.mix & 0x01)
				{
					leftOutput += voiceSample;
				}
				if(voice->status.mix & 0x02)
				{
					rightOutput += voiceSample;
				}
			}

			// filter

			int32_t *filterBufferL = internals->filterBuffer[0];
			int32_t *filterBufferR = internals->filterBuffer[1];

			for(int f = AUDIO_FILTER_BUFFER_SIZE - 1; f > 0; f--)
			{
				filterBufferL[f] = filterBufferL[f - 1];
				filterBufferR[f] = filterBufferR[f - 1];
			}
			filterBufferL[0] = leftOutput;
			filterBufferR[0] = rightOutput;

			leftOutput = ((filterBufferL[0] >> 2) + (filterBufferL[1] >> 1) + (filterBufferL[2] >> 2));
			rightOutput = ((filterBufferR[0] >> 2) + (filterBufferR[1] >> 1) + (filterBufferR[2] >> 2));
		}

		stereoOutput[i++] = leftOutput >> volume;
		stereoOutput[i++] = rightOutput >> volume;
	}
}
