//
// Copyright 2016-2018 Timo Kloss
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

#include "machine.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "core.h"
#include "video_chip.h"
#include "io_chip.h"
#include "audio_chip.h"

void machine_init(struct Core *core)
{
    assert(sizeof(struct Machine) == VM_SIZE);
}

void machine_reset(struct Core *core, bool resetPersistent)
{
    // video ram, working ram
    memset(core->machine, 0, 0xE000);
    
    if (resetPersistent)
    {
        // persistent ram
        memset(core->machine->persistentRam, 0, PERSISTENT_RAM_SIZE);
    }

    // TODO: memset(core->machine->nothing2, 0, VM_SIZE - 0x0FB00);

    // sprite
    memset((void*)&(core->machine->spriteRegisters), 0, sizeof(struct SpriteRegisters));
    memset((void*)&(core->machine->colorRegisters), 0, sizeof(struct ColorRegisters));
    memset((void*)&(core->machine->videoRegisters), 0, sizeof(struct VideoRegisters));
    memset((void*)&(core->machine->dmaRegisters), 0, sizeof(struct DmaRegisters));
    memset((void*)&(core->machine->audioRegisters), 0, sizeof(struct AudioRegisters));
    memset((void*)&(core->machine->ioRegisters), 0, sizeof(struct IORegisters));

    // // rom
    // memset(core->machine->cartridgeRom, 0, 0x10000);
    
    memset(core->machineInternals, 0, sizeof(struct MachineInternals));
    audio_reset(core);
}

int machine_peek(struct Core *core, int address)
{
    if (address < 0 || address > VM_MAX)
    {
        return -1;
    }
    if (address >= 0x0E000 && address < 0x0F800) // persistent
    {
        if (!core->machineInternals->hasAccessedPersistent)
        {
            delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
            core->machineInternals->hasAccessedPersistent = true;
        }
    }
    
    // read byte
    return *(uint8_t *)((uint8_t *)core->machine + address);
}

short int machine_peek_short(struct Core *core, int address)
{
    int peek1=machine_peek(core, address);
    int peek2=machine_peek(core, address+1);
    if(peek1<0 || peek2<0) return -1;
    return peek1|(peek2<<8);
}

bool machine_poke(struct Core *core, int address, int value)
{
    if (address < 0 && address >= 0x10000)
    {
        // cartridge ROM or outside RAM
        return false;
    }
    // if((address>=0x0F800&&address<0x0FB00)
    // && (address>=0x0FEFC&&address<0x0FF00)
    // && (address>=0x0FEFC&&address<0x0FF40)
    // && (address>=0x0FF36&&address<0x0FF00)
    // {
    //     // reserved memory
    //     return false;
    // }
    // if (address >= 0xFF80) // TODO: update this
    // {
    //     // reserved registers
    //     return false;
    // }
    if (address == 0xFF76) // IO attributes
    {
        // check for illegal input change (gamepad <-> touch)
        union IOAttributes currAttr = core->machine->ioRegisters.attr;
        union IOAttributes newAttr;
        newAttr.value = value;
    }
    else if (address >= 0x0E000 && address < 0x0F800) // persistent
    {
        if (!core->machineInternals->hasAccessedPersistent)
        {
            delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
            core->machineInternals->hasAccessedPersistent = true;
        }
        core->machineInternals->hasChangedPersistent = true;
    }
    
    // write byte
    *(uint8_t *)((uint8_t *)core->machine + address) = value & 0xFF;
    
    if (address == 0xFF76) // IO attributes
    {
        delegate_controlsDidChange(core);
    }
    else if (address >= 0xFF40 && address < 0xFF70) // audio
    {
        machine_enableAudio(core);
    }
    return true;
}

bool machine_poke_short(struct Core *core, int address, int16_t value)
{
    bool poke1 = machine_poke(core, address, value);
    bool poke2 = machine_poke(core, address + 1, value >> 8);
    if (!poke1 || !poke2) return false;
    return true;
}

void machine_enableAudio(struct Core *core)
{
    if (!core->machineInternals->audioInternals.audioEnabled)
    {
        core->machineInternals->audioInternals.audioEnabled = true;
        delegate_controlsDidChange(core);
    }
}

void machine_suspendEnergySaving(struct Core *core, int numUpdates)
{
    if (core->machineInternals->energySavingTimer < numUpdates)
    {
        core->machineInternals->energySavingTimer = numUpdates;
    }
}
