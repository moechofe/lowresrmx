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
#include "pcg_basic.h"
#include "overlay_debugger.h"

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
	memset((void *)&(core->machine->spriteRegisters), 0, sizeof(struct SpriteRegisters));
	memset((void *)&(core->machine->colorRegisters), 0, sizeof(struct ColorRegisters));
	memset((void *)&(core->machine->videoRegisters), 0, sizeof(struct VideoRegisters));
	memset((void *)&(core->machine->dmaRegisters), 0, sizeof(struct DmaRegisters));
	memset((void *)&(core->machine->audioRegisters), 0, sizeof(struct AudioRegisters));
	memset((void *)&(core->machine->ioRegisters), 0, sizeof(struct IORegisters));

	// // rom
	// memset(core->machine->cartridgeRom, 0, 0x10000);

	memset(core->machineInternals, 0, sizeof(struct MachineInternals));
	audio_reset(core);

	pcg32_srandom_r(&core->interpreter->defaultRng, 4715711917271117164, (intptr_t)&core->interpreter->defaultRng);
}

int machine_peek(struct Core *core, int address)
{
	if (
		 (address < 0) // outside mapped memory
	|| (address > VM_MAX) // outside mapped memory
	// || (address >= 0x0f800 && address < 0x0fb00) // nothing 1
	|| (address >= 0x0fefc && address < 0x0ff00) // nothing 2
	|| (address >= 0x0ff34 && address < 0x0ff40) // nothing 3
	|| (address >= 0x0ff8c && address < 0x0ffa0) // nothing 4
	|| (address >= 0x0ffb0 && address < 0x10000) // nothing 5
	)
	{
		return -1;
	}

	else if (address >= 0x0e000 && address < 0x0fb00) // persistent
	{
		if (!core->machineInternals->hasAccessedPersistent)
		{
			delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
			core->machineInternals->hasAccessedPersistent = true;
		}
	}

	// manually mapped
	else if (address == 0x0ffa6) return core->interpreter->cycles & 0xff;
	else if (address == 0x0ffa7) return (core->interpreter->cycles >> 8) & 0xff;
	else if (address == 0x0ffa8) return (core->interpreter->cycles >> 16) & 0xff;
	else if (address == 0x0ffa9) return (core->interpreter->cycles >> 24) & 0xff;

	else if (address == 0x0ffaa) return core->interpreter->numSimpleVariables & 0xff;
	else if (address == 0x0ffab) return (core->interpreter->numSimpleVariables >> 8) & 0xff;

	else if (address == 0x0ffac) return core->interpreter->numArrayVariables & 0xff;
	else if (address == 0x0ffad) return (core->interpreter->numArrayVariables >> 8) & 0xff;

	else if (address == 0x0ffae) return core->interpreter->numLabelStackItems & 0xff;
	else if (address == 0x0ffaf) return (core->interpreter->numLabelStackItems >> 8) & 0xff;

	machine_checkForTrakedMemoryAccess(core, (uint16_t)address, true, false);

	// read byte
	return *(uint8_t *)((uint8_t *)core->machine + address);
}

int16_t machine_peek_short(struct Core *core, int address, enum ErrorCode *errorCode)
{
	*errorCode = ErrorNone;
	int peek1 = machine_peek(core, address);
	int peek2 = machine_peek(core, address + 1);
	if (peek1 < 0 || peek2 < 0)
		*errorCode = ErrorIllegalMemoryAccess;
	return peek1 | (peek2 << 8); // MAY return negative number, and it's ok
}

int32_t machine_peek_long(struct Core *core, int address, enum ErrorCode *errorCode)
{
	*errorCode = ErrorNone;
	int peek1 = machine_peek(core, address);
	int peek2 = machine_peek(core, address + 1);
	int peek3 = machine_peek(core, address + 2);
	int peek4 = machine_peek(core, address + 3);
	if (peek1 < 0 || peek2 < 0 || peek3 < 0 || peek4 < 0)
		*errorCode = ErrorIllegalMemoryAccess;
	return peek1 | (peek2 << 8) | (peek3 << 16) | (peek4 << 24);
}

bool machine_poke(struct Core *core, int address, int value)
{
	if (
		 (address < 0) // outside mapped memory
	|| (address >= 0x10000) // ROM
	// || (address >= 0x0f800 && address < 0x0fb00) // nothing 1
	|| (address >= 0x0fefc && address < 0x0ff00) // nothing 2
	|| (address >= 0x0ff34 && address < 0x0ff40) // nothing 3
	|| (address >= 0x0ff88 && address < 0x0ffa0) // nothing 4
	|| (address >= 0x0ffb0 && address < 0x10000) // nothing 5
	|| (address >= 0x0ffa6 && address < 0x0ffb0) // manually mapped
	)
	{
		return false;
	}
	else if (address >= 0x0e000 && address < 0x0fb00) // persistent
	{
		if (!core->machineInternals->hasAccessedPersistent)
		{
			delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
			core->machineInternals->hasAccessedPersistent = true;
		}
		core->machineInternals->hasChangedPersistent = true;
	}
	else if (address >= 0x0ff70 && address < 0x0ffa0) // io registers
	{
		if (address == 0xff87) {} // haptic
		else return false; // read only
	}

	machine_checkForTrakedMemoryAccess(core, (uint16_t)address, false, true);

	// write byte
	*(uint8_t *)((uint8_t *)core->machine + address) = value & 0xFF;

	if (address == 0x0ff70+0x15) // IOStatus
	{
		delegate_controlsDidChange(core);
	}
	else if (address >= 0x0ff40 && address < 0x0ff70) // AudioRegisters
	{
		machine_enableAudio(core);
	}

	return true;
}

bool machine_poke_short(struct Core *core, int address, int16_t value)
{
	bool poke1 = machine_poke(core, address, value);
	bool poke2 = machine_poke(core, address + 1, value >> 8);
	if (!poke1 || !poke2)
		return false;
	return true;
}

bool machine_poke_long(struct Core *core, int address, int32_t value)
{
	bool poke1 = machine_poke(core, address, value);
	bool poke2 = machine_poke(core, address + 1, value >> 8);
	bool poke3 = machine_poke(core, address + 2, value >> 16);
	bool poke4 = machine_poke(core, address + 3, value >> 24);
	if (!poke1 || !poke2 || !poke3 || !poke4)
		return false;
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

void machine_trackMemory(struct Core *core, uint16_t address, bool read, bool write)
{
	// modify existing track
	for (int i = 0; i < core->machineInternals->numMemoryTracks; i++)
	{
		struct MemoryTrack *track = &core->machineInternals->memoryTracks[i];
		if (track->address == address)
		{
			if (read) track->read = true;
			if (write) track->write = true;
			return;
		}
	}
	// add new track
	if (core->machineInternals->numMemoryTracks < MAX_MEMORY_TRACK)
	{
		struct MemoryTrack *track = &core->machineInternals->memoryTracks[core->machineInternals->numMemoryTracks++];
		track->address = address;
		track->read = read;
		track->write = write;
	}
}

void machine_checkForTrakedMemoryAccess(struct Core *core, uint16_t address, bool read, bool write)
{
	if (core->interpreter->pass == StatePaused) return;
	for (int i = 0; i < core->machineInternals->numMemoryTracks; i++)
	{
		struct MemoryTrack *track = &core->machineInternals->memoryTracks[i];
		if (track->address == address)
		{
			if ((read && track->read) || (write && track->write))
			{
				trigger_debugger(core);
			}
			return;
		}
	}
}
