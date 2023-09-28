//
// Copyright 2017 Timo Kloss
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

#include "startup_sequence.h"
#include "core.h"
#include <string.h>
#include <stdint.h>
#include "io_chip.h"

#define FONT_CHAR_OFFSET 192

extern uint32_t better_palette[];

void runStartupSequence(struct Core *core)
{
    struct DataEntry *entries = core->interpreter->romDataManager.entries;
    
    // init font and window
    struct TextLib *textLib = &core->interpreter->textLib;
    textLib->fontCharOffset = FONT_CHAR_OFFSET;
    txtlib_clearScreen(textLib);

    struct IORegisters *io = &core->machine->ioRegisters;

    textLib->windowX = (io->safe.left+7)/8;
    textLib->windowY = (io->safe.top+7)/8;
    textLib->windowWidth = io->shown.width/8 - (io->safe.left+7)/8 - (io->safe.right+7)/8;
    textLib->windowHeight = io->shown.height/8 - (io->safe.top+7)/8 - (io->safe.bottom+7)/8;
    
    // default characters/font
    if (strcmp(entries[0].comment, "FONT") == 0)
    {
        memcpy(&core->machine->videoRam.characters[FONT_CHAR_OFFSET], &core->machine->cartridgeRom[entries[0].start], entries[0].length);
    }
    
    // default palettes
    uint8_t *colors = core->machine->colorRegisters.colors;
    
    colors[0] = 15;
    colors[1] = 2;
    colors[2] = 12;
    colors[3] = 0;

    colors[4] = 0;
    colors[5] = 29;
    colors[6] = 31;
    colors[7] = 0;
    
    colors[8] = 0;
    colors[9] = 30;
    colors[10] = 17;
    colors[11] = 0;

    colors[12] = 0;
    colors[13] = 2;
    colors[14] = 30;
    colors[15] = 0;
    
    for (int i = 0; i < 16; i += 4)
    {
        colors[16 + i] = 0;
        colors[17 + i] = 2;
        colors[18 + i] = 3;
        colors[19 + i] = 5;
    }
    
    // main palettes
    int palLen = entries[1].length;
    if (palLen > 32) palLen = 32;
    memcpy(core->machine->colorRegisters.colors, &core->machine->cartridgeRom[entries[1].start], palLen);
    
    // main characters
    memcpy(core->machine->videoRam.characters, &core->machine->cartridgeRom[entries[2].start], entries[2].length);

    // main background source
    int bgStart = entries[3].start;
    core->interpreter->textLib.sourceAddress = 0x10000 + bgStart + 4;
    core->interpreter->textLib.sourceWidth = core->machine->cartridgeRom[bgStart + 2];
    core->interpreter->textLib.sourceHeight = core->machine->cartridgeRom[bgStart + 3];
    
    // voices
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct Voice *voice = &core->machine->audioRegisters.voices[i];
        voice->attr.pulseWidth = 8;
        voice->status.volume = 15;
        voice->status.mix = 3;
        voice->envS = 15;
    }
    
    // main sound source
    core->interpreter->audioLib.sourceAddress = 0x10000 + entries[15].start;
}
