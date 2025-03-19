//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef io_chip_h
#define io_chip_h

#include <stdint.h>

union IOStatus
{
	struct
	{
		uint8_t touch : 1;
		uint8_t keyboardVisible : 1;
	};
	uint8_t value;
};

struct IORegisters
{
	float touchX; // offset: 0
	float touchY; // offset: 0x4
	struct
	{
		uint16_t width, height;
	} shown; // offset: 0x8
	struct
	{
		uint16_t left, top, right, bottom;
	} safe; // offset: 0xC
	char key; // offset: 0x14
	union IOStatus status; // offset: 0x15
	uint8_t haptic; // offset: 0x16
};

#endif /* io_chip_h */
