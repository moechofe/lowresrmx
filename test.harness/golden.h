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

#ifndef golden_h
#define golden_h

#include <stdbool.h>
#include <stdint.h>

// one 60 Hz frame of interleaved stereo at 44100 Hz, counted in int16_t elements
#define GOLDEN_AUDIO_SAMPLES 1470

struct Core;

/** Hashes the frame as normalized R,G,B bytes, so the result is the same whatever ABGR is. */
uint64_t golden_hashFrame(struct Core *core, uint32_t *scratchPixels);

/** Hashes one frame of audio rendered at full scale. */
uint64_t golden_hashAudio(struct Core *core, int16_t *scratchSamples);

/** Writes a SCREEN_WIDTH x SCREEN_HEIGHT frame, for eyeballing a golden mismatch. */
bool golden_writePng(const char *filename, uint32_t *pixels);

/** Writes an RGB png from 32 bit pixels; pitch is the source row stride in bytes. */
bool writeImage(const char *filename, int width, int height, uint32_t *pixels, int pitch, int scale);

#endif /* golden_h */
