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
	// 0x0ff70
	float touchX;
	// 0x0ff74
	float touchY;
	// 0x0ff78
	struct
	{
		uint16_t width, height;
	} shown;
	// 0x0ff7c
	struct
	{
		uint16_t left, top, right, bottom;
	} safe;
	// 0x0ff84
	char key;
	// 0x0ff85
	union IOStatus status;
	// 0x0ff87
	uint8_t haptic;
	// 0x0ff88
	int keyboardHeight;
	// 0x0ff8c
};

#endif /* io_chip_h */
