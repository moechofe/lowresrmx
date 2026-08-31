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

#ifndef haptic_h
#define haptic_h

#include "core_delegate.h" // enum HapticMode

void haptic_init(void);
void haptic_deinit(void);

/** Starts playing the pattern of the given mode, replacing any pattern still playing */
void haptic_play(enum HapticMode mode);

/** Must be called once per frame to play the remaining pulses of a multi pulse pattern */
void haptic_update(void);

#endif /* haptic_h */
