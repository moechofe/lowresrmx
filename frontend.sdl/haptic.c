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

#include "haptic.h"
#include "sdl_include.h"

// Knobs for a device that needs more punch than Apple's numbers give. Keep them
// relative: clamping a minimum duration instead would collapse Rigid, Selection
// and Light into Medium.
#define HAPTIC_INTENSITY_SCALE 1.0f
#define HAPTIC_DURATION_SCALE 1.0f

#define HAPTIC_MAX_PULSES 4

struct HapticPulse
{
	uint16_t onset; // ms from the start of the pattern
	uint16_t duration; // ms, encodes sharpness: short is sharp, long is dull
	float intensity; // 0..1
};

struct HapticPattern
{
	int numPulses;
	struct HapticPulse pulses[HAPTIC_MAX_PULSES];
};

struct HapticQueuedPulse
{
	uint64_t at; // SDL ticks at which the pulse must be played
	uint16_t duration;
	float intensity;
};

// try to replicate Apple's nine iOS patterns
// Sauce: https://developer.apple.com/design/human-interface-guidelines/playing-haptics
static const struct HapticPattern hapticPatterns[] = {
	{0, {{0, 0, 0.0f}}}, // None
	{4, {{0, 12, 0.8f}, {50, 12, 0.8f}, {136, 12, 1.0f}, {236, 12, 0.6f}}}, // Error
	{2, {{0, 12, 0.9f}, {134, 12, 0.7f}}}, // Warning
	{2, {{0, 12, 0.7f}, {116, 12, 1.0f}}}, // Success
	{1, {{0, 20, 1.0f}}}, // Heavy
	{1, {{0, 10, 0.6f}}}, // Light
	{1, {{0, 15, 0.8f}}}, // Medium
	{1, {{0, 8, 0.9f}}}, // Rigid
	{1, {{0, 25, 0.7f}}}, // Soft
	{1, {{0, 8, 0.6f}}}, // Selection
};

static SDL_Haptic *hapticDevice = NULL;
static struct HapticQueuedPulse hapticQueue[HAPTIC_MAX_PULSES];
static int hapticQueueCount = 0;
static int hapticQueueIndex = 0;

void haptic_init(void)
{
	// SDL_GetHaptics returns a non NULL but empty list when there is no device,
	// which is the normal case on a desktop, so the count has to be checked or
	// SDL_OpenHaptic(0) fails and logs on every launch.
	int numHaptics = 0;
	SDL_HapticID *haptics = SDL_GetHaptics(&numHaptics);
	if(haptics)
	{
		if(numHaptics > 0)
		{
			hapticDevice = SDL_OpenHaptic(haptics[0]);
			if(!hapticDevice)
				SDL_Log("Haptic: %s", SDL_GetError());
		}
		SDL_free(haptics);
	}

	if(!hapticDevice)
		return;

	// SDL_PlayHapticRumble needs this, and on Android it claims the one and only
	// SDL_HAPTIC_LEFTRIGHT effect slot the built in vibrator offers.
	if(!SDL_InitHapticRumble(hapticDevice))
	{
		SDL_Log("Haptic: %s", SDL_GetError());
		SDL_CloseHaptic(hapticDevice);
		hapticDevice = NULL;
	}
}

void haptic_deinit(void)
{
	if(!hapticDevice)
		return;

	SDL_StopHapticRumble(hapticDevice);
	SDL_CloseHaptic(hapticDevice);
	hapticDevice = NULL;
	hapticQueueCount = 0;
	hapticQueueIndex = 0;
}

void haptic_play(enum HapticMode mode)
{
	if(!hapticDevice)
		return;

	if(mode <= None || mode > Selection)
		return;

	const struct HapticPattern *pattern = &hapticPatterns[mode];
	uint64_t now = SDL_GetTicks();

	for(int i = 0; i < pattern->numPulses; i++)
	{
		hapticQueue[i].at = now + pattern->pulses[i].onset;
		hapticQueue[i].duration = pattern->pulses[i].duration;
		hapticQueue[i].intensity = pattern->pulses[i].intensity;
	}
	hapticQueueCount = pattern->numPulses;
	hapticQueueIndex = 0;

	// Plays the first pulse right away, the frame loop plays the others.
	haptic_update();
}

void haptic_update(void)
{
	if(!hapticDevice || hapticQueueIndex >= hapticQueueCount)
		return;

	const struct HapticQueuedPulse *pulse = &hapticQueue[hapticQueueIndex];
	if(SDL_GetTicks() < pulse->at)
		return;

	SDL_PlayHapticRumble(
		hapticDevice, pulse->intensity * HAPTIC_INTENSITY_SCALE, (Uint32)(pulse->duration * HAPTIC_DURATION_SCALE));
	++hapticQueueIndex;
}
