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

#ifndef machine_h
#define machine_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "io_chip.h"
#include "video_chip.h"
#include "audio_chip.h"
#include "error.h"

#define VM_SIZE 0x20000
#define VM_MAX 0x1FFFF
#define PERSISTENT_RAM_SIZE 6144

struct DmaRegisters {
  uint16_t src_addr; // Support RAM or ROM
  uint16_t bytes_count;
  uint16_t dst_addr; // Only RAM
};

struct Core;

// 128Kibi
struct Machine {

    // 0x00000..0x09000
    struct VideoRam videoRam; // 36Kibi

    // 0x09000..0x0E000
    uint8_t workingRam[0x05000]; // 20Kibi

    // 0x0E000..0x0F800
    uint8_t persistentRam[PERSISTENT_RAM_SIZE]; // 6Kibi

    // TODO: absorbe nothing1 into persidentRam

    // 0x0F800..0x0FB00
    uint8_t nothing1[0x0FB00-0x0F800]; // 768 Bytes

    // 0x0FB00..0x0FF00
    struct SpriteRegisters spriteRegisters; // 1020 Bytes
    uint8_t nothing2[0x400 - sizeof(struct SpriteRegisters)]; // 4 bytes

    // 0x0FF00
    struct ColorRegisters colorRegisters; // 32 Bytes

    // 0x0FF20..0x0FF40
    struct VideoRegisters videoRegisters; // 20 Bytes
    uint8_t nothing3[0x20 - sizeof(struct VideoRegisters)]; // 12 Bytes

    // 0x0FF40..0x0FF70
    struct AudioRegisters audioRegisters;

    // 0x0FF70..0x0FFA0
    struct IORegisters ioRegisters; // 24 Bytes
    uint8_t nothing4[0x30 - sizeof(struct IORegisters)]; // 24 Bytes

    // 0x0FFA0..0x0FFB0
    struct DmaRegisters dmaRegisters; // 6 Bytes
    uint8_t manually_mapped[0x10 - sizeof(struct DmaRegisters)]; // 10 Bytes

    // 0x0FFB0.0x10000
    uint8_t nothing5[0x10000 - 0xFFB0]; // 80 Bytes !!!!!

    // 0x10000..0x20000
    uint8_t cartridgeRom[0x10000]; // 64Kibi
};

struct MachineInternals {
    struct AudioInternals audioInternals;
    bool hasAccessedPersistent;
    bool hasChangedPersistent;
    bool isEnergySaving;
    int energySavingTimer;
		bool planeColor0IsOpaque[4];
};

void machine_init(struct Core *core);
void machine_reset(struct Core *core, bool resetPersistent);
int machine_peek(struct Core *core, int address);
int16_t machine_peek_short(struct Core *core, int address, enum ErrorCode *errorCode);
int32_t machine_peek_long(struct Core *core, int address, enum ErrorCode *errorCode);
bool machine_poke(struct Core *core, int address, int value);
bool machine_poke_short(struct Core *core, int address, int16_t value);
bool machine_poke_long(struct Core *core, int address, int32_t value);
void machine_enableAudio(struct Core *core);
void machine_suspendEnergySaving(struct Core *core, int numUpdates);

#endif /* machine_h */
