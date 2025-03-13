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

#include "core.h"
#include "core.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

void disk_init(struct Core *core)
{
    // init lazily in disk_prepare()
}

void disk_deinit(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data)
    {
        free(dataManager->data);
        dataManager->data = NULL;
    }
    data_deinit(dataManager);
}

void disk_reset(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data)
    {
        data_reset(dataManager);
    }
}

bool disk_prepare(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data == NULL)
    {
        dataManager->data = calloc(DATA_SIZE, 1);
        if (!dataManager->data) exit(EXIT_FAILURE);

        data_init(dataManager);
    }
    return delegate_diskDriveWillAccess(core);
}

bool disk_saveFile(struct Core *core, int index, char *comment, int address, int length)
{
    if (!disk_prepare(core))
    {
        return false;
    }

    assert(address >= 0 && address + length <= sizeof(struct Machine));
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (!data_canSetEntry(dataManager, index, length))
    {
        delegate_diskDriveIsFull(core);
    }
    else
    {
        uint8_t *source = &((uint8_t *)core->machine)[address];
        data_setEntry(dataManager, index, comment, source, length);

        delegate_diskDriveDidSave(core);
    }
    return true;
}

bool disk_loadFile(struct Core *core, int index, int address, int maxLength, int offset, bool *pokeFailed)
{
    if (!disk_prepare(core))
    {
        return false;
    }

    struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];
    uint8_t *data = core->diskDrive->dataManager.data;

    // read file
    int start = entry->start + offset;
    int length = entry->length;
    if (maxLength > 0 && length > maxLength)
    {
        length = maxLength;
    }
    if (offset + length > entry->length)
    {
        length = entry->length - offset;
    }
    for (int i = 0; i < length; i++)
    {
        bool poke = machine_poke(core, address + i, data[i + start]);
        if (!poke)
        {
            *pokeFailed = true;
            return true;
        }
    }
    return true;
}
//
// Copyright 2017-2018 Timo Kloss
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

#include "core.h"
#include "core.h"

const int bootIntroStateAddress = 0xA000;

// const char *bootIntroSourceCode = "VER$=\"" CORE_VERSION "\"\n\nGLOBAL JIN\n\nFONT 64\n\nENVELOPE 0,0,10,0,6\nENVELOPE 1,0,10,0,6\nENVELOPE 2,0,7,0,7\nENVELOPE 3,0,7,0,7\nLFO 0,5,4,0,0\nLFO 1,5,4,0,0\nVOLUME 0,15,%10\nVOLUME 1,15,%01\nVOLUME 2,15,%10\nVOLUME 3,15,%01\n\nSPRITE.A 0,(3,0,0,0,1)\nSPRITE.A 1,(0,0,0,0,1)\nSPRITE.A 2,(0,0,0,0,3)\nSPRITE.A 3,(1,0,0,0,0)\nSPRITE.A 4,(2,0,0,0,1)\n\nSPRITE 1,72,56,5\nSPRITE 2,64,56,1\n\nPAL 2\nCELL 0,15,39\nCELL 1,15,40\nCELL 2,15,41\nCELL 3,15,42\nCELL 4,15,43\nCELL 5,15,44\n\nTEXT 20-LEN(VER$),15,VER$\n\n\nDO\n SPRITE 3,76,40,7\n WAIT 30\n FOR Y=40 TO 56\n  SPRITE 3,76,Y,7\n  WAIT 2\n NEXT Y\n FOR I=1 TO 30\n  WAIT VBL\n  IF PEEK($A000)=1 THEN GOTO LOADING\n NEXT I\nLOOP\n\nLOADING:\nON VBL CALL JINGLE\nFOR Y=56 TO 49 STEP -1\n SPRITE 0,72,Y,37\n SPRITE 1,,Y,\n WAIT 4\nNEXT Y\nWAIT 60\n\nPOKE $A000,2\nDO\n N=N+1\n SPRITE 4,76,80,8+(N MOD 4)*2\n WAIT 10\nLOOP\n\nSUB JINGLE\n IF JIN=0 THEN\n  PLAY 0,52,0\n  PLAY 1,48,0\n ELSE IF JIN=15 THEN\n  STOP\n  PLAY 2,50,1\n  PLAY 3,46,1\n ELSE IF JIN=30 THEN\n  ON VBL OFF\n END IF\n JIN=JIN+1\nEND SUB\n\n#1:MAIN PALETTES\n0A2A150030381500003F2F0F003F0A34\n003F2A15003F2A15003F2A15003F2A15\n\n#2:MAIN CHARACTERS\n00000000000000000000000000000000\n3F7FC0809CBEBEBE3F40BFFFFFFFFFFF\nFF80808080808080FFFFFFFFFFFFFFFF\nFF01010101010101FFFFFFFFFFFFFFFF\nFCFE0321210D6D61FC02FDDFFFFFFFFF\n7FFF809F9F9F9F9F7F80FFFFFFFFFFFF\nFEFF01F9F9F9F9F9FE01FFFFFFFFFFFF\n7EBDBDBDBD8181FF7EC3DBC3C3FFFFFF\n08090200C100204800412200C1002241\n00000000800000000000000080000000\n00412200C100224108482000C1000209\n00000000800000000000000080000000\n08482000C10002090849220000002249\n00000000800000000000000000000000\n084922000000224908090200C1002048\n00000000000000000000000080000000\n00000000000000000000000000000000\n9C8088948880403FFFFFFFFFFFFF7F3F\n80B6929B89B692FFFFC9FFE4FFC9FFFF\n01D9496D25D949FFFF27FF93FF27FFFF\n01011129110102FCFFFFFFFFFFFFFEFC\n9F9F9F80FFFF80FFFFFFFFFFFF80FFFF\nF9F9F901FFFF01FFFFFFFFFFFF01FFFF\n00000000000000000000000000000000\n08000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000800000000000000\n00000000000000000000000000000000\n08000000000000000800000000000000\n00000000000000000000000000000000\n08000000000000000800000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n0000000000000301000000000F0F0C0F\n000000000000C08000000000F0F030F0\n00004344444473000000434444447300\n000022A2AAB62200000022A2AAB62200\n0000E794E79497000000E794E7949700\n00009C201804B80000009C201804B800\n00004564544C450000004564544C4500\n000010A040A01000000010A040A01000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000F0F000000000000\n0000000000000000F0F0000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00000000000000000000000000000000\n00181818180018000000000000000000\n006C6C24000000000000000000000000\n00247E24247E24000000000000000000\n00083E380E3E08000000000000000000\n00626408102646000000000000000000\n001C34386E643A000000000000000000\n00181830000000000000000000000000\n00000408080804000000040808080400\n00001008080810000000100808081000\n000024187E1824000000000000000000\n000018187E1818000000000000000000\n00000000181830000000000000000000\n000000007E0000000000000000000000\n00000000000010000000000000001000\n00060C18306040000000000000000000\n00001C2222221C0000001C2222221C00\n0000380808083E000000380808083E00\n00003C021C203E0000003C021C203E00\n00003C021C023C0000003C021C023C00\n00002020283E080000002020283E0800\n00003E203C023C0000003E203C023C00\n00001C203C221C0000001C203C221C00\n00003E040810200000003E0408102000\n00001C221C221C0000001C221C221C00\n00001C221E023C0000001C221E023C00\n00000018001800000000000000000000\n00000018001830000000000000000000\n00000C1830180C000000000000000000\n0000007E007E00000000000000000000\n000030180C1830000000000000000000\n003C660C180018000000000000000000\n003C666E6E603C000000000000000000\n00183C667E6666000000000000000000\n007C667C66667C000000000000000000\n003C666060663C000000000000000000\n00786C66666C78000000000000000000\n007E607860607E000000000000000000\n007E6078606060000000000000000000\n003C606E66663C000000000000000000\n0066667E666666000000000000000000\n003C181818183C000000000000000000\n001E060606663C000000000000000000\n00666C78786C66000000000000000000\n0060606060607E000000000000000000\n0042667E7E6666000000000000000000\n0066767E6E6666000000000000000000\n003C666666663C000000000000000000\n007C667C606060000000000000000000\n003C66666A6C3E000000000000000000\n007C667C786C66000000000000000000\n003E603C06067C000000000000000000\n007E1818181818000000000000000000\n0066666666663C000000000000000000\n00666666663C18000000000000000000\n0066667E7E6642000000000000000000\n00663C183C6666000000000000000000\n0066663C181818000000000000000000\n007E0C1830607E000000000000000000\n003C303030303C000000000000000000\n006030180C0602000000000000000000\n003C0C0C0C0C3C000000000000000000\n00183C66000000000000000000000000\n0000000000007E000000000000000000\n";

const char *bootIntroSourceCode = "POKE $A000,2\nSTOP";
//
// Copyright 2016-2020 Timo Kloss
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

#include "core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "core.h"
#include "core.h"

const char CoreInputKeyReturn = '\n';
const char CoreInputKeyBackspace = '\b';
const char CoreInputKeyRight = 17;
const char CoreInputKeyLeft = 18;
const char CoreInputKeyDown = 19;
const char CoreInputKeyUp = 20;

void core_handleInput(struct Core *core, struct CoreInput *input);

void core_init(struct Core *core)
{
	memset(core, 0, sizeof(struct Core));

	core->machine = calloc(1, sizeof(struct Machine));
	if (!core->machine)
		exit(EXIT_FAILURE);

	core->machineInternals = calloc(1, sizeof(struct MachineInternals));
	if (!core->machineInternals)
		exit(EXIT_FAILURE);

	core->interpreter = calloc(1, sizeof(struct Interpreter));
	if (!core->interpreter)
		exit(EXIT_FAILURE);

	core->diskDrive = calloc(1, sizeof(struct DiskDrive));
	if (!core->diskDrive)
		exit(EXIT_FAILURE);

	core->overlay = calloc(1, sizeof(struct Overlay));
	if (!core->overlay)
		exit(EXIT_FAILURE);

	struct IORegisters *ioRegisters = &core->machine->ioRegisters;

	machine_init(core);
	itp_init(core);
	overlay_init(core);
	disk_init(core);
}

void core_deinit(struct Core *core)
{
	itp_deinit(core);
	disk_deinit(core);

	free(core->machine);
	core->machine = NULL;

	free(core->machineInternals);
	core->machineInternals = NULL;

	free(core->interpreter);
	core->interpreter = NULL;

	free(core->diskDrive);
	core->diskDrive = NULL;

	free(core->overlay);
	core->overlay = NULL;
}

void core_setDelegate(struct Core *core, struct CoreDelegate *delegate)
{
	core->delegate = delegate;
}

struct CoreError core_compileProgram(struct Core *core, const char *sourceCode, bool resetPersistent)
{
	machine_reset(core, resetPersistent);
	overlay_reset(core);
	disk_reset(core);
	return itp_compileProgram(core, sourceCode);
}

void core_traceError(struct Core *core, struct CoreError error)
{
	core->interpreter->debug = false;
	struct TextLib *lib = &core->overlay->textLib;
	txtlib_printText(lib, err_getString(error.code));
	txtlib_printText(lib, "\n");
	if (error.sourcePosition >= 0 && core->interpreter->sourceCode)
	{
		int number = lineNumber(core->interpreter->sourceCode, error.sourcePosition);
		char lineNumberText[30];
		sprintf(lineNumberText, "IN LINE %d:\n", number);
		txtlib_printText(lib, lineNumberText);

		const char *line = lineString(core->interpreter->sourceCode, error.sourcePosition);
		if (line)
		{
			txtlib_printText(lib, line);
			txtlib_printText(lib, "\n");
			free((void *)line);
		}
	}
}

void core_willRunProgram(struct Core *core, long secondsSincePowerOn)
{
	runStartupSequence(core);
	core->interpreter->timer = (float)(secondsSincePowerOn * 60 % TIMER_WRAP_VALUE);
	machine_suspendEnergySaving(core, 30);
	delegate_controlsDidChange(core);
}

void core_update(struct Core *core, struct CoreInput *input)
{
	core_handleInput(core, input);
	itp_runInterrupt(core, InterruptTypeVBL);
	prtclib_interrupt(core, &core->interpreter->particlesLib);
	itp_runProgram(core);
	prtclib_update(core, &core->interpreter->particlesLib);
	itp_didFinishVBL(core);
	overlay_updateLayout(core, input);
	overlay_draw(core, true);
	audio_bufferRegisters(core);
}

void core_handleInput(struct Core *core, struct CoreInput *input)
{
	struct IORegisters *ioRegisters = &core->machine->ioRegisters;
	// union IOAttributes ioAttr = ioRegisters->attr;

	bool processedOtherInput = false;

	if (input->key != 0)
	{
		// if (ioRegisters->status.keyboardEnabled)
		// if (ioAttr.keyboardEnabled)
		// {
			char key = input->key;
			if ((key >= 32 && key < 127) || key == CoreInputKeyBackspace || key == CoreInputKeyReturn || key == CoreInputKeyDown || key == CoreInputKeyUp || key == CoreInputKeyRight || key == CoreInputKeyLeft)
			{
				ioRegisters->key = key;
			}
		// }
		input->key = 0;
		machine_suspendEnergySaving(core, 2);
	}

	if (input->touch)
	{
		{
			ioRegisters->status.touch = 1;
			float x = input->touchX;
			float y = input->touchY;
			if (core->interpreter->compat)
			{
				int sw = ioRegisters->shown.width != 0 ? ioRegisters->shown.width : SCREEN_WIDTH;
				int sh = ioRegisters->shown.height != 0 ? ioRegisters->shown.height : SCREEN_HEIGHT;
				x -= (sw - 160) / 2;
				y -= (sh - 128) / 2;
				// if (x < 0) x = 0; else if (x >= 160) x = 160 - 1;
				// if (y < 0) y = 0; else if (y >= 128) y = 128 - 1;
			}
			ioRegisters->touchX = x;
			ioRegisters->touchY = y;
		}
		machine_suspendEnergySaving(core, 2);
	}
	else
	{
		ioRegisters->status.touch = 0;
	}

	ioRegisters->shown.width = input->width;
	ioRegisters->shown.height = input->height;

	ioRegisters->safe.right = input->right;
	ioRegisters->safe.top = input->top;
	ioRegisters->safe.left = input->left;
	ioRegisters->safe.bottom = input->bottom;

	struct TextLib *textLib = &core->interpreter->textLib;
	if (textLib->windowWidth == 0 && textLib->windowHeight == 0)
	{
		textLib->windowX = (input->left + 7) / 8;
		textLib->windowY = (input->top + 7) / 8;
		textLib->windowWidth = input->width / 8 - (input->left + 7) / 8 - (input->right + 7) / 8;
		textLib->windowHeight = input->height / 8 - (input->top + 7) / 8 - (input->bottom + 7) / 8;
	}
	//
	//    for (int i = 0; i < NUM_GAMEPADS; i++)
	//    {
	//        union Gamepad *gamepad = &ioRegisters->gamepads[i];
	//        if (ioAttr.gamepadsEnabled > i && !ioAttr.keyboardEnabled)
	//        {
	//            struct CoreInputGamepad *inputGamepad = &input->gamepads[i];
	//            gamepad->up = inputGamepad->up && !inputGamepad->down;
	//            gamepad->down = inputGamepad->down && !inputGamepad->up;
	//            gamepad->left = inputGamepad->left && !inputGamepad->right;
	//            gamepad->right = inputGamepad->right && !inputGamepad->left;
	//            gamepad->buttonA = inputGamepad->buttonA;
	//            gamepad->buttonB = inputGamepad->buttonB;
	//
	//            if (inputGamepad->up || inputGamepad->down || inputGamepad->left || inputGamepad->right)
	//            {
	//                // some d-pad combinations are not registered as I/O, but mark them anyway.
	//                processedOtherInput = true;
	//            }
	//
	//            if (gamepad->value)
	//            {
	//                machine_suspendEnergySaving(core, 2);
	//            }
	//        }
	//        else
	//        {
	//            gamepad->value = 0;
	//        }
	//    }

	if (input->pause)
	{
		if (core->interpreter->state == StatePaused)
		{
			core->interpreter->state = StateEvaluate;
			overlay_updateState(core);
			processedOtherInput = true;
		}
		else if (!core->machine->ioRegisters.status.keyboardEnabled)
		{
			// else if (!ioAttr.keyboardEnabled) {
			ioRegisters->status.pause = 1;
		}
		input->pause = false;
	}

	input->out_hasUsedInput = processedOtherInput || ioRegisters->key || ioRegisters->status.value;
	// || ioRegisters->gamepads[0].value || ioRegisters->gamepads[1].value;
}

void core_willSuspendProgram(struct Core *core)
{
	if (core->machineInternals->hasChangedPersistent)
	{
		delegate_persistentRamDidChange(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
		core->machineInternals->hasChangedPersistent = false;
	}
}

void core_setDebug(struct Core *core, bool enabled)
{
	core->interpreter->debug = enabled;
	overlay_updateState(core);
}

bool core_getDebug(struct Core *core)
{
	return core->interpreter->debug;
}

bool core_isKeyboardEnabled(struct Core *core)
{
	return core->machine->ioRegisters.status.keyboardEnabled;
	// return core->machine->ioRegisters.attr.keyboardEnabled;
}

void core_setKeybordEnabled(struct Core *core, bool enabled)
{
	core->machine->ioRegisters.status.keyboardEnabled = enabled;
}

bool core_shouldRender(struct Core *core)
{
	enum State state = core->interpreter->state;
	bool shouldRender = (!core->machineInternals->isEnergySaving && state != StateEnd && state != StateNoProgram) || core->machineInternals->energySavingTimer > 0 || core->machineInternals->energySavingTimer % 20 == 0;

	core->machineInternals->energySavingTimer--;
	return shouldRender;
}

void core_diskLoaded(struct Core *core)
{
	core->interpreter->state = StateEvaluate;
}
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

#include <stdio.h>
#include "core.h"

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError)
{
    if (core->delegate->interpreterDidFail)
    {
        core->delegate->interpreterDidFail(core->delegate->context, coreError);
    }
}

bool delegate_diskDriveWillAccess(struct Core *core)
{
    if (core->delegate->diskDriveWillAccess)
    {
        return core->delegate->diskDriveWillAccess(core->delegate->context, &core->diskDrive->dataManager);
    }
    return true;
}

void delegate_diskDriveDidSave(struct Core *core)
{
    if (core->delegate->diskDriveDidSave)
    {
        core->delegate->diskDriveDidSave(core->delegate->context, &core->diskDrive->dataManager);
    }
}

void delegate_diskDriveIsFull(struct Core *core)
{
    if (core->delegate->diskDriveIsFull)
    {
        core->delegate->diskDriveIsFull(core->delegate->context, &core->diskDrive->dataManager);
    }
}

void delegate_controlsDidChange(struct Core *core)
{
    if (core->delegate->controlsDidChange)
    {
        struct ControlsInfo info;
				if (core->machine->ioRegisters.status.keyboardEnabled)
        {
						info.keyboardMode = KeyboardModeOn;
        }
        else
        {
            info.keyboardMode = KeyboardModeOff;
        }
        info.isAudioEnabled = core->machineInternals->audioInternals.audioEnabled;
        core->delegate->controlsDidChange(core->delegate->context, info);
    }
}

void delegate_persistentRamWillAccess(struct Core *core, uint8_t *destination, int size)
{
    if (core->delegate->persistentRamWillAccess)
    {
        core->delegate->persistentRamWillAccess(core->delegate->context, destination, size);
    }
}

void delegate_persistentRamDidChange(struct Core *core, uint8_t *data, int size)
{
    if (core->delegate->persistentRamDidChange)
    {
        core->delegate->persistentRamDidChange(core->delegate->context, data, size);
    }
}
//
// Copyright 2020 Timo Kloss
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

#include "core.h"
#include <string.h>
#include <stdlib.h>
#include "core.h"

void stats_init(struct Stats *stats)
{
    memset(stats, 0, sizeof(struct Stats));

    stats->tokenizer = calloc(1, sizeof(struct Tokenizer));
    if (!stats->tokenizer) exit(EXIT_FAILURE);

    stats->romDataManager = calloc(1, sizeof(struct DataManager));
    if (!stats->romDataManager) exit(EXIT_FAILURE);

    stats->romDataManager->data = calloc(1, DATA_SIZE);
    if (!stats->romDataManager->data) exit(EXIT_FAILURE);
}

void stats_deinit(struct Stats *stats)
{
    free(stats->romDataManager->data);
    stats->romDataManager->data = NULL;

    free(stats->tokenizer);
    stats->tokenizer = NULL;

    free(stats->romDataManager);
    stats->romDataManager = NULL;
}

struct CoreError stats_update(struct Stats *stats, const char *sourceCode)
{
    stats->numTokens = 0;
    stats->romSize = 0;

    struct CoreError error = err_noCoreError();

    const char *upperCaseSourceCode = uppercaseString(sourceCode);
    if (!upperCaseSourceCode)
    {
        error = err_makeCoreError(ErrorOutOfMemory, -1);
        goto cleanup;
    }

    error = tok_tokenizeUppercaseProgram(stats->tokenizer, upperCaseSourceCode);
    if (error.code != ErrorNone)
    {
        goto cleanup;
    }

    stats->numTokens = stats->tokenizer->numTokens;

    struct DataManager *romDataManager = stats->romDataManager;
    error = data_uppercaseImport(romDataManager, upperCaseSourceCode, false);
    if (error.code != ErrorNone)
    {
        goto cleanup;
    }

    stats->romSize = data_currentSize(stats->romDataManager);

    // add default characters if ROM entry 0 is unused
    struct DataEntry *entry0 = &romDataManager->entries[0];
    if (entry0->length == 0 && (DATA_SIZE - data_currentSize(romDataManager)) >= 1024)
    {
        stats->romSize += 1024;
    }

cleanup:
    tok_freeTokens(stats->tokenizer);
    if (upperCaseSourceCode)
    {
        free((void *)upperCaseSourceCode);
    }

    return error;
}
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

#include "core.h"
#include "core.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "core.h"

int data_calcOutputSize(struct DataManager *manager);

void data_init(struct DataManager *manager)
{
    data_reset(manager);
}

void data_deinit(struct DataManager *manager)
{
    assert(manager);

    if (manager->diskSourceCode)
    {
        free((void *)manager->diskSourceCode);
        manager->diskSourceCode = NULL;
    }
}

void data_reset(struct DataManager *manager)
{
    memset(manager->entries, 0, sizeof(struct DataEntry) * MAX_ENTRIES);

    strcpy(manager->entries[1].comment, "MAIN PALETTES");
    strcpy(manager->entries[2].comment, "MAIN CHARACTERS");
    strcpy(manager->entries[3].comment, "MAIN BG");
    strcpy(manager->entries[15].comment, "MAIN SOUND");

    if (manager->diskSourceCode)
    {
        free((void *)manager->diskSourceCode);
        manager->diskSourceCode = NULL;
    }
}

struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode)
{
    assert(manager);
    assert(input);

    const char *uppercaseInput = uppercaseString(input);
    if (!uppercaseInput) return err_makeCoreError(ErrorOutOfMemory, -1);

    struct CoreError error = data_uppercaseImport(manager, uppercaseInput, keepSourceCode);
    free((void *)uppercaseInput);

    return error;
}

struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode)
{
    assert(manager);
    assert(input);

    data_reset(manager);

    const char *character = input;
    uint8_t *currentDataByte = manager->data;
    uint8_t *endDataByte = &manager->data[DATA_SIZE];

    // skip stuff before
    const char *prevChar = NULL;
    while (*character && !(*character == '#' && (!prevChar || *prevChar == '\n')))
    {
        prevChar = character;
        character++;
    }

    if (keepSourceCode)
    {
        size_t length = (size_t)(character - input);

        char *diskSourceCode = malloc(length + 1);
        if (!diskSourceCode) exit(EXIT_FAILURE);

        stringConvertCopy(diskSourceCode, input, length);
        manager->diskSourceCode = diskSourceCode;
    }

    while (*character)
    {
        if (*character == '#')
        {
            character++;

            // entry index
            int entryIndex = 0;
            while (*character)
            {
                if (strchr(CharSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    entryIndex *= 10;
                    entryIndex += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            if (*character != ':') return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
            character++;

            if (entryIndex >= MAX_ENTRIES) return err_makeCoreError(ErrorIndexOutOfBounds, (int)(character - input));

            struct DataEntry *entry = &manager->entries[entryIndex];
            if (entry->length > 0) return err_makeCoreError(ErrorIndexAlreadyDefined, (int)(character - input));

            // file comment
            const char *comment = character;
            do
            {
                character++;
            }
            while (*character && *character != '\n' && *character != '\r');
            size_t commentLen = (character - comment);
            if (commentLen >= ENTRY_COMMENT_SIZE) commentLen = ENTRY_COMMENT_SIZE - 1;
            memset(entry->comment, 0, ENTRY_COMMENT_SIZE);
            strncpy(entry->comment, comment, commentLen);

            // binary data
            uint8_t *startByte = currentDataByte;
            bool shift = true;
            int value = 0;
            while (*character && *character != '#')
            {
                char *spos = strchr(CharSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - CharSetHex);
                    if (shift)
                    {
                        value = digit << 4;
                    }
                    else
                    {
                        value |= digit;
                        if (currentDataByte >= endDataByte) return err_makeCoreError(ErrorRomIsFull, (int)(character - input));
                        *currentDataByte = value;
                        ++currentDataByte;
                    }
                    shift = !shift;
                }
                else if (*character != ' ' && *character != '\t' && *character != '\n' && *character != '\r')
                {
                    return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
                }
                character++;
            }
            if (!shift) return err_makeCoreError(ErrorSyntax, (int)(character - input)); // incomplete hex value

            int start = (int)(startByte - manager->data);
            int length = (int)(currentDataByte - startByte);
            entry->start = start;
            entry->length = length;

            for (int i = entryIndex + 1; i < MAX_ENTRIES; i++)
            {
                manager->entries[i].start = entry->start + entry->length;
            }
        }
        else if (*character == ' ' || *character == '\t' || *character == '\n' || *character == '\r')
        {
            character++;
        }
        else
        {
            return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
        }
    }
    return err_noCoreError();
}

char *data_export(struct DataManager *manager)
{
    assert(manager);

    size_t outputSize = data_calcOutputSize(manager);
    if (outputSize > 0)
    {
        char *output = malloc(outputSize);
        if (output)
        {
            char *current = output;

            if (manager->diskSourceCode)
            {
                size_t len = strlen(manager->diskSourceCode);
                if (len > 0)
                {
                    strcpy(current, manager->diskSourceCode);
                    char endChar = current[len - 1];
                    current += len;
                    if (endChar != '\n')
                    {
                        // add new line after end of program
                        current[0] = '\n';
                        current++;
                    }
                }
            }

            for (int i = 0; i < MAX_ENTRIES; i++)
            {
                struct DataEntry *entry = &manager->entries[i];
                if (entry->length > 0)
                {
                    sprintf(current, "#%d:%s\n", i, entry->comment);
                    current += strlen(current);
                    int valuesInLine = 0;
                    int pos = 0;
                    uint8_t *entryData = &manager->data[entry->start];
                    while (pos < entry->length)
                    {
                        sprintf(current, "%02X", entryData[pos]);
                        current += strlen(current);
                        pos++;
                        valuesInLine++;
                        if (pos == entry->length)
                        {
                            sprintf(current, "\n\n");
                        }
                        else if (valuesInLine == 16)
                        {
                            sprintf(current, "\n");
                            valuesInLine = 0;
                        }
                        current += strlen(current);
                    }

                }
            }
        }
        return output;
    }
    return NULL;
}

int data_calcOutputSize(struct DataManager *manager)
{
    int size = 0;
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        struct DataEntry *entry = &manager->entries[i];
        if (entry->length > 0)
        {
            size += (i >= 10 ? 4 : 3) + strlen(entry->comment) + 1; // #10:comment\n
            size += entry->length * 2; // 2x hex letters
            size += entry->length / 16 + 1; // new line every 16 values
            size += 1; // new line
        }
    }
    if (manager->diskSourceCode)
    {
        size += strlen(manager->diskSourceCode) + 1; // possible new line between program and data
    }
    size += 1; // 0-byte
    return size;
}

int data_currentSize(struct DataManager *manager)
{
    int size = 0;
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        size += manager->entries[i].length;
    }
    return size;
}

bool data_canSetEntry(struct DataManager *manager, int index, int length)
{
    int size = 0;
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        if (i != index)
        {
            size += manager->entries[i].length;
        }
    }
    return size + length <= DATA_SIZE;
}

void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length)
{
    struct DataEntry *entry = &manager->entries[index];
    uint8_t *data = manager->data;

    // move data of higher entries
    int nextStart = entry->start + length;
    assert(nextStart <= DATA_SIZE);

    if (length > entry->length) // new entry is bigger
    {
        int diff = length - entry->length;
        for (int i = DATA_SIZE - 1; i >= nextStart; i--)
        {
            data[i] = data[i - diff];
        }
    }
    else if (length < entry->length) // new entry is smaller
    {
        int diff = entry->length - length;
        for (int i = nextStart; i < DATA_SIZE - diff; i++)
        {
            data[i] = data[i + diff];
        }
        for (int i = DATA_SIZE - diff; i < DATA_SIZE; i++)
        {
            data[i] = 0;
        }
    }

    // write new entry
    strncpy(entry->comment, comment, ENTRY_COMMENT_SIZE);
    entry->comment[ENTRY_COMMENT_SIZE - 1] = 0;
    entry->length = length;
    int start = entry->start;
    for (int i = 0; i < length; i++)
    {
        data[i + start] = source[i];
    }

    // move entry positions
    for (int i = index + 1; i < MAX_ENTRIES; i++)
    {
        struct DataEntry *thisEntry = &manager->entries[i];
        struct DataEntry *prevEntry = &manager->entries[i - 1];
        thisEntry->start = prevEntry->start + prevEntry->length;
    }
}
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

#include "core.h"

const char *CharSetDigits = "0123456789";
const char *CharSetLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
const char *CharSetAlphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
const char *CharSetHex = "0123456789ABCDEF";
//
// Copyright 2018 Timo Kloss
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

#include "core.h"
#include "core.h"
#include "core.h"

enum ErrorCode cmd_SOUND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SOUND
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // wave value
    struct TypedValue waveValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
    if (waveValue.type == ValueTypeError) return waveValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // pulse width value
    struct TypedValue pwValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (pwValue.type == ValueTypeError) return pwValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // length value
    struct TypedValue lenValue = itp_evaluateOptionalNumericExpression(core, 0, 255);
    if (lenValue.type == ValueTypeError) return lenValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (waveValue.type != ValueTypeNull)
        {
            voice->attr.wave = waveValue.v.floatValue;
        }
        if (pwValue.type != ValueTypeNull)
        {
            voice->attr.pulseWidth = pwValue.v.floatValue;
        }
        if (lenValue.type != ValueTypeNull)
        {
            int len = lenValue.v.floatValue;
            voice->length = len;
            voice->attr.timeout = (len > 0) ? 1 : 0;
        }
    }

    return itp_endOfCommand(interpreter);
}

//enum ErrorCode cmd_SOUND_COPY(struct Core *core)
//{
//    struct Interpreter *interpreter = core->interpreter;
//
//    // SOUND COPY
//    ++interpreter->pc;
//    ++interpreter->pc;
//
//    // sound value
//    struct TypedValue sValue = itp_evaluateNumericExpression(core, 0, 15);
//    if (sValue.type == ValueTypeError) return sValue.v.errorCode;
//
//    // TO
//    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
//    ++interpreter->pc;
//
//    // voice value
//    struct TypedValue vValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
//    if (vValue.type == ValueTypeError) return vValue.v.errorCode;
//
//    if (interpreter->pass == PassRun)
//    {
//        audlib_copySound(&interpreter->audioLib, interpreter->audioLib.sourceAddress, sValue.v.floatValue, vValue.v.floatValue);
//    }
//
//    return itp_endOfCommand(interpreter);
//}

enum ErrorCode cmd_VOLUME(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // VOLUME
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // volume value
    struct TypedValue volValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (volValue.type == ValueTypeError) return volValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // mix value
    struct TypedValue mixValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
    if (mixValue.type == ValueTypeError) return mixValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (volValue.type != ValueTypeNull)
        {
            voice->status.volume = volValue.v.floatValue;
        }
        if (mixValue.type != ValueTypeNull)
        {
            int mix = mixValue.v.floatValue;
            voice->status.mix = mix;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ENVELOPE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // ENVELOPE
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // attack value
    struct TypedValue attValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (attValue.type == ValueTypeError) return attValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // decay value
    struct TypedValue decValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (decValue.type == ValueTypeError) return decValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // sustain value
    struct TypedValue susValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (susValue.type == ValueTypeError) return susValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // release value
    struct TypedValue relValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (relValue.type == ValueTypeError) return relValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (attValue.type != ValueTypeNull)
        {
            voice->envA = attValue.v.floatValue;
        }
        if (decValue.type != ValueTypeNull)
        {
            voice->envD = decValue.v.floatValue;
        }
        if (susValue.type != ValueTypeNull)
        {
            voice->envS = susValue.v.floatValue;
        }
        if (relValue.type != ValueTypeNull)
        {
            voice->envR = relValue.v.floatValue;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LFO(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // LFO
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // rate value
    struct TypedValue rateValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (rateValue.type == ValueTypeError) return rateValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // osc amount value
    struct TypedValue oscValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (oscValue.type == ValueTypeError) return oscValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // vol amount value
    struct TypedValue volValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (volValue.type == ValueTypeError) return volValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // pw amount value
    struct TypedValue pwValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (pwValue.type == ValueTypeError) return pwValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (rateValue.type != ValueTypeNull)
        {
            voice->lfoFrequency = rateValue.v.floatValue;
        }
        if (oscValue.type != ValueTypeNull)
        {
            voice->lfoOscAmount = oscValue.v.floatValue;
        }
        if (volValue.type != ValueTypeNull)
        {
            voice->lfoVolAmount = volValue.v.floatValue;
        }
        if (pwValue.type != ValueTypeNull)
        {
            voice->lfoPWAmount = pwValue.v.floatValue;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LFO_A(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // LFO.A
    ++interpreter->pc;

    // obsolete syntax!

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    struct Voice *voice = NULL;
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        voice = &core->machine->audioRegisters.voices[n];
    }

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    union LFOAttributes attr;
    if (voice) attr = voice->lfoAttr; else attr.value = 0;

    // attr value
    struct TypedValue attrValue = itp_evaluateLFOAttributes(core, attr);
    if (attrValue.type == ValueTypeError) return attrValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        voice->lfoAttr.value = attrValue.v.floatValue;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LFO_WAVE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // LFO WAVE
    ++interpreter->pc;
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // wave value
    struct TypedValue wavValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
    if (wavValue.type == ValueTypeError) return wavValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // inv value
    struct TypedValue invValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
    if (invValue.type == ValueTypeError) return invValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // env value
    struct TypedValue envValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
    if (envValue.type == ValueTypeError) return envValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // tri value
    struct TypedValue triValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
    if (triValue.type == ValueTypeError) return triValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];

        if (wavValue.type != ValueTypeNull) voice->lfoAttr.wave = wavValue.v.floatValue;
        if (invValue.type != ValueTypeNull) voice->lfoAttr.invert = invValue.v.floatValue ? 1 : 0;
        if (envValue.type != ValueTypeNull) voice->lfoAttr.envMode = envValue.v.floatValue ? 1 : 0;
        if (triValue.type != ValueTypeNull) voice->lfoAttr.trigger = triValue.v.floatValue ? 1 : 0;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PLAY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // PLAY
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // pitch value
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 96);
    if (pValue.type == ValueTypeError) return pValue.v.errorCode;

    int len = -1;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;

        // length value
        struct TypedValue lenValue = itp_evaluateNumericExpression(core, 0, 255);
        if (lenValue.type == ValueTypeError) return lenValue.v.errorCode;

        len = lenValue.v.floatValue;
    }

    int sound = -1;
    if (interpreter->pc->type == TokenSOUND)
    {
        // SOUND
        ++interpreter->pc;

        // length value
        struct TypedValue sValue = itp_evaluateNumericExpression(core, 0, NUM_SOUNDS - 1);
        if (sValue.type == ValueTypeError) return sValue.v.errorCode;

        sound = sValue.v.floatValue;
    }

    if (interpreter->pass == PassRun)
    {
        audlib_play(&core->interpreter->audioLib, nValue.v.floatValue, pValue.v.floatValue, len, sound);
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_STOP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // STOP
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        if (nValue.type != ValueTypeNull)
        {
            int n = nValue.v.floatValue;
            audlib_stopVoice(&interpreter->audioLib, n);
        }
        else
        {
            audlib_stopAll(&interpreter->audioLib);
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_MUSIC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // MUSIC
    ++interpreter->pc;

    // pattern value
    struct TypedValue pValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_PATTERNS - 1);
    if (pValue.type == ValueTypeError) return pValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int startPattern = (pValue.type != ValueTypeNull) ? pValue.v.floatValue : 0;
        audlib_playMusic(&interpreter->audioLib, startPattern);
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TRACK(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // TRACK
    ++interpreter->pc;

    // track value
    struct TypedValue tValue = itp_evaluateNumericExpression(core, 0, NUM_TRACKS - 1);
    if (tValue.type == ValueTypeError) return tValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // voice value
    struct TypedValue vValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (vValue.type == ValueTypeError) return vValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        audlib_playTrack(&interpreter->audioLib, tValue.v.floatValue, vValue.v.floatValue);
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SOUND_SOURCE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SOUND
    ++interpreter->pc;

    // SOURCE
    ++interpreter->pc;

    // address value
    struct TypedValue aValue = itp_evaluateNumericExpression(core, 0, VM_MAX);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        interpreter->audioLib.sourceAddress = aValue.v.floatValue;
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_MUSIC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // MUSIC
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        int x = xValue.v.floatValue;
        struct ComposerPlayer *player = &interpreter->audioLib.musicPlayer;
        switch (x)
        {
            case 0:
                value.v.floatValue = player->index;
                break;
            case 1:
                value.v.floatValue = player->row;
                break;
            case 2:
                value.v.floatValue = player->tick;
                break;
            case 3:
                value.v.floatValue = player->speed;
                break;
            default:
                return val_makeError(ErrorInvalidParameter);
        }
    }
    return value;
}
//
// Copyright 2017-2019 Timo Kloss
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

#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include <assert.h>
#include <math.h>

enum ErrorCode cmd_BG(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG
	++interpreter->pc;

	// bg value
	struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
	if (bgValue.type == ValueTypeError)
		return bgValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		interpreter->textLib.bg = bgValue.v.floatValue;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_SOURCE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG SOURCE
	++interpreter->pc;
	++interpreter->pc;

	// address value
	struct TypedValue aValue = itp_evaluateNumericExpression(core, 0, VM_MAX);
	if (aValue.type == ValueTypeError)
		return aValue.v.errorCode;

	int w = 0;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// width value
		struct TypedValue wValue = itp_evaluateNumericExpression(core, 1, VM_MAX);
		if (wValue.type == ValueTypeError)
			return wValue.v.errorCode;

		w = wValue.v.floatValue;
	}

	int h = 0;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// height value
		struct TypedValue hValue = itp_evaluateNumericExpression(core, 1, VM_MAX);
		if (hValue.type == ValueTypeError)
			return hValue.v.errorCode;

		h = hValue.v.floatValue;
	}

	if (interpreter->pass == PassRun)
	{
		int address = aValue.v.floatValue;
		if (w > 0)
		{
			core->interpreter->textLib.sourceAddress = address;
			core->interpreter->textLib.sourceWidth = w;
			core->interpreter->textLib.sourceHeight = h;
		}
		else
		{
			// data with preceding size (W x H)
			core->interpreter->textLib.sourceAddress = address + 4;
			core->interpreter->textLib.sourceWidth = machine_peek(core, address + 2);
			core->interpreter->textLib.sourceHeight = machine_peek(core, address + 3);
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_COPY(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG COPY
	++interpreter->pc;
	++interpreter->pc;

	// src X value
	struct TypedValue srcXValue = itp_evaluateNumericExpression(core, 0, VM_MAX);
	if (srcXValue.type == ValueTypeError)
		return srcXValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// src Y value
	struct TypedValue srcYValue = itp_evaluateNumericExpression(core, 0, VM_MAX);
	if (srcYValue.type == ValueTypeError)
		return srcYValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// width value
	struct TypedValue wValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS);
	if (wValue.type == ValueTypeError)
		return wValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// height value
	struct TypedValue hValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS);
	if (hValue.type == ValueTypeError)
		return hValue.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// dst X value
	struct TypedValue dstXValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (dstXValue.type == ValueTypeError)
		return dstXValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// dst Y value
	struct TypedValue dstYValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (dstYValue.type == ValueTypeError)
		return dstYValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		txtlib_copyBackground(&interpreter->textLib, srcXValue.v.floatValue, srcYValue.v.floatValue, wValue.v.floatValue, hValue.v.floatValue, dstXValue.v.floatValue, dstYValue.v.floatValue);
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_SCROLL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG SCROLL
	++interpreter->pc;
	++interpreter->pc;

	// x1 value
	struct TypedValue x1Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
	if (x1Value.type == ValueTypeError)
		return x1Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y1 value
	struct TypedValue y1Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
	if (y1Value.type == ValueTypeError)
		return y1Value.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// x2 value
	struct TypedValue x2Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
	if (x2Value.type == ValueTypeError)
		return x2Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y2 value
	struct TypedValue y2Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
	if (y2Value.type == ValueTypeError)
		return y2Value.v.errorCode;

	// STEP
	if (interpreter->pc->type != TokenSTEP)
		return ErrorSyntax;
	++interpreter->pc;

	// dx value
	struct TypedValue dxValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (dxValue.type == ValueTypeError)
		return dxValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// dy value
	struct TypedValue dyValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (dyValue.type == ValueTypeError)
		return dyValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		txtlib_scrollBackground(&interpreter->textLib, x1Value.v.floatValue, y1Value.v.floatValue, x2Value.v.floatValue, y2Value.v.floatValue, dxValue.v.floatValue, dyValue.v.floatValue);
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ATTR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ATTR
	++interpreter->pc;

	// attributes value
	struct TypedValue aValue = itp_evaluateCharAttributes(core, interpreter->textLib.charAttr);
	if (aValue.type == ValueTypeError)
		return aValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		interpreter->textLib.charAttr.value = aValue.v.floatValue;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PAL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// PAL
	++interpreter->pc;

	// value
	struct TypedValue value = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		interpreter->textLib.charAttr.palette = value.v.floatValue;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FLIP(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// FLIP
	++interpreter->pc;

	// x value
	struct TypedValue fxValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
	// struct TypedValue fxValue = itp_evaluateNumericExpression(core, -1, 1);
	if (fxValue.type == ValueTypeError)
		return fxValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue fyValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
	// struct TypedValue fyValue = itp_evaluateNumericExpression(core, -1, 1);
	if (fyValue.type == ValueTypeError)
		return fyValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		if (fxValue.type == ValueTypeFloat) interpreter->textLib.charAttr.flipX = fxValue.v.floatValue == 0 ? 0 : 1;
		// interpreter->textLib.charAttr.flipX = fxValue.v.floatValue ? 1 : 0;
		if (fyValue.type == ValueTypeFloat) interpreter->textLib.charAttr.flipY = fyValue.v.floatValue == 0 ? 0 : 1;
		// interpreter->textLib.charAttr.flipY = fyValue.v.floatValue ? 1 : 0;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PRIO(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// PRIO
	++interpreter->pc;

	// value
	struct TypedValue value = itp_evaluateNumericExpression(core, -1, 1);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		interpreter->textLib.charAttr.priority = value.v.floatValue ? 1 : 0;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_FILL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG FILL
	++interpreter->pc;
	++interpreter->pc;

	// x1 value
	struct TypedValue x1Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (x1Value.type == ValueTypeError)
		return x1Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y1 value
	struct TypedValue y1Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (y1Value.type == ValueTypeError)
		return y1Value.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// x2 value
	struct TypedValue x2Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (x2Value.type == ValueTypeError)
		return x2Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y2 value
	struct TypedValue y2Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (y2Value.type == ValueTypeError)
		return y2Value.v.errorCode;

	// CHAR
	if (interpreter->pc->type == TokenCHAR)
	{
		++interpreter->pc;

		// write character with current attributes

		// character value
		struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_CHARACTERS - 1);
		if (cValue.type == ValueTypeError)
			return cValue.v.errorCode;

		if (interpreter->pass == PassRun)
		{
			txtlib_setCells(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), cValue.v.floatValue);
		}
	}
	else
	{
		// write current attributes (obsolete syntax!)

		if (interpreter->pass == PassRun)
		{
			txtlib_setCells(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), -1);
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_TINT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BG TINT
	++interpreter->pc;
	++interpreter->pc;

	// x1 value
	struct TypedValue x1Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (x1Value.type == ValueTypeError)
		return x1Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y1 value
	struct TypedValue y1Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (y1Value.type == ValueTypeError)
		return y1Value.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// x2 value
	struct TypedValue x2Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (x2Value.type == ValueTypeError)
		return x2Value.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y2 value
	struct TypedValue y2Value = itp_evaluateExpression(core, TypeClassNumeric);
	if (y2Value.type == ValueTypeError)
		return y2Value.v.errorCode;

	struct SimpleAttributes attrs;
	enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
	if (attrsError != ErrorNone)
		return attrsError;

	if (interpreter->pass == PassRun)
	{
		txtlib_setCellsAttr(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), attrs.pal, attrs.flipX, attrs.flipY, attrs.prio);
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CELL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// CELL
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// character value
	struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
	if (cValue.type == ValueTypeError)
		return cValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		int c = (cValue.type == ValueTypeFloat) ? cValue.v.floatValue : -1;
		txtlib_setCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue), c);
	}

	return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_CELL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// CELL.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		struct Cell *cell = txtlib_getCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
		if (type == TokenCELLA)
		{
			value.v.floatValue = cell->attr.value;
		}
		else if (type == TokenCELLC)
		{
			value.v.floatValue = cell->character;
		}
		else
		{
			assert(0);
		}
	}
	return value;
}

enum ErrorCode cmd_MCELL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// MCELL
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, interpreter->textLib.sourceWidth - 1);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, interpreter->textLib.sourceHeight - 1);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// character value
	struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
	if (cValue.type == ValueTypeError)
		return cValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		int c = (cValue.type == ValueTypeFloat) ? cValue.v.floatValue : -1;
		bool success = txtlib_setSourceCell(&interpreter->textLib, xValue.v.floatValue, yValue.v.floatValue, c);
		if (!success)
			return ErrorIllegalMemoryAccess;
	}

	return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_MCELL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// MCELL.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		int x = floorf(xValue.v.floatValue);
		int y = floorf(yValue.v.floatValue);
		value.v.floatValue = txtlib_getSourceCell(&interpreter->textLib, x, y, (type == TokenMCELLA));
	}
	return value;
}

enum ErrorCode cmd_TINT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// TINT
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	struct SimpleAttributes attrs;
	enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
	if (attrsError != ErrorNone)
		return attrsError;

	if (interpreter->pass == PassRun)
	{
		struct Cell *cell = txtlib_getCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
		if (attrs.pal >= 0)
			cell->attr.palette = attrs.pal;
		if (attrs.flipX >= 0)
			cell->attr.flipX = attrs.flipX;
		if (attrs.flipY >= 0)
			cell->attr.flipY = attrs.flipY;
		if (attrs.prio >= 0)
			cell->attr.priority = attrs.prio;
	}

	return itp_endOfCommand(interpreter);
}
//
// Copyright 2017-2020 Timo Kloss
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

#include "core.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_END(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// END
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		itp_endProgram(core);
		return ErrorNone;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_IF(struct Core *core, bool isAfterBlockElse)
{
	struct Interpreter *interpreter = core->interpreter;

	// IF
	struct Token *tokenIF = interpreter->pc;
	++interpreter->pc;

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	// THEN
	if (interpreter->pc->type != TokenTHEN)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		if (interpreter->pc->type == TokenEol)
		{
			// IF block
			if (interpreter->isSingleLineIf)
				return ErrorExpectedCommand;
			enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, isAfterBlockElse ? LabelTypeELSEIF : LabelTypeIF, tokenIF);
			if (errorCode != ErrorNone)
				return errorCode;

			// Eol
			++interpreter->pc;
		}
		else
		{
			// single line IF
			interpreter->isSingleLineIf = true;
			struct Token *token = interpreter->pc;
			while (token->type != TokenEol && token->type != TokenELSE)
			{
				token++;
			}
			tokenIF->jumpToken = token + 1; // after ELSE or Eol
		}
	}
	else if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenIF->jumpToken; // after ELSE or END IF, or Eol for single line
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_ELSE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ELSE
	struct Token *tokenELSE = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		if (interpreter->isSingleLineIf)
		{
			if (interpreter->pc->type == TokenEol)
				return ErrorExpectedCommand;
			struct Token *token = interpreter->pc;
			while (token->type != TokenEol)
			{
				token++;
			}
			tokenELSE->jumpToken = token + 1; // after Eol
		}
		else
		{
			struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
			if (!item)
				return ErrorElseWithoutIf;
			if (item->type == LabelTypeIF)
			{
				item->token->jumpToken = interpreter->pc;
			}
			else if (item->type == LabelTypeELSEIF)
			{
				item->token->jumpToken = interpreter->pc;

				item = lab_popLabelStackItem(interpreter);
				assert(item->type == LabelTypeELSE);
				item->token->jumpToken = tokenELSE;
			}
			else
			{
				return ErrorElseWithoutIf;
			}

			enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeELSE, tokenELSE);
			if (errorCode != ErrorNone)
				return errorCode;

			if (interpreter->pc->type == TokenIF)
			{
				return cmd_IF(core, true);
			}
			else
			{
				// Eol
				if (interpreter->pc->type != TokenEol)
					return ErrorSyntax;
				++interpreter->pc;
			}
		}
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenELSE->jumpToken; // after END IF, or Eol for single line
	}
	return ErrorNone;
}

enum ErrorCode cmd_END_IF(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// END IF
	++interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item)
		{
			return ErrorEndIfWithoutIf;
		}
		else if (item->type == LabelTypeIF || item->type == LabelTypeELSE)
		{
			item->token->jumpToken = interpreter->pc;
		}
		else if (item->type == LabelTypeELSEIF)
		{
			item->token->jumpToken = interpreter->pc;

			item = lab_popLabelStackItem(interpreter);
			assert(item->type == LabelTypeELSE);
			item->token->jumpToken = interpreter->pc;
		}
		else
		{
			return ErrorEndIfWithoutIf;
		}
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_FOR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// FOR
	struct Token *tokenFOR = interpreter->pc;
	++interpreter->pc;

	// Variable
	struct Token *tokenFORVar = interpreter->pc;
	enum ErrorCode errorCode = ErrorNone;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeFloat)
		return ErrorTypeMismatch;

	// Eq
	if (interpreter->pc->type != TokenEq)
		return ErrorSyntax;
	++interpreter->pc;

	// start value
	struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (startValue.type == ValueTypeError)
		return startValue.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// limit value
	struct Token *tokenFORLimit = interpreter->pc;
	struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (limitValue.type == ValueTypeError)
		return limitValue.v.errorCode;

	// STEP
	struct TypedValue stepValue;
	if (interpreter->pc->type == TokenSTEP)
	{
		++interpreter->pc;

		// step value
		stepValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (stepValue.type == ValueTypeError)
			return stepValue.v.errorCode;
	}
	else
	{
		stepValue.type = ValueTypeFloat;
		stepValue.v.floatValue = 1.0f;
	}

	if (interpreter->pass == PassPrepare)
	{
		lab_pushLabelStackItem(interpreter, LabelTypeFORLimit, tokenFORLimit);
		lab_pushLabelStackItem(interpreter, LabelTypeFORVar, tokenFORVar);
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeFOR, tokenFOR);
		if (errorCode != ErrorNone)
			return errorCode;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		varValue->floatValue = startValue.v.floatValue;

		// limit check
		if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
		{
			interpreter->pc = tokenFOR->jumpToken; // after NEXT's Eol
		}
		else
		{
			// Eol
			if (interpreter->pc->type != TokenEol)
				return ErrorSyntax;
			++interpreter->pc;
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_NEXT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct LabelStackItem *itemFORLimit = NULL;
	struct LabelStackItem *itemFORVar = NULL;
	struct LabelStackItem *itemFOR = NULL;

	if (interpreter->pass == PassPrepare)
	{
		itemFOR = lab_popLabelStackItem(interpreter);
		if (!itemFOR || itemFOR->type != LabelTypeFOR)
			return ErrorNextWithoutFor;

		itemFORVar = lab_popLabelStackItem(interpreter);
		assert(itemFORVar && itemFORVar->type == LabelTypeFORVar);

		itemFORLimit = lab_popLabelStackItem(interpreter);
		assert(itemFORLimit && itemFORLimit->type == LabelTypeFORLimit);
	}

	// NEXT
	struct Token *tokenNEXT = interpreter->pc;
	++interpreter->pc;

	// Variable
	enum ErrorCode errorCode = ErrorNone;
	struct Token *tokenVar = interpreter->pc;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeFloat)
		return ErrorTypeMismatch;

	if (interpreter->pass == PassPrepare)
	{
		if (tokenVar->symbolIndex != itemFORVar->token->symbolIndex)
			return ErrorNextWithoutFor;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		itemFOR->token->jumpToken = interpreter->pc;
		tokenNEXT->jumpToken = itemFORLimit->token;
	}
	else if (interpreter->pass == PassRun)
	{
		struct Token *storedPc = interpreter->pc;
		interpreter->pc = tokenNEXT->jumpToken;

		// limit value
		struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (limitValue.type == ValueTypeError)
			return limitValue.v.errorCode;

		// STEP
		struct TypedValue stepValue;
		if (interpreter->pc->type == TokenSTEP)
		{
			++interpreter->pc;

			// step value
			stepValue = itp_evaluateExpression(core, TypeClassNumeric);
			if (stepValue.type == ValueTypeError)
				return stepValue.v.errorCode;
		}
		else
		{
			stepValue.type = ValueTypeFloat;
			stepValue.v.floatValue = 1.0f;
		}

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;

		varValue->floatValue += stepValue.v.floatValue;

		// limit check
		if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
		{
			interpreter->pc = storedPc; // after NEXT's Eol
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_GOTO(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// GOTO
	struct Token *tokenGOTO = interpreter->pc;
	++interpreter->pc;

	// Identifier
	if (interpreter->pc->type != TokenIdentifier)
		return ErrorExpectedLabel;
	struct Token *tokenIdentifier = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
		if (!item)
			return ErrorUndefinedLabel;
		tokenGOTO->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenGOTO->jumpToken; // after label
	}
	return ErrorNone;
}

enum ErrorCode cmd_GOSUB(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// GOSUB
	struct Token *tokenGOSUB = interpreter->pc;
	++interpreter->pc;

	// Identifier
	if (interpreter->pc->type != TokenIdentifier)
		return ErrorExpectedLabel;
	struct Token *tokenIdentifier = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
		if (!item)
			return ErrorUndefinedLabel;
		tokenGOSUB->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeGOSUB, interpreter->pc);
		if (errorCode != ErrorNone)
			return errorCode;

		interpreter->pc = tokenGOSUB->jumpToken; // after label
	}
	return ErrorNone;
}

enum ErrorCode cmd_RETURN(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// RETURN
	struct Token *tokenRETURN = interpreter->pc;
	++interpreter->pc;

	// Identifier
	struct Token *tokenIdentifier = NULL;
	if (interpreter->pc->type == TokenIdentifier)
	{
		tokenIdentifier = interpreter->pc;
		++interpreter->pc;
	}

	if (interpreter->pass == PassPrepare)
	{
		if (tokenIdentifier)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenRETURN->jumpToken = item->token;
		}
	}
	else if (interpreter->pass == PassRun)
	{
		struct LabelStackItem *itemGOSUB = lab_popLabelStackItem(interpreter);
		if (!itemGOSUB)
			return ErrorReturnWithoutGosub;

		if (itemGOSUB->type == LabelTypeGOSUB)
		{
			if (tokenRETURN->jumpToken)
			{
				// jump to label
				interpreter->pc = tokenRETURN->jumpToken; // after label
				// clear stack
				interpreter->numLabelStackItems = 0;
			}
			else
			{
				// jump back
				interpreter->pc = itemGOSUB->token; // after GOSUB
			}
		}
		else
		{
			return ErrorReturnWithoutGosub;
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_WAIT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	struct Token *save = interpreter->pc;

	// WAIT
	++interpreter->pc;

	int wait = 0;
	if (interpreter->pc->type == TokenVBL)
	{
		// VBL
		++interpreter->pc;
	}
	else if (interpreter->pc->type == TokenTAP)
	{
		++interpreter->pc;
		if (interpreter->pass == PassRun)
		{
			interpreter->exitEvaluation = true;
			interpreter->waitTap = true;
		}
		return itp_endOfCommand(interpreter);
	}
	else
	{
		// value
		struct TypedValue value = itp_evaluateNumericExpression(core, 1, VM_MAX);
		if (value.type == ValueTypeError)
			return value.v.errorCode;
		wait = value.v.floatValue - 1;
	}

	if (interpreter->pass == PassRun)
	{
		interpreter->exitEvaluation = true;
		interpreter->waitCount = wait;
	}
	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ON(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ON
	++interpreter->pc;

	// RASTER/VBL/PARTICLE/EMITTER
	if (interpreter->pc->type == TokenRASTER || interpreter->pc->type == TokenVBL || interpreter->pc->type == TokenPARTICLE || interpreter->pc->type == TokenEMITTER)
	{
		enum TokenType type = interpreter->pc->type;
		++interpreter->pc;

		if (interpreter->pc->type == TokenOFF)
		{
			// OFF
			++interpreter->pc;

			if (interpreter->pass == PassRun)
			{
				if (type == TokenRASTER)
				{
					interpreter->currentOnRasterToken = NULL;
				}
				else if (type == TokenVBL)
				{
					interpreter->currentOnVBLToken = NULL;
				}
				else if (type == TokenPARTICLE)
				{
					interpreter->currentOnParticleToken = NULL;
				}
				else if (type == TokenEMITTER)
				{
					interpreter->currentOnEmitterToken = NULL;
				}
			}
		}
		else if (interpreter->pc->type == TokenCALL)
		{
			// CALL
			// if (interpreter->pc->type != TokenCALL) return ErrorSyntax;
			struct Token *tokenCALL = interpreter->pc;
			++interpreter->pc;

			// Identifier
			if (interpreter->pc->type != TokenIdentifier)
				return ErrorExpectedSubprogramName;
			struct Token *tokenIdentifier = interpreter->pc;
			++interpreter->pc;

			if (interpreter->pass == PassPrepare)
			{
				struct SubItem *item = tok_getSub(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
				if (!item)
					return ErrorUndefinedSubprogram;
				tokenCALL->jumpToken = item->token;
			}
			else if (interpreter->pass == PassRun)
			{
				if (type == TokenRASTER)
				{
					interpreter->currentOnRasterToken = tokenCALL->jumpToken;
				}
				else if (type == TokenVBL)
				{
					interpreter->currentOnVBLToken = tokenCALL->jumpToken;
				}
				else if (type == TokenPARTICLE)
				{
					interpreter->currentOnParticleToken = tokenCALL->jumpToken;
				}
				else if (type == TokenEMITTER)
				{
					interpreter->currentOnEmitterToken = tokenCALL->jumpToken;
				}
			}
		}
	}
	else
	{
		// n value
		struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 255);
		if (nValue.type == ValueTypeError)
			return nValue.v.errorCode;
		int n = nValue.v.floatValue;

		struct Token *tokenGOTO = NULL;
		struct Token *tokenGOSUB = NULL;
		struct Token *tokenRESTORE = NULL;

		if (interpreter->pc->type == TokenGOTO)
		{
			tokenGOTO = interpreter->pc;
			++interpreter->pc;
		}
		else if (interpreter->pc->type == TokenGOSUB)
		{
			tokenGOSUB = interpreter->pc;
			++interpreter->pc;
		}
		else if (interpreter->pc->type == TokenRESTORE)
		{
			tokenRESTORE = interpreter->pc;
			++interpreter->pc;
		}
		else
		{
			return ErrorSyntax;
		}

		if (interpreter->pass == PassRun)
			while (n > 0)
			{
				++interpreter->pc;
				if (interpreter->pc->type != TokenComma)
					return ErrorExpectedLabel;
				++interpreter->pc;
				--n;
			}

		// Identifier
		if (interpreter->pc->type != TokenIdentifier)
			return ErrorExpectedLabel;
		struct Token *tokenIdentifier = interpreter->pc;
		++interpreter->pc;

		while (interpreter->pc->type == TokenComma)
		{
			++interpreter->pc;
			if (interpreter->pc->type != TokenIdentifier)
				return ErrorSyntax;
			++interpreter->pc;
		}

		if (tokenGOTO)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenGOTO->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				interpreter->pc = tokenGOTO->jumpToken; // after label
			}
		}
		else if (tokenGOSUB)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenGOSUB->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeGOSUB, interpreter->pc);
				if (errorCode != ErrorNone)
					return errorCode;

				interpreter->pc = tokenGOSUB->jumpToken; // after label
			}
		}
		else if (tokenRESTORE)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenRESTORE->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				dat_restoreData(interpreter, tokenRESTORE->jumpToken);
			}
		}
		else
			return ErrorNone;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DO(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// DO
	struct Token *tokenDO = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeDO, tokenDO);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_LOOP(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// LOOP
	struct Token *tokenLOOP = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeDO)
			return ErrorLoopWithoutDo;

		tokenLOOP->jumpToken = item->token + 1;
		item->token->jumpToken = tokenLOOP + 1;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenLOOP->jumpToken; // after DO
	}

	return ErrorNone;
}

enum ErrorCode cmd_REPEAT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// REPEAT
	struct Token *tokenREPEAT = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeREPEAT, tokenREPEAT);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_UNTIL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// UNTIL
	struct Token *tokenUNTIL = interpreter->pc;
	++interpreter->pc;

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeREPEAT)
			return ErrorUntilWithoutRepeat;

		tokenUNTIL->jumpToken = item->token + 1;
		item->token->jumpToken = interpreter->pc;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenUNTIL->jumpToken; // after REPEAT
		}
		else
		{
			// Eol
			if (interpreter->pc->type != TokenEol)
				return ErrorSyntax;
			++interpreter->pc;
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_WHILE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// WHILE
	struct Token *tokenWHILE = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeWHILE, tokenWHILE);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenWHILE->jumpToken; // after WEND
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_WEND(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// WEND
	struct Token *tokenWEND = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeWHILE)
			return ErrorWendWithoutWhile;

		tokenWEND->jumpToken = item->token;
		item->token->jumpToken = tokenWEND + 1;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenWEND->jumpToken; // on WHILE
	}

	return ErrorNone;
}

enum ErrorCode cmd_EXIT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// EXIT
	struct Token *tokenEXIT = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum LabelType types[] = {LabelTypeFOR, LabelTypeDO, LabelTypeWHILE, LabelTypeREPEAT};
		struct LabelStackItem *item = lab_searchLabelStackItem(interpreter, types, 4);
		if (!item)
			return ErrorExitNotInsideLoop;

		tokenEXIT->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenEXIT->jumpToken->jumpToken;
	}

	return ErrorNone;
}

enum ErrorCode cmd_SYSTEM(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// SYSTEM
	++interpreter->pc;

	// type value
	struct TypedValue tValue = itp_evaluateNumericExpression(core, 0, 4);
	if (tValue.type == ValueTypeError)
		return tValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// setting value
	struct TypedValue sValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (sValue.type == ValueTypeError)
		return sValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		switch ((int)tValue.v.floatValue)
		{
		case 0:
			core->machineInternals->isEnergySaving = (sValue.v.floatValue != 0.0f);
			break;

		case 1:
			core->machineInternals->planeColor0IsOpaque[0] = (sValue.v.floatValue != 0.0f);
			break;

		case 2:
			core->machineInternals->planeColor0IsOpaque[1] = (sValue.v.floatValue != 0.0f);
			break;

		case 3:
			core->machineInternals->planeColor0IsOpaque[2] = (sValue.v.floatValue != 0.0f);
			break;

		case 4:
			core->machineInternals->planeColor0IsOpaque[3] = (sValue.v.floatValue != 0.0f);
			break;
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_COMPAT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// COMPAT
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		interpreter->compat = true;
	}

	return itp_endOfCommand(interpreter);
}
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

#include "core.h"
#include "core.h"

enum ErrorCode cmd_DATA(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    if (interpreter->pass == PassPrepare)
    {
        if (!interpreter->firstData)
        {
            interpreter->firstData = interpreter->pc;
        }
        if (interpreter->lastData)
        {
            interpreter->lastData->jumpToken = interpreter->pc;
        }
        interpreter->lastData = interpreter->pc;
    }

    do
    {
        ++interpreter->pc; // DATA at first, then comma

        if (interpreter->pc->type == TokenString)
        {
            ++interpreter->pc;
        }
        else if (interpreter->pc->type == TokenFloat)
        {
            ++interpreter->pc;
        }
        else if (interpreter->pc->type == TokenMinus)
        {
            ++interpreter->pc;
            if (interpreter->pc->type != TokenFloat) return ErrorSyntax;
            ++interpreter->pc;
        }
        else
        {
            return ErrorSyntax;
        }
    }
    while (interpreter->pc->type == TokenComma);

    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;

    return ErrorNone;
}

enum ErrorCode cmd_READ(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    do
    {
         // READ at first, then comma
        ++interpreter->pc;

        // variable
        enum ValueType varType = ValueTypeNull;
        enum ErrorCode errorCode = ErrorNone;
        union Value *varValue = itp_readVariable(core, &varType, &errorCode, true);
        if (!varValue) return errorCode;

        if (interpreter->pass == PassRun)
        {
            if (!interpreter->currentDataValueToken) return ErrorOutOfData;

            struct Token *dataValueToken = interpreter->currentDataValueToken;
            if (dataValueToken->type == TokenFloat)
            {
                if (varType != ValueTypeFloat) return ErrorTypeMismatch;
                varValue->floatValue = dataValueToken->floatValue;
            }
            else if (dataValueToken->type == TokenMinus)
            {
                if (varType != ValueTypeFloat) return ErrorTypeMismatch;
                interpreter->currentDataValueToken++;
                varValue->floatValue = -interpreter->currentDataValueToken->floatValue;
            }
            else if (dataValueToken->type == TokenString)
            {
                if (varType != ValueTypeString) return ErrorTypeMismatch;
                if (varValue->stringValue)
                {
                    rcstring_release(varValue->stringValue);
                }
                varValue->stringValue = dataValueToken->stringValue;
                rcstring_retain(varValue->stringValue);
            }

            dat_nextData(interpreter);
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SKIP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SKIP
    struct Token *tokenSKIP = interpreter->pc;
    ++interpreter->pc;

    // skip value
    struct TypedValue skipValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (skipValue.type == ValueTypeError) return skipValue.v.errorCode;
    int skip = (int)skipValue.v.floatValue;

    if (interpreter->pass == PassRun)
    {
				// TODO: must check if there is enough data
        while(skip-->0) dat_nextData(interpreter);
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_RESTORE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // RESTORE
    struct Token *tokenRESTORE = interpreter->pc;
    ++interpreter->pc;

    // optional jump label
    if (interpreter->pc->type == TokenIdentifier)
    {
        if (interpreter->pass == PassPrepare)
        {
            struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, interpreter->pc->symbolIndex);
            if (!item) return ErrorUndefinedLabel;
            tokenRESTORE->jumpToken = item->token;
        }
        else if (interpreter->pass == PassRun)
        {
            // find DATA after label
            dat_restoreData(interpreter, tokenRESTORE->jumpToken);
        }
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        // restore to first DATA
        dat_restoreData(interpreter, NULL);
    }

    return itp_endOfCommand(interpreter);
}
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

#include "core.h"
#include "core.h"
#include <assert.h>
#include <string.h>

enum ErrorCode cmd_LOAD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;

    // LOAD
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;

    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    int maxLength = 0;
    int offset = 0;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;

        // max length value
        struct TypedValue maxLengthValue = itp_evaluateNumericExpression(core, 0, DATA_SIZE);
        if (maxLengthValue.type == ValueTypeError) return maxLengthValue.v.errorCode;
        maxLength = maxLengthValue.v.floatValue;

        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;

            // offset value
            struct TypedValue offsetValue = itp_evaluateNumericExpression(core, 0, DATA_SIZE);
            if (offsetValue.type == ValueTypeError) return offsetValue.v.errorCode;
            offset = offsetValue.v.floatValue;
        }
    }

    if (interpreter->pass == PassRun)
    {
        bool pokeFailed = false;
        bool ready = disk_loadFile(core, fileValue.v.floatValue, addressValue.v.floatValue, maxLength, offset, &pokeFailed);
        if (pokeFailed) return ErrorIllegalMemoryAccess;

        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SAVE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;

    // SAVE
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;

    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // comment value
    struct TypedValue commentValue = itp_evaluateExpression(core, TypeClassString);
    if (commentValue.type == ValueTypeError) return commentValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // length value
    struct TypedValue lengthValue = itp_evaluateNumericExpression(core, 1, DATA_SIZE);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int address = addressValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        if (address + length > VM_SIZE)
        {
            return ErrorIllegalMemoryAccess;
        }
        bool ready = disk_saveFile(core, fileValue.v.floatValue, commentValue.v.stringValue->chars, address, length);
        rcstring_release(commentValue.v.stringValue);

        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FILES(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;

    // FILES
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassRun)
    {
        bool ready = disk_prepare(core);

        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_FILE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // FILE$
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;

    if (interpreter->pass == PassRun)
    {
        if (core->diskDrive->dataManager.data == NULL) return val_makeError(ErrorDirectoryNotLoaded);

        int index = fileValue.v.floatValue;
        struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];

        size_t len = strlen(entry->comment);
        resultValue.v.stringValue = rcstring_new(entry->comment, len);
        rcstring_retain(resultValue.v.stringValue);
        interpreter->cycles += len;
    }
    return resultValue;
}

struct TypedValue fnc_FSIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // FSIZE
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        if (core->diskDrive->dataManager.data == NULL) return val_makeError(ErrorDirectoryNotLoaded);

        int index = fileValue.v.floatValue;
        struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];

        resultValue.v.floatValue = entry->length;
    }
    return resultValue;
}
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

#include "core.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_KEYBOARD(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// KEYBOARD
	++interpreter->pc;

	// ON/OFF
	enum TokenType type = interpreter->pc->type;
	if (type != TokenON && type != TokenOFF)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		core->machine->ioRegisters.status.keyboardEnabled = (type == TokenON);
		delegate_controlsDidChange(core);
	}

	return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_KEYBOARD(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// KEYBOARD
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		value.v.floatValue = core->machine->ioRegisters.status.keyboardEnabled > 0 ? -1 : 0;
	}
	return value;
}

enum ErrorCode cmd_PAUSE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// PAUSE
	++interpreter->pc;

	// ON/OFF?
	enum TokenType type = interpreter->pc->type;
	if (type == TokenON || type == TokenOFF)
	{
		++interpreter->pc;
	}

	if (interpreter->pass == PassRun)
	{
		if (type == TokenON)
		{
			core->machine->ioRegisters.status.pause = 0;
			interpreter->handlesPause = true;
		}
		else if (type == TokenOFF)
		{
			interpreter->handlesPause = false;
		}
		else
		{
			interpreter->state = StatePaused;
			overlay_updateState(core);
		}
	}
	return itp_endOfCommand(interpreter);
}

// struct TypedValue fnc_UP_DOWN_LEFT_RIGHT(struct Core *core)
// {
//     struct Interpreter *interpreter = core->interpreter;

//     // UP/DOWN/LEFT/RIGHT
//     enum TokenType type = interpreter->pc->type;
//     ++interpreter->pc;

//     // TAP
//     bool tap = false;
//     if (interpreter->pc->type == TokenTAP)
//     {
//         ++interpreter->pc;
//         tap = true;
//     }

//     // bracket open
//     if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
//     ++interpreter->pc;

//     // p expression
//     struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
//     if (pValue.type == ValueTypeError) return pValue;

//     // bracket close
//     if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
//     ++interpreter->pc;

//     struct TypedValue value;
//     value.type = ValueTypeFloat;

//     if (interpreter->pass == PassRun)
//     {
//         if (core->machine->ioRegisters.attr.gamepadsEnabled == 0) return val_makeError(ErrorGamepadNotEnabled);

//         int p = pValue.v.floatValue;
//         int active = 0;
//         int lastFrameActive = 0;
//         union Gamepad *gamepad = &core->machine->ioRegisters.gamepads[p];
//         union Gamepad *lastFrameGamepad = &core->interpreter->lastFrameGamepads[p];
//         switch (type)
//         {
//             case TokenUP:
//                 active = gamepad->up;
//                 lastFrameActive = lastFrameGamepad->up;
//                 break;

//             case TokenDOWN:
//                 active = gamepad->down;
//                 lastFrameActive = lastFrameGamepad->down;
//                 break;

//             case TokenLEFT:
//                 active = gamepad->left;
//                 lastFrameActive = lastFrameGamepad->left;
//                 break;

//             case TokenRIGHT:
//                 active = gamepad->right;
//                 lastFrameActive = lastFrameGamepad->right;
//                 break;

//             default:
//                 assert(0);
//                 break;
//         }
//         value.v.floatValue = active && !(tap && lastFrameActive) ? BAS_TRUE : BAS_FALSE;
//     }
//     return value;
// }

// TODO: deleteme

// struct TypedValue fnc_BUTTON(struct Core *core)
// {
//     struct Interpreter *interpreter = core->interpreter;

//     // BUTTON
//     ++interpreter->pc;

//     // TAP
//     bool tap = false;
//     if (interpreter->pc->type == TokenTAP)
//     {
//         ++interpreter->pc;
//         tap = true;
//     }

//     // bracket open
//     if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
//     ++interpreter->pc;

//     // p expression
//     struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
//     if (pValue.type == ValueTypeError) return pValue;

//     int n = -1;
//     if (interpreter->pc->type == TokenComma)
//     {
//         // comma
//         ++interpreter->pc;

//         // n expression
//         struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 1);
//         if (nValue.type == ValueTypeError) return nValue;

//         n = nValue.v.floatValue;
//     }

//     // bracket close
//     if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
//     ++interpreter->pc;

//     struct TypedValue value;
//     value.type = ValueTypeFloat;

//     if (interpreter->pass == PassRun)
//     {
//         int p = pValue.v.floatValue;
//         union Gamepad *gamepad = &core->machine->ioRegisters.gamepads[p];

//         int active = (n == -1) ? (gamepad->buttonA || gamepad->buttonB) : (n == 0) ? gamepad->buttonA : gamepad->buttonB;

//         if (active && tap)
//         {
//             // invalidate button if it was already pressed last frame
//             union Gamepad *lastFrameGamepad = &core->interpreter->lastFrameGamepads[p];
//             if ((n == -1) ? (lastFrameGamepad->buttonA || lastFrameGamepad->buttonB) : (n == 0) ? lastFrameGamepad->buttonA : lastFrameGamepad->buttonB)
//             {
//                 active = 0;
//             }
//         }

//         value.v.floatValue = active ? BAS_TRUE : BAS_FALSE;
//     }
//     return value;
// }

struct TypedValue fnc_TOUCH(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// TOUCH
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		value.v.floatValue = core->machine->ioRegisters.status.touch ? BAS_TRUE : BAS_FALSE;
	}
	return value;
}

struct TypedValue fnc_TAP(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// TAP
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		value.v.floatValue = (core->machine->ioRegisters.status.touch && !core->interpreter->lastFrameIOStatus.touch) ? BAS_TRUE : BAS_FALSE;
	}
	return value;
}

struct TypedValue fnc_TOUCH_X_Y(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// TOUCH.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		if (type == TokenTOUCHX)
		{
			value.v.floatValue = core->machine->ioRegisters.touchX;
		}
		else if (type == TokenTOUCHY)
		{
			value.v.floatValue = core->machine->ioRegisters.touchY;
		}
		else
		{
			assert(0);
		}
	}
	return value;
}

struct TypedValue fnc_SHOWN(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// SHOWN.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		if (type == TokenSHOWNW)
		{
			value.v.floatValue = core->machine->ioRegisters.shown.width;
		}
		else if (type == TokenSHOWNH)
		{
			value.v.floatValue = core->machine->ioRegisters.shown.height;
		}
		else
		{
			assert(0);
		}
	}
	return value;
}

struct TypedValue fnc_SAFE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// SAFE.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		if (type == TokenSAFEL)
		{
			value.v.floatValue = core->machine->ioRegisters.safe.left;
		}
		else if (type == TokenSAFET)
		{
			value.v.floatValue = core->machine->ioRegisters.safe.top;
		}
		else if (type == TokenSAFER)
		{
			value.v.floatValue = core->machine->ioRegisters.safe.right;
		}
		else if (type == TokenSAFEB)
		{
			value.v.floatValue = core->machine->ioRegisters.safe.bottom;
		}
		else
		{
			assert(0);
		}
	}
	return value;
}

struct TypedValue fnc_PAUSE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// PAUSE
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		if (interpreter->handlesPause)
			return val_makeError(ErrorAutomaticPauseNotDisabled);

		value.v.floatValue = core->machine->ioRegisters.status.pause ? BAS_TRUE : BAS_FALSE;
		core->machine->ioRegisters.status.pause = 0;
	}
	return value;
}
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

#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"

#define _USE_MATH_DEFINES
#ifndef __USE_MISC
  #define __USE_MISC
#endif

#include <math.h>
#include <assert.h>
#include <stdlib.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

struct TypedValue fnc_math0(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPI:
                value.v.floatValue = 3.14159265358979323846264338327950288;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math1(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenABS:
                value.v.floatValue = fabsf(xValue.v.floatValue);
                break;

            case TokenACOS:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = acosf(xValue.v.floatValue);
                break;

            case TokenASIN:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = asinf(xValue.v.floatValue);
                break;

            case TokenATAN:
                value.v.floatValue = atanf(xValue.v.floatValue);
                break;

            case TokenCOS:
                value.v.floatValue = cosf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenEXP:
                value.v.floatValue = expf(xValue.v.floatValue);
                break;

            case TokenHCOS:
                value.v.floatValue = coshf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenHSIN:
                value.v.floatValue = sinhf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenHTAN:
                value.v.floatValue = tanhf(xValue.v.floatValue);
                break;

            case TokenINT:
                value.v.floatValue = floorf(xValue.v.floatValue);
                break;

            case TokenLOG:
                if (xValue.v.floatValue <= 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = logf(xValue.v.floatValue);
                break;

            case TokenSGN:
                value.v.floatValue = (xValue.v.floatValue > 0) ? 1 : (xValue.v.floatValue < 0) ? BAS_TRUE : BAS_FALSE;
                break;

            case TokenSIN:
                value.v.floatValue = sinf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenSQR:
                if (xValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = sqrtf(xValue.v.floatValue);
                break;

            case TokenTAN:
                value.v.floatValue = tanf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenCEIL:
                    value.v.floatValue = ceilf(xValue.v.floatValue);
                    break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math2(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        float x = xValue.v.floatValue;
        float y = yValue.v.floatValue;

        switch (type)
        {
            case TokenMAX:
                value.v.floatValue = (x > y) ? x : y;
                break;

            case TokenMIN:
                value.v.floatValue = (x < y) ? x : y;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math3(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // z expression
    struct TypedValue zValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (zValue.type == ValueTypeError) return zValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        float x = xValue.v.floatValue;
        float y = yValue.v.floatValue;
        float z = zValue.v.floatValue;

        switch (type)
        {
            case TokenCLAMP:
                value.v.floatValue = (x < y) ? y : (x > z) ? z : x;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

float easeOutBounce(float x) {

  if (x < 1 / 2.75) return (7.5625 * x * x);
  if (x < 2 / 2.75) {
    x = x - (1.5 / 2.75);
    return (7.5625 * x * x + 0.75);
  }
  else if (x < 2.5 / 2.75) {
    x = x - (2.25 / 2.75);
    return (7.5625 * x * x + 0.9375);
  }
  x = x - (2.625 / 2.75);
  return (7.5625 * x * x + 0.984375);
}

float easeInBounce(float x) {
  return 1 - easeOutBounce(1 - x);
}

float easeInOutBounce(float x) {
  return x < 0.5
    ? (1 - easeOutBounce(1 - 2 * x)) / 2
    : (1 + easeOutBounce(2 * x - 1)) / 2;
}

struct TypedValue fnc_EASE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // easing value
    struct TypedValue easing = itp_evaluateNumericExpression(core, 0, 9);
    if (easing.type == ValueTypeError) return easing;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // inout value
    struct TypedValue inout = itp_evaluateNumericExpression(core, -1, 1);
    if (inout.type == ValueTypeError) return inout;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    int e=(int)easing.v.floatValue;
    int io=(int)inout.v.floatValue;
    float x=xValue.v.floatValue;
    float v;

    x = x>1.0f ? 1.0f : x<0.0f ? 0.0f : x;

    const float c1 = 1.70158;
    const float c2 = c1 * 1.525;
    const float c3 = c1 + 1;
    const float c4 = (2 * M_PI) / 3;
    const float c5 = (2 * M_PI) / 4.5;

    // linear
    if (e==0) v = xValue.v.floatValue;

    // sine
    else if(e==1 && io<0) v = 1 - cosf((x * M_PI) / 2);
    else if(e==1 && io==0) v = -(cosf(M_PI * x) - 1) / 2;
    else if(e==1 && io>0) v = sinf((x * M_PI) / 2);

    // quad
    else if(e==2 && io<0) v =  x * x;
    else if(e==2 && io==0) v = x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
    else if(e==2 && io>0) v = 1 - (1 - x) * (1 - x);

    // cubic
    else if(e==3 && io<0) v = x * x * x;
    else if(e==3 && io==0) v = x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
    else if(e==3 && io>0) v = 1 - pow(1 - x, 3);

    // quart
    else if(e==4 && io<0) v = x * x * x * x;
    else if(e==4 && io==0) v = x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
    else if(e==4 && io>0) v = 1 - pow(1 - x, 4);

    // quint
    else if(e==5 && io<0) v =  x * x * x * x * x;
    else if(e==5 && io==0) v = x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
    else if(e==5 && io>0) v = 1 - pow(1 - x, 5);

    // circ
    else if(e==6 && io<0) v = 1 - sqrt(1 - pow(x, 2));
    else if(e==6 && io==0) v = x < 0.5 ? (1 - sqrt(1 - pow(2 * x, 2))) / 2 : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
    else if(e==6 && io>0) v = sqrt(1 - pow(x - 1, 2));

    // back
    else if(e==7 && io<0) v = c3 * x * x * x - c1 * x * x;
    else if(e==7 && io==0) v = x < 0.5 ? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2 : (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    else if(e==7 && io>0) v = 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);

    // elastic
    else if(e==8 && io<0) v = x == 0 ? 0 : x == 1 ? 1 : -pow(2, 10 * x - 10) * sinf((x * 10 - 10.75) * c4);
    else if(e==8 && io==0) v = x == 0 ? 0 : x == 1 ? 1 : x < 0.5 ? -(pow(2, 20 * x - 10) * sinf((20 * x - 11.125) * c5)) / 2 : (pow(2, -20 * x + 10) * sinf((20 * x - 11.125) * c5)) / 2 + 1;
    else if(e==8 && io>0) v = x == 0 ? 0 : x == 1 ? 1 : pow(2, -10 * x) * sinf((x * 10 - 0.75) * c4) + 1;

    // bounce
    else if(e==9 && io<0) v = easeInBounce(x);
    else if(e==9 && io==0) v = easeInOutBounce(x);
    else if(e==9 && io>0) v = easeOutBounce(x);

    struct TypedValue value;
    value.type = ValueTypeFloat;
    value.v.floatValue = v;

    return value;
}

enum ErrorCode cmd_RANDOMIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // RANDOMIZE
    ++interpreter->pc;

    struct TypedValue yValue;
    yValue.type = ValueTypeNull;

    pcg32_random_t *rng = &interpreter->defaultRng;

    // RANDOMIZE seed
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    // XXX: if (xValue.type != ValueTypeNull)

    // RANDOMIZE seed [,]
    if(interpreter->pc->type == TokenComma && !core->interpreter->compat) {
      ++interpreter->pc;

      // RANDOMIZE seed [, addr]
      yValue = itp_evaluateExpression(core, TypeClassNumeric);
      if (yValue.type == ValueTypeError) return yValue.v.errorCode;

      if (interpreter->pass == PassRun)
      {
          int addr = yValue.v.floatValue;
          rng = (pcg32_random_t *)(((uint8_t*)core->machine)+addr);
      }
    }

    if (interpreter->pass == PassRun)
    {
        if(interpreter->compat) interpreter->seed = xValue.v.floatValue;
        else {
          pcg32_srandom_r(rng, (uint32_t)xValue.v.floatValue, (intptr_t)rng);
        }
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_RND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // RND
    ++interpreter->pc;

    struct TypedValue xValue;
    xValue.type=ValueTypeNull;
    pcg32_random_t *rng = &interpreter->defaultRng;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // RND(
        ++interpreter->pc;

        // RND(bound
        xValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (xValue.type == ValueTypeError) return xValue;

        // RND(bound [,]
        if(interpreter->pc->type == TokenComma && !core->interpreter->compat) {
          ++interpreter->pc;

          // RND(bound [, addr]
          struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
          if (yValue.type == ValueTypeError) return yValue;

          if (interpreter->pass == PassRun) {
              int addr = yValue.v.floatValue;
              rng = (pcg32_random_t *)((uint8_t*)(core->machine)+addr);
          }
        }

        // RND(bound)
        // RND(bound [, addr])
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    }

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        if(core->interpreter->compat) {
            int seed = (1140671485 * interpreter->seed + 12820163) & 0xFFFFFF;
            interpreter->seed = seed;
            float rnd = seed / (float)0x1000000;

            if (xValue.type!=ValueTypeNull && xValue.v.floatValue >= 0)
            {
                // integer 0...x
                value.v.floatValue = floorf(rnd * (xValue.v.floatValue + 1));
            }
            else
            {
                // float 0..<1
                value.v.floatValue = rnd;
            }
        } else {

            if (xValue.type==ValueTypeNull) {
                value.v.floatValue = (float)ldexp(pcg32_random_r(rng), -32);
            } else {
                value.v.floatValue = (float)pcg32_boundedrand_r(rng, (uint32_t)xValue.v.floatValue + 1);
            }
        }
    }
    return value;
}

enum ErrorCode cmd_ADD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // ADD
    ++interpreter->pc;

    enum ErrorCode errorCode = ErrorNone;

    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // n vale
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    bool hasRange = false;
    float base = 0;
    float top = 0;

    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;

        // base value
        struct TypedValue baseValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (baseValue.type == ValueTypeError) return baseValue.v.errorCode;
        base = baseValue.v.floatValue;

        // TO
        if (interpreter->pc->type != TokenTO) return ErrorSyntax;
        ++interpreter->pc;

        // top value
        struct TypedValue topValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (topValue.type == ValueTypeError) return topValue.v.errorCode;
        top = topValue.v.floatValue;

        hasRange = true;
    }

    if (interpreter->pass == PassRun)
    {
        varValue->floatValue += nValue.v.floatValue;
        if (hasRange)
        {
            if (varValue->floatValue < base) varValue->floatValue = top;
            if (varValue->floatValue > top) varValue->floatValue = base;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INC_DEC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // INC/DEC
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    enum ErrorCode errorCode = ErrorNone;

    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenINC:
                ++varValue->floatValue;
                break;

            case TokenDEC:
                --varValue->floatValue;
                break;

            default:
                assert(0);
                break;
        }
    }

    return itp_endOfCommand(interpreter);
}
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

#include "core.h"
#include "core.h"
#include "core.h"
#include <assert.h>

struct TypedValue fnc_PEEK(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // PEEK/W/L
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // expression
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPEEK:
            {
                int peek = machine_peek(core, addressValue.v.floatValue);
                if (peek == -1) return val_makeError(ErrorIllegalMemoryAccess);
                resultValue.v.floatValue = peek;
                break;
            }

            case TokenPEEKW:
            {
                int peek1 = machine_peek(core, addressValue.v.floatValue);
                int peek2 = machine_peek(core, addressValue.v.floatValue + 1);
                if (peek1 == -1 || peek2 == -1) return val_makeError(ErrorIllegalMemoryAccess);

                int16_t value = peek1 | (peek2 << 8);
                resultValue.v.floatValue = value;
                break;
            }

            case TokenPEEKL:
            {
                int peek1 = machine_peek(core, addressValue.v.floatValue);
                int peek2 = machine_peek(core, addressValue.v.floatValue + 1);
                int peek3 = machine_peek(core, addressValue.v.floatValue + 2);
                int peek4 = machine_peek(core, addressValue.v.floatValue + 3);
                if (peek1 == -1 || peek2 == -1 || peek3 == -1 || peek4 == -1) return val_makeError(ErrorIllegalMemoryAccess);

                int32_t value = peek1 | (peek2 << 8) | (peek3 << 16) | (peek4 << 24);
                resultValue.v.floatValue = value;
                break;
            }

            default:
                assert(0);
        }
    }
    return resultValue;
}

enum ErrorCode cmd_POKE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // POKE/W/L
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // poke vale
    struct TypedValue pokeValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (pokeValue.type == ValueTypeError) return pokeValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPOKE:
            {
                bool poke = machine_poke(core, addressValue.v.floatValue, pokeValue.v.floatValue);
                if (!poke) return ErrorIllegalMemoryAccess;
                break;
            }

            case TokenPOKEW:
            {
                long value = (long)pokeValue.v.floatValue;
                bool poke1 = machine_poke(core, addressValue.v.floatValue    , value & 0xff);
                bool poke2 = machine_poke(core, addressValue.v.floatValue + 1, (value>>8) & 0xff);
                if (!poke1 || !poke2) return ErrorIllegalMemoryAccess;
                break;
            }

            case TokenPOKEL:
            {
                long value = (long)pokeValue.v.floatValue;
                bool poke1 = machine_poke(core, addressValue.v.floatValue    , value & 0xff);
                bool poke2 = machine_poke(core, addressValue.v.floatValue + 1, (value>>8) & 0xff);
                bool poke3 = machine_poke(core, addressValue.v.floatValue + 2, (value>>16) & 0xff);
                bool poke4 = machine_poke(core, addressValue.v.floatValue + 3, (value>>24) & 0xff);
                if (!poke1 || !poke2 || !poke3 || !poke4) return ErrorIllegalMemoryAccess;
                break;
            }

            default:
                assert(0);
        }

    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FILL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // FILL
    ++interpreter->pc;

    // start value
    struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueTypeError) return startValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // length value
    struct TypedValue lengthValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;

    int fill = 0;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;

        // fill value
        struct TypedValue fillValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (fillValue.type == ValueTypeError) return fillValue.v.errorCode;
        fill = fillValue.v.floatValue;
    }

    if (interpreter->pass == PassRun)
    {
        int start = startValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        for (int i = 0; i < length; i++)
        {
            bool poke = machine_poke(core, start + i, fill);
            if (!poke) return ErrorIllegalMemoryAccess;
        }
        interpreter->cycles += length;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_COPY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // COPY
    ++interpreter->pc;

    // source value
    struct TypedValue sourceValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (sourceValue.type == ValueTypeError) return sourceValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // length value
    struct TypedValue lengthValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;

    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;

    // destination value
    struct TypedValue destinationValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (destinationValue.type == ValueTypeError) return destinationValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int source = sourceValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        int destination = destinationValue.v.floatValue;
        if (source < destination)
        {
            for (int i = length - 1; i >= 0; i--)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        else if (source > destination)
        {
            for (int i = 0; i < length; i++)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        interpreter->cycles += length;
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_ROM_SIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // ROM/SIZE
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // index expression
    struct TypedValue indexValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (indexValue.type == ValueTypeError) return indexValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        int index = indexValue.v.floatValue;
        if (type == TokenSIZE)
        {
            value.v.floatValue = interpreter->romDataManager.entries[index].length;
        }
        else
        {
            value.v.floatValue = interpreter->romDataManager.entries[index].start + 0x10000;
        }
    }
    return value;
}

enum ErrorCode cmd_ROL_ROR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // ROL/ROR
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // n vale
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int value = machine_peek(core, addressValue.v.floatValue);
        if (value == -1) return ErrorIllegalMemoryAccess;

        int n = (int)nValue.v.floatValue;
        if (type == TokenROR)
        {
            n = -n;
        }
        n &= 0x07;

        value = value << n;
        value = value | (value >> 8);

        bool poke = machine_poke(core, addressValue.v.floatValue, value);
        if (!poke) return ErrorIllegalMemoryAccess;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DMA_COPY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    int base_addr = 0;

    // DMA
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // DMA COPY
    if (interpreter->pc->type != TokenCOPY) return ErrorSyntax;
    ++interpreter->pc;

		// DMA COPY ROM
    if (interpreter->pc->type == TokenROM)
    {
        ++interpreter->pc;
        base_addr=0x10000;
    }

    if (interpreter->pass == PassRun)
    {
        if (interpreter->mode != ModeInterrupt) return ErrorNotAllowedOutsideOfInterrupt;

        int source = core->machine->dmaRegisters.src_addr + base_addr;
        int destination = core->machine->dmaRegisters.dst_addr;
        int length = core->machine->dmaRegisters.bytes_count;
        if (source < destination)
        {
            for (int i = length - 1; i >= 0; i--)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        else if (source > destination)
        {
            for (int i = 0; i < length; i++)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        interpreter->cycles += (length+31)/32;
    }

    return itp_endOfCommand(interpreter);
}
#include "core.h"
#include "core.h"

#include "core.h"
#include "core.h"
#include "core.h"

enum ErrorCode cmd_PARTICLE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    struct ParticlesLib *lib=&interpreter->particlesLib;

    // PARTICLE
    ++interpreter->pc;

    // PARTICLE OFF
    if(interpreter->pc->type==TokenOFF)
    {
        ++interpreter->pc;

        prtclib_clear(core, lib);
        return itp_endOfCommand(interpreter);
    }

    // PARTICLE <NUM>
    // PARTICLE <APPAREANCE>
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);

    if(interpreter->pc->type==TokenComma)
    {
        // PARTICLE <NUM>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>NUM_SPRITES-1)) return ErrorInvalidParameter;

        ++interpreter->pc;

        // PARTICLE <NUM>,<COUNT>
        struct TypedValue cValue=itp_evaluateNumericExpression(core,0,NUM_SPRITES);
        if(cValue.type==ValueTypeError) return cValue.v.errorCode;

        // PARTICLE <NUM>,<COUNT> AT
        if(interpreter->pc->type!=TokenAT) return ErrorSyntax;
        ++interpreter->pc;

        // PARTICLE <NUM>,<COUNT> AT <ADDR>
        struct TypedValue aValue=itp_evaluateExpression(core,TypeClassNumeric);
        if(aValue.type==ValueTypeError) return aValue.v.errorCode;

        // TODO: check if last particle is out of RAM

        if (interpreter->pass == PassRun) prtclib_setupPool(lib,(int)nValue.v.floatValue,(int)cValue.v.floatValue,(int)aValue.v.floatValue);
    }
    else if(interpreter->pc->type==TokenDATA)
    {
        // PARTICLE <APPAREANCE>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>APPEARANCE_MAX-1)) return ErrorInvalidParameter;

        // PARTICLE <APPAREANCE> DATA
        ++interpreter->pc;

        // PARTICLE <APPAREANCE> DATA <LABEL>
        if(interpreter->pc->type!=TokenIdentifier) return ErrorExpectedLabel;
        struct Token *tk=interpreter->pc;
        ++interpreter->pc;

        if (interpreter->pass == PassPrepare)
        {
           struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);
           if (!item) return ErrorUndefinedLabel;
        }
        else if(interpreter->pass == PassRun)
        {
          struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);

          struct Token *dataToken = dat_reachData(interpreter, item->token);

          prtclib_setApperanceLabel(lib,(int)nValue.v.floatValue,dataToken);
        }
    }
    else return ErrorSyntax;

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_EMITTER(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    struct ParticlesLib *lib=&interpreter->particlesLib;

    // EMITTER
    ++interpreter->pc;

    // EMITTER <COUNT>
    // EMITTER <SPAWNER>
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);

    if(interpreter->pc->type==TokenAT)
    {
        // EMITTER <COUNT>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>EMITTER_MAX-1)) return ErrorInvalidParameter;

        // EMITTER <COUNT> AT
        if(interpreter->pc->type!=TokenAT) return ErrorSyntax;
        ++interpreter->pc;

        // EMITTER <COUNT> AT <ADDR>
        struct TypedValue aValue=itp_evaluateExpression(core,TypeClassNumeric);
        if(aValue.type==ValueTypeError) return aValue.v.errorCode;

        // TODO: check if last particle is out of RAM

        if(interpreter->pass == PassRun) prtclib_setupEmitter(lib,(int)nValue.v.floatValue,(int)aValue.v.floatValue);
    }
    else if(interpreter->pc->type==TokenDATA)
    {
        // EMITTER <SPAWNER>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>SPAWNER_MAX-1)) return ErrorInvalidParameter;

        // EMITTER <SPAWNER> DATA
        ++interpreter->pc;

        // EMITTER <SPAWNER> DATA <LABEL>
        if(interpreter->pc->type!=TokenIdentifier) return ErrorExpectedLabel;
        struct Token *tk=interpreter->pc;
        ++interpreter->pc;

        if (interpreter->pass == PassPrepare)
        {
           struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);
           if (!item) return ErrorUndefinedLabel;
        }
        else if(interpreter->pass == PassRun)
        {
          struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);

          struct Token *dataToken = dat_reachData(interpreter, item->token);

          prtclib_setSpawnerLabel(lib,(int)nValue.v.floatValue,dataToken);
        }
    }
    else if(interpreter->pc->type==TokenON)
    {
        // EMITTER <SPAWNER>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>SPAWNER_MAX-1)) return ErrorInvalidParameter;

        // EMITTER <SPAWNER> ON
        ++interpreter->pc;

        // EMITTER <SPAWNER> ON <X>
        struct TypedValue xValue=itp_evaluateExpression(core,TypeClassNumeric);
        if(xValue.type==ValueTypeError) return xValue.v.errorCode;

        // EMITTER <SPAWNER> ON <X>,
        if(interpreter->pc->type!=TokenComma) return ErrorSyntax;
        ++interpreter->pc;

        // EMITTER <SPAWNER> ON <X>,<Y>
        struct TypedValue yValue=itp_evaluateExpression(core,TypeClassNumeric);
        if(yValue.type==ValueTypeError) return yValue.v.errorCode;

        if(interpreter->pass == PassRun) prtclib_spawn(lib, (int)nValue.v.floatValue, xValue.v.floatValue, yValue.v.floatValue);
    }
    else if(interpreter->pc->type==TokenOFF)
    {
        // EMITTER <SPAWNER>
        if(core->interpreter->pass==PassPrepare && nValue.type!=ValueTypeFloat) return ErrorTypeMismatch;
        else if(core->interpreter->pass==PassRun && ((int)nValue.v.floatValue<0 || (int)nValue.v.floatValue>SPAWNER_MAX-1)) return ErrorInvalidParameter;

        // EMITTER <SPAWNER> OFF
        ++interpreter->pc;

        if(interpreter->pass == PassRun) prtclib_stop(lib, (int)nValue.v.floatValue);
    }
    else return ErrorSyntax;

    return itp_endOfCommand(interpreter);
}
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

#include "core.h"
#include "core.h"
#include <assert.h>
#include <stdint.h>
#include "core.h"

enum ErrorCode cmd_PALETTE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // PALETTE
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // c0 value
    struct TypedValue c0Value = itp_evaluateOptionalNumericExpression(core, 0, 63);
    if (c0Value.type == ValueTypeError) return c0Value.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // c1 value
    struct TypedValue c1Value = itp_evaluateOptionalNumericExpression(core, 0, 63);
    if (c1Value.type == ValueTypeError) return c1Value.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // c2 value
    struct TypedValue c2Value = itp_evaluateOptionalNumericExpression(core, 0, 63);
    if (c2Value.type == ValueTypeError) return c2Value.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // c3 value
    struct TypedValue c3Value = itp_evaluateOptionalNumericExpression(core, 0, 63);
    if (c3Value.type == ValueTypeError) return c3Value.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        uint8_t *palColors = &core->machine->colorRegisters.colors[n * 4];
        if (c0Value.type != ValueTypeNull) palColors[0] = c0Value.v.floatValue;
        if (c1Value.type != ValueTypeNull) palColors[1] = c1Value.v.floatValue;
        if (c2Value.type != ValueTypeNull) palColors[2] = c2Value.v.floatValue;
        if (c3Value.type != ValueTypeNull) palColors[3] = c3Value.v.floatValue;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SCROLL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SCROLL
    ++interpreter->pc;

    // bg value
    struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
    if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        struct VideoRegisters *reg = &core->machine->videoRegisters;
        int bg = bgValue.v.floatValue;
        int x = (int)xValue.v.floatValue;
        int y = (int)yValue.v.floatValue;
        if (bg == 0)
        {
            reg->scrollAX = x & 0x1FF;
            reg->scrollAY = y & 0x1FF;
            // reg->scrollMSB.aX = (x >> 8) & 1;
            // reg->scrollMSB.aY = (y >> 8) & 1;
        }
        else if(bg == 1)
        {
            reg->scrollBX = x & 0x1FF;
            reg->scrollBY = y & 0x1FF;
            // reg->scrollMSB.bX = (x >> 8) & 1;
            // reg->scrollMSB.bY = (y >> 8) & 1;
        }
        else if(bg == 2)
        {
            reg->scrollCX = x & 0x1FF;
            reg->scrollCY = y & 0x1FF;
            // reg->scrollMSB.cX = (x >> 8) & 1;
            // reg->scrollMSB.cY = (y >> 8) & 1;
        }
        else if(bg == 3)
        {
            reg->scrollDX = x & 0x1FF;
            reg->scrollDY = y & 0x1FF;
            // reg->scrollMSB.dX = (x >> 8) & 1;
            // reg->scrollMSB.dY = (y >> 8) & 1;
        }
    }

    return itp_endOfCommand(interpreter);
}

// enum ErrorCode cmd_DISPLAY(struct Core *core)
// {
//     struct Interpreter *interpreter = core->interpreter;

//     // DISPLAY
//     ++interpreter->pc;

//     // obsolete syntax!

//     // atrb value
//     struct TypedValue aValue = itp_evaluateDisplayAttributes(core, core->machine->videoRegisters.attr);
//     if (aValue.type == ValueTypeError) return aValue.v.errorCode;

//     if (interpreter->pass == PassRun)
//     {
//          core->machine->videoRegisters.attr.value = aValue.v.floatValue;
//     }

//     return itp_endOfCommand(interpreter);
// }

enum ErrorCode cmd_SPRITE_VIEW(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE VIEW
    ++interpreter->pc;
    ++interpreter->pc;

    // ON/OFF
    enum TokenType type = interpreter->pc->type;
    if (type != TokenON && type != TokenOFF) return ErrorSyntax;
    ++interpreter->pc;

    if (interpreter->pass == PassRun)
    {
        core->machine->videoRegisters.attr.spritesEnabled = type == TokenON ? 1 : 0;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_VIEW(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // BG VIEW
    ++interpreter->pc;
    ++interpreter->pc;

    // ON/OFF
    enum TokenType type = interpreter->pc->type;
    if (type != TokenON && type != TokenOFF) return ErrorSyntax;
    ++interpreter->pc;

    // bg value
    struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
    if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        int value = type == TokenON ? 1 : 0;
        if (bgValue.v.floatValue == 0)
        {
            core->machine->videoRegisters.attr.planeAEnabled = value;
        }
        else if (bgValue.v.floatValue == 1)
        {
            core->machine->videoRegisters.attr.planeBEnabled = value;
        }
        else if (bgValue.v.floatValue == 2)
        {
            core->machine->videoRegisters.attr.planeCEnabled = value;
        }
        else if (bgValue.v.floatValue == 3)
        {
            core->machine->videoRegisters.attr.planeDEnabled = value;
        }
    }

    return itp_endOfCommand(interpreter);
}

// enum ErrorCode cmd_CELL_SIZE(struct Core *core)
// {
//     struct Interpreter *interpreter = core->interpreter;

//     // CELL SIZE
//     ++interpreter->pc;
//     ++interpreter->pc;

//     // bg value
//     struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
//     if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;

//     // comma
//     if (interpreter->pc->type != TokenComma) return ErrorSyntax;
//     ++interpreter->pc;

//     // size value
//     struct TypedValue sValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
//     if (sValue.type == ValueTypeError) return sValue.v.errorCode;

//     if (interpreter->pass == PassRun)
//     {
//         if (bgValue.v.floatValue == 0)
//         {
//             core->machine->videoRegisters.attr.planeACellSize = sValue.v.floatValue;
//         }
//         else if(bgValue.v.floatValue == 1)
//         {
//             core->machine->videoRegisters.attr.planeBCellSize = sValue.v.floatValue;
//         }
//         else if(bgValue.v.floatValue == 2)
//         {
//             core->machine->videoRegisters.attr.planeCCellSize = sValue.v.floatValue;
//         }
//         else if(bgValue.v.floatValue == 3)
//         {
//             core->machine->videoRegisters.attr.planeDCellSize = sValue.v.floatValue;
//         }
//     }

//     return itp_endOfCommand(interpreter);
// }

struct TypedValue fnc_COLOR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // COLOR
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // pal expression
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
    if (pValue.type == ValueTypeError) return pValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // pal expression
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 3);
    if (nValue.type == ValueTypeError) return nValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        int p = pValue.v.floatValue;
        int n = nValue.v.floatValue;
        value.v.floatValue = core->machine->colorRegisters.colors[p * 4 + n];
    }
    return value;
}

struct TypedValue fnc_screen0(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenTIMER:
                value.v.floatValue = core->interpreter->timer;
                break;

            case TokenRASTER:
                value.v.floatValue = core->machine->videoRegisters.rasterLine;
                break;

            // case TokenDISPLAY:
            //     // obsolete syntax!
            //     value.v.floatValue = core->machine->videoRegisters.attr.value;
            //     break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_SCROLL_X_Y(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SCROLL.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // bg value
    struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
    if (bgValue.type == ValueTypeError) return bgValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        int bg = bgValue.v.floatValue;
        struct VideoRegisters *reg = &core->machine->videoRegisters;
        switch (type)
        {
            case TokenSCROLLX:
                if (bg == 0)
                {
                    value.v.floatValue = reg->scrollAX; // | (reg->scrollMSB.aX << 8);
                }
                else if (bg == 1)
                {
                    value.v.floatValue = reg->scrollBX; // | (reg->scrollMSB.bX << 8);
                }
                else if (bg == 2)
                {
                    value.v.floatValue = reg->scrollCX; // | (reg->scrollMSB.cX << 8);
                }
                else if (bg == 3)
                {
                    value.v.floatValue = reg->scrollDX; // | (reg->scrollMSB.dX << 8);
                }
                break;

            case TokenSCROLLY:
                if (bg == 0)
                {
                    value.v.floatValue = reg->scrollAY; // | (reg->scrollMSB.aY << 8);
                }
                else if (bg == 1)
                {
                    value.v.floatValue = reg->scrollBY; // | (reg->scrollMSB.bY << 8);
                }
                else if (bg == 2)
                {
                    value.v.floatValue = reg->scrollCY; // | (reg->scrollMSB.cY << 8);
                }
                else if (bg == 3)
                {
                    value.v.floatValue = reg->scrollDY; // | (reg->scrollMSB.cY << 8);
                }
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}
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

#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_SPRITE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    // comma
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;

        // x value
        struct TypedValue xValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (xValue.type == ValueTypeError) return xValue.v.errorCode;

        // comma
        if (interpreter->pc->type != TokenComma) return ErrorSyntax;
        ++interpreter->pc;

        // y value
        struct TypedValue yValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (yValue.type == ValueTypeError) return yValue.v.errorCode;

        // comma
        if (interpreter->pc->type != TokenComma) return ErrorSyntax;
        ++interpreter->pc;

        // c value
        struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
        if (cValue.type == ValueTypeError) return cValue.v.errorCode;

        if (interpreter->pass == PassRun)
        {
            // User entry       Saved value
            // ----------------------------
            // 0                0x0
            // 1                0x10        0b_0001_0000
            // 0.9375   *x16    0xf         0b_0000_1111
            // 0.5                          0b_0000_1000
            // 0.25
            // 0.125
            // 0.0625                       0b_0000_0001

            int n = nValue.v.floatValue;
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];
            if (xValue.type != ValueTypeNull) sprite->x = (int)((xValue.v.floatValue + SPRITE_OFFSET_X)*16) & 0x1FFF;
            if (yValue.type != ValueTypeNull) sprite->y = (int)((yValue.v.floatValue + SPRITE_OFFSET_Y)*16) & 0x1FFF;
            if (cValue.type != ValueTypeNull) sprite->character = cValue.v.floatValue;
        }
    }
    else
    {
        struct SimpleAttributes attrs;
        enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
        if (attrsError != ErrorNone) return attrsError;

        if (interpreter->pass == PassRun)
        {
            int n = nValue.v.floatValue;
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];

            if (attrs.pal >= 0) sprite->attr.palette = attrs.pal;
            if (attrs.flipX >= 0) sprite->attr.flipX = attrs.flipX;
            if (attrs.flipY >= 0) sprite->attr.flipY = attrs.flipY;
            if (attrs.prio >= 0) sprite->attr.priority = attrs.prio;
            if (attrs.size >= 0) sprite->attr.size = attrs.size;
        }
    }

    return itp_endOfCommand(interpreter);
}

// TODO: disable
enum ErrorCode cmd_SPRITE_A(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE.A
    ++interpreter->pc;

    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    struct Sprite *sprite = NULL;
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        sprite = &core->machine->spriteRegisters.sprites[n];
    }

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    union CharacterAttributes attr;
    if (sprite)
    {
        attr = sprite->attr;
    }
    else
    {
        attr.value = 0;
    }

    // attr value
    struct TypedValue aValue = itp_evaluateCharAttributes(core, attr);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        sprite->attr.value = aValue.v.floatValue;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SPRITE_OFF(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE
    ++interpreter->pc;

    // OFF
    if (interpreter->pc->type != TokenOFF) return ErrorSyntax;
    ++interpreter->pc;

    int from = 0;
    int to = NUM_SPRITES - 1;

    if (!itp_isEndOfCommand(interpreter))
    {
        // from value
        struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
        if (nValue.type == ValueTypeError) return nValue.v.errorCode;
        from = nValue.v.floatValue;
        to = from;

        // TO
        if (interpreter->pc->type == TokenTO)
        {
            ++interpreter->pc;

            // to value
            struct TypedValue mValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
            if (mValue.type == ValueTypeError) return mValue.v.errorCode;
            to = mValue.v.floatValue;
        }
    }

    if (interpreter->pass == PassRun)
    {
        for (int i = from; i <= to; i++)
        {
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[i];
            sprite->x = 0;
            sprite->y = 0;
        }
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_SPRITE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // expression
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];
        switch (type)
        {
            case TokenSPRITEX:
                value.v.floatValue = (float)sprite->x/16 - SPRITE_OFFSET_X;
                break;

            case TokenSPRITEY:
                value.v.floatValue = (float)sprite->y/16 - SPRITE_OFFSET_Y;
                break;

            case TokenSPRITEC:
                value.v.floatValue = sprite->character;
                break;

            case TokenSPRITEA:
                value.v.floatValue = sprite->attr.value;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_SPRITE_HIT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SPRITE
    ++interpreter->pc;

    // HIT
    if (interpreter->pc->type != TokenHIT) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // sprite number
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue;

    int first = 0;
    int last = NUM_SPRITES - 1;

    // other sprite number
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
        struct TypedValue otherValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
        if (otherValue.type == ValueTypeError) return otherValue;
        first = otherValue.v.floatValue;
        last = first;

        // last sprite number
        if (interpreter->pc->type == TokenTO)
        {
            ++interpreter->pc;
            struct TypedValue lastValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
            if (lastValue.type == ValueTypeError) return lastValue;
            last = lastValue.v.floatValue;
        }
    }

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        bool hits = sprlib_checkCollision(&interpreter->spritesLib, nValue.v.floatValue, first, last);
        value.v.floatValue = hits ? BAS_TRUE : BAS_FALSE;
    }

    return value;
}

struct TypedValue fnc_HIT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // HIT
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = interpreter->spritesLib.lastHit;
    }

    return value;
}
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

#include "core.h"
#include "core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "core.h"

struct TypedValue fnc_ASC(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ASC
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		char ch = stringValue.v.stringValue->chars[0];
		rcstring_release(stringValue.v.stringValue);

		if (ch == 0)
			return val_makeError(ErrorInvalidParameter);
		value.v.floatValue = ch;
	}
	return value;
}

struct TypedValue fnc_BIN_HEX(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// BIN$/HEX$
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// x expression
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue;

	int maxLen = (type == TokenHEX) ? 8 : 16;
	int width = 0;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// width expression
		struct TypedValue widthValue = itp_evaluateNumericExpression(core, 0, maxLen);
		if (widthValue.type == ValueTypeError)
			return widthValue;
		width = widthValue.v.floatValue;
	}

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		int x = xValue.v.floatValue;

		struct RCString *rcstring = rcstring_new(NULL, maxLen);
		if (!rcstring)
			return val_makeError(ErrorOutOfMemory);

		if (type == TokenBIN)
		{
			txtlib_itobin(rcstring->chars, maxLen + 1, width, x);
		}
		else if (type == TokenHEX)
		{
			snprintf(rcstring->chars, maxLen + 1, "%0*X", width, x);
		}
		resultValue.v.stringValue = rcstring;
		interpreter->cycles += maxLen;
	}
	return resultValue;
}

struct TypedValue fnc_CHR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// CHR$
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// expression
	struct TypedValue numericValue = itp_evaluateNumericExpression(core, 0, 255);
	if (numericValue.type == ValueTypeError)
		return numericValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		char ch = numericValue.v.floatValue;
		struct RCString *rcstring = rcstring_new(&ch, 1);
		if (!rcstring)
			return val_makeError(ErrorOutOfMemory);

		resultValue.v.stringValue = rcstring;
		interpreter->cycles += 1;
	}
	return resultValue;
}

struct TypedValue fnc_INKEY(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// INKEY$
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		char key = core->machine->ioRegisters.key;
		if (key)
		{
			core->machine->ioRegisters.key = 0;

			struct RCString *rcstring = rcstring_new(&key, 1);
			if (!rcstring)
				return val_makeError(ErrorOutOfMemory);

			resultValue.v.stringValue = rcstring;
			interpreter->cycles += 1;
		}
		else
		{
			resultValue.v.stringValue = interpreter->nullString;
			rcstring_retain(resultValue.v.stringValue);
		}
	}
	return resultValue;
}

struct TypedValue fnc_INSTR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// INSTR
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// string expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// search value
	struct TypedValue searchValue = itp_evaluateExpression(core, TypeClassString);
	if (searchValue.type == ValueTypeError)
		return searchValue;

	int startIndex = 0;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// number value
		struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (posValue.type == ValueTypeError)
			return posValue;

		startIndex = posValue.v.floatValue - 1;
	}

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		char *string = stringValue.v.stringValue->chars;
		char *search = searchValue.v.stringValue->chars;
		size_t stringlen = strlen(string);
		if (startIndex >= stringlen || search[0] == 0)
		{
			resultValue.v.floatValue = 0;
		}
		else
		{
			char *found = strstr(&string[startIndex], search);
			if (found)
			{
				resultValue.v.floatValue = (found - string) + 1;
			}
			else
			{
				resultValue.v.floatValue = 0;
			}
		}
		rcstring_release(stringValue.v.stringValue);
		rcstring_release(searchValue.v.stringValue);
	}
	return resultValue;
}

struct TypedValue fnc_LEFTStr_RIGHTStr(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// LEFT$/RIGHT$
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (numberValue.type == ValueTypeError)
		return numberValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		if (numberValue.v.floatValue < 0)
			return val_makeError(ErrorInvalidParameter);

		size_t len = strlen(stringValue.v.stringValue->chars);
		size_t number = numberValue.v.floatValue;

		if (number < len)
		{
			size_t start = (type == TokenLEFTStr) ? 0 : len - number;

			struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[start], number);
			if (!rcstring)
				return val_makeError(ErrorOutOfMemory);

			resultValue.v.stringValue = rcstring;
			interpreter->cycles += number;
		}
		else
		{
			resultValue.v.stringValue = stringValue.v.stringValue;
			rcstring_retain(resultValue.v.stringValue);
		}
		rcstring_release(stringValue.v.stringValue);
	}
	return resultValue;
}

struct TypedValue fnc_LEN(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// LEN
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	// expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassAny);

	// LEN(x
	if (stringValue.type == ValueTypeFloat)
	{
		struct TypedValue xValue = stringValue;

		// LEN(x,
		if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
		++interpreter->pc;

		// LEN(x,y
		struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (yValue.type == ValueTypeError) return yValue;

		// LEN(x,y)
		if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
		++interpreter->pc;

		if (interpreter->pass == PassRun)
		{
			value.v.floatValue = sqrtf(xValue.v.floatValue * xValue.v.floatValue + yValue.v.floatValue * yValue.v.floatValue);
		}
	}

	// LEN($STR)
	// LEN("HELLO")
	else if(stringValue.type == ValueTypeString)
	{
		// bracket close
		if (interpreter->pc->type != TokenBracketClose)
			return val_makeError(ErrorSyntax);
		++interpreter->pc;

		if (interpreter->pass == PassRun)
		{
			value.v.floatValue = strlen(stringValue.v.stringValue->chars);
			rcstring_release(stringValue.v.stringValue);
		}
	}
	else
	{
		value.type = ValueTypeError;
	}

	return value;
}

struct TypedValue fnc_MID(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// MID$
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// string expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// position value
	struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (posValue.type == ValueTypeError)
		return posValue;

	// comma
	if (interpreter->pc->type != TokenComma)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// number value
	struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (numberValue.type == ValueTypeError)
		return numberValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		if (numberValue.v.floatValue < 0)
			return val_makeError(ErrorInvalidParameter);
		if (posValue.v.floatValue < 1)
			return val_makeError(ErrorInvalidParameter);

		size_t len = strlen(stringValue.v.stringValue->chars);
		size_t index = posValue.v.floatValue - 1;
		size_t number = numberValue.v.floatValue;

		if (index >= len)
		{
			resultValue.v.stringValue = interpreter->nullString;
			rcstring_retain(resultValue.v.stringValue);
		}
		else if (index > 0 || number < len)
		{
			if (index + number > len)
			{
				number = len - index;
			}
			struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[index], number);
			if (!rcstring)
				return val_makeError(ErrorOutOfMemory);

			resultValue.v.stringValue = rcstring;
			interpreter->cycles += number;
		}
		else
		{
			resultValue.v.stringValue = stringValue.v.stringValue;
			rcstring_retain(resultValue.v.stringValue);
		}

		rcstring_release(stringValue.v.stringValue);
	}
	return resultValue;
}

struct TypedValue fnc_STR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// STR$
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// expression
	struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (numericValue.type == ValueTypeError)
		return numericValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue resultValue;
	resultValue.type = ValueTypeString;

	if (interpreter->pass == PassRun)
	{
		struct RCString *rcstring = rcstring_new(NULL, 20);
		if (!rcstring)
			return val_makeError(ErrorOutOfMemory);

		snprintf(rcstring->chars, 20, "%0.7g", numericValue.v.floatValue);
		resultValue.v.stringValue = rcstring;
		interpreter->cycles += strlen(rcstring->chars);
	}
	return resultValue;
}

struct TypedValue fnc_VAL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// VAL
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	// expression
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue;

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return val_makeError(ErrorSyntax);
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		value.v.floatValue = atof(stringValue.v.stringValue->chars);
		rcstring_release(stringValue.v.stringValue);
	}
	return value;
}

enum ErrorCode cmd_LEFT_RIGHT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// LEFT$/RIGHT$
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return ErrorSyntax;
	++interpreter->pc;

	// variable
	enum ErrorCode errorCode = ErrorNone;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeString)
		return ErrorTypeMismatch;

	size_t number = SIZE_MAX;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// number expression
		struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (numberValue.type == ValueTypeError)
			return numberValue.v.errorCode;
		number = numberValue.v.floatValue;
	}

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return ErrorSyntax;
	++interpreter->pc;

	// equal sign
	if (interpreter->pc->type != TokenEq)
		return ErrorSyntax;
	++interpreter->pc;

	// replace expression
	struct TypedValue replaceValue = itp_evaluateExpression(core, TypeClassString);
	if (replaceValue.type == ValueTypeError)
		return replaceValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		size_t resultLen = strlen(varValue->stringValue->chars);

		struct RCString *resultRCString = varValue->stringValue;
		if (resultRCString->refCount > 1)
		{
			// copy string if shared
			resultRCString = rcstring_new(varValue->stringValue->chars, resultLen);
			rcstring_release(varValue->stringValue);
			varValue->stringValue = resultRCString;
		}

		char *resultString = resultRCString->chars;
		char *replaceString = replaceValue.v.stringValue->chars;
		size_t replaceLen = strlen(replaceString);
		if (number > replaceLen)
		{
			number = replaceLen;
		}
		if (number > resultLen)
		{
			number = resultLen;
		}

		if (type == TokenLEFTStr)
		{
			for (size_t i = 0; i < number; i++)
			{
				resultString[i] = replaceString[i];
			}
		}
		else if (type == TokenRIGHTStr)
		{
			for (size_t i = 0; i < number; i++)
			{
				resultString[resultLen - 1 - i] = replaceString[replaceLen - 1 - i];
			}
		}
		interpreter->cycles += number;

		rcstring_release(replaceValue.v.stringValue);
	}

	return itp_endOfCommand(interpreter);
}

// TODO: move to cmd_strings.c
enum ErrorCode cmd_MID(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// MID$
	++interpreter->pc;

	// bracket open
	if (interpreter->pc->type != TokenBracketOpen)
		return ErrorSyntax;
	++interpreter->pc;

	// variable
	enum ErrorCode errorCode = ErrorNone;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeString)
		return ErrorTypeMismatch;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// position expression
	struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (posValue.type == ValueTypeError)
		return posValue.v.errorCode;

	size_t number = SIZE_MAX;
	if (interpreter->pc->type == TokenComma)
	{
		// comma
		++interpreter->pc;

		// number expression
		struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (numberValue.type == ValueTypeError)
			return numberValue.v.errorCode;
		number = numberValue.v.floatValue;
	}

	// bracket close
	if (interpreter->pc->type != TokenBracketClose)
		return ErrorSyntax;
	++interpreter->pc;

	// equal sign
	if (interpreter->pc->type != TokenEq)
		return ErrorSyntax;
	++interpreter->pc;

	// replace expression
	struct TypedValue replaceValue = itp_evaluateExpression(core, TypeClassString);
	if (replaceValue.type == ValueTypeError)
		return replaceValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		size_t index = posValue.v.floatValue - 1;
		size_t resultLen = strlen(varValue->stringValue->chars);

		struct RCString *resultRCString = varValue->stringValue;
		if (resultRCString->refCount > 1)
		{
			// copy string if shared
			resultRCString = rcstring_new(varValue->stringValue->chars, resultLen);
			rcstring_release(varValue->stringValue);
			varValue->stringValue = resultRCString;
		}

		if (index < resultLen)
		{
			char *resultString = resultRCString->chars;
			char *replaceString = replaceValue.v.stringValue->chars;
			size_t replaceLen = strlen(replaceString);
			if (number > replaceLen)
			{
				number = replaceLen;
			}
			if (index + number > resultLen)
			{
				number = resultLen - index;
			}

			for (size_t i = 0; i < number; i++)
			{
				resultString[index + i] = replaceString[i];
			}
			interpreter->cycles += number;
		}
		rcstring_release(replaceValue.v.stringValue);
	}

	return itp_endOfCommand(interpreter);
}
//
// Copyright 2018-2020 Timo Kloss
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

#include "core.h"
#include "core.h"

enum ErrorCode cmd_CALL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // CALL
    struct Token *tokenCALL = interpreter->pc;
    ++interpreter->pc;

    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedSubprogramName;
    struct Token *tokenSubIdentifier = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct SubItem *item = tok_getSub(&interpreter->tokenizer, tokenSubIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedSubprogram;
        tokenCALL->jumpToken = item->token;
    }

    // optional arguments
    int numArguments = 0;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        do
        {
            // bracket or comma
            ++interpreter->pc;

            // argument
            struct Token *tokens = interpreter->pc;
            if ((interpreter->pc->type == TokenIdentifier || interpreter->pc->type == TokenStringIdentifier)
                && tokens[1].type == TokenBracketOpen
                && tokens[2].type == TokenBracketClose)
            {
                // pass array by reference
                if (interpreter->pass == PassRun)
                {
                    struct ArrayVariable *variable = var_getArrayVariable(interpreter, interpreter->pc->symbolIndex, interpreter->subLevel);
                    if (!variable) return ErrorArrayNotDimensionized;

                    enum ErrorCode errorCode = ErrorNone;
                    var_createArrayVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, variable);
                    if (errorCode != ErrorNone) return errorCode;
                }
                interpreter->pc += 3;
            }
            else
            {
                // expression
                struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
                if (value.type == ValueTypeError) return value.v.errorCode;

                if (interpreter->pass == PassRun)
                {
                    enum ErrorCode errorCode = ErrorNone;
                    if (interpreter->lastVariableValue)
                    {
                        // pass by reference (simple variable or array element)
                        enum ErrorCode errorCode = ErrorNone;
                        var_createSimpleVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, value.type, interpreter->lastVariableValue);
                        if (errorCode != ErrorNone) return errorCode;
                    }
                    else
                    {
                        // pass by value
                        struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, value.type, NULL);
                        if (!variable) return errorCode;

                        variable->v = value.v;
                    }
                }
            }
            ++numArguments;
        }
        while (interpreter->pc->type == TokenComma);

        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;
    }

    if (interpreter->pass == PassRun)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeCALL, interpreter->pc);
        if (errorCode != ErrorNone) return errorCode;

        interpreter->pc = tokenCALL->jumpToken; // after sub name
        interpreter->subLevel++;

        // parameters
        if (interpreter->pc->type == TokenBracketOpen)
        {
            int parameterIndex = 0;
            do
            {
                if (parameterIndex >= numArguments) return ErrorArgumentCountMismatch;

                // bracket or comma
                ++interpreter->pc;

                // parameter
                struct Token *tokenIdentifier = interpreter->pc;
                if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
                enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);

                struct Token *nextToken = interpreter->pc + 1;
                if (nextToken->type == TokenBracketOpen)
                {
                    // array
                    struct ArrayVariable *variable = var_getArrayVariable(interpreter, parameterIndex + 1, interpreter->subLevel);
                    if (!variable || variable->type != varType) return ErrorTypeMismatch;

                    variable->symbolIndex = tokenIdentifier->symbolIndex;

                    interpreter->pc += 2;

                    if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
                    ++interpreter->pc;
                }
                else
                {
                    // simple variable
                    struct SimpleVariable *variable = var_getSimpleVariable(interpreter, parameterIndex + 1, interpreter->subLevel);
                    if (!variable || variable->type != varType) return ErrorTypeMismatch;

                    variable->symbolIndex = tokenIdentifier->symbolIndex;

                    ++interpreter->pc;
                }

                ++parameterIndex;
            }
            while (interpreter->pc->type == TokenComma);

            if (parameterIndex < numArguments) return ErrorArgumentCountMismatch;

            if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
            ++interpreter->pc;
        }
        else if (numArguments > 0)
        {
            return ErrorArgumentCountMismatch;
        }

        return ErrorNone;
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SUB
    struct Token *tokenSUB = interpreter->pc;
    ++interpreter->pc;

    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedSubprogramName;
    ++interpreter->pc;

    // parameters
    if (interpreter->pc->type == TokenBracketOpen)
    {
        do
        {
            // bracket or comma
            ++interpreter->pc;

            // parameter
            struct Token *tokenIdentifier = interpreter->pc;
            if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
            ++interpreter->pc;

            if (interpreter->pc->type == TokenBracketOpen)
            {
                ++interpreter->pc;
                if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
                ++interpreter->pc;
            }
        }
        while (interpreter->pc->type == TokenComma);

        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;
    }

    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->numLabelStackItems > 0)
        {
            return ErrorSubCannotBeNested;
        }
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeSUB, tokenSUB);
        if (errorCode != ErrorNone) return errorCode;

        interpreter->subLevel++;

        // Eol
        if (interpreter->pc->type != TokenEol) return ErrorSyntax;
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenSUB->jumpToken; // after END SUB
    }

    return ErrorNone;
}

enum ErrorCode cmd_END_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // END SUB
    ++interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item)
        {
            return ErrorEndSubWithoutSub;
        }
        else if (item->type == LabelTypeSUB)
        {
            item->token->jumpToken = interpreter->pc;
        }
        else
        {
            enum ErrorCode errorCode = itp_labelStackError(item);
            return errorCode != ErrorNone ? errorCode : ErrorEndSubWithoutSub;
        }

        // Eol
        if (interpreter->pc->type != TokenEol) return ErrorSyntax;
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemCALL = lab_popLabelStackItem(interpreter);
        if (!itemCALL) return ErrorEndSubWithoutSub;

        // clean local variables
        var_freeSimpleVariables(interpreter, interpreter->subLevel);
        var_freeArrayVariables(interpreter, interpreter->subLevel);

        if (itemCALL->type == LabelTypeONCALL)
        {
            // exit from interrupt
            interpreter->exitEvaluation = true;
        }
        else if (itemCALL->type == LabelTypeCALL)
        {
            // jump back
            interpreter->pc = itemCALL->token; // after CALL
        }
        else
        {
            return ErrorEndSubWithoutSub;
        }
    }
    interpreter->subLevel--;

    return ErrorNone;
}

/*
enum ErrorCode cmd_SHARED(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassPrepare && interpreter->subLevel == 0) return ErrorSharedOutsideOfASubprogram;

    do
    {
        // SHARED or comma
        ++interpreter->pc;

        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorExpectedVariableIdentifier;
        ++interpreter->pc;

        enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
        int symbolIndex = tokenIdentifier->symbolIndex;

        if (interpreter->pc->type == TokenBracketOpen)
        {
            // array
            ++interpreter->pc;

            if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
            ++interpreter->pc;

            if (interpreter->pass == PassRun)
            {
                struct ArrayVariable *globalVariable = var_getArrayVariable(interpreter, symbolIndex, 0);
                if (!globalVariable) return ErrorArrayNotDimensionized;

                enum ErrorCode errorCode = ErrorNone;
                var_createArrayVariable(interpreter, &errorCode, symbolIndex, interpreter->subLevel, globalVariable);
                if (errorCode != ErrorNone) return errorCode;
            }
        }
        else
        {
            // simple variable
            if (interpreter->pass == PassRun)
            {
                struct SimpleVariable *globalVariable = var_getSimpleVariable(interpreter, symbolIndex, 0);
                if (!globalVariable) return ErrorVariableNotInitialized;

                enum ErrorCode errorCode = ErrorNone;
                var_createSimpleVariable(interpreter, &errorCode, symbolIndex, interpreter->subLevel, varType, &globalVariable->v);
                if (errorCode != ErrorNone) return errorCode;
            }
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}
*/

enum ErrorCode cmd_GLOBAL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassPrepare && interpreter->subLevel > 0) return ErrorGlobalInsideOfASubprogram;

    do
    {
        // GLOBAL or comma
        ++interpreter->pc;

        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
        ++interpreter->pc;

        int symbolIndex = tokenIdentifier->symbolIndex;

        if (interpreter->pass == PassRun)
        {
            struct SimpleVariable *variable = var_getSimpleVariable(interpreter, symbolIndex, 0);
            if (variable)
            {
                variable->subLevel = SUB_LEVEL_GLOBAL;
            }
            else
            {
                enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
                enum ErrorCode errorCode = ErrorNone;
                variable = var_createSimpleVariable(interpreter, &errorCode, symbolIndex, SUB_LEVEL_GLOBAL, varType, NULL);
                if (!variable) return errorCode;
            }
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_EXIT_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // EXIT
    ++interpreter->pc;

    // SUB
    if (interpreter->pc->type != TokenSUB) return ErrorSyntax;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->subLevel == 0) return ErrorExitSubOutsideOfASubprogram;
        return itp_endOfCommand(interpreter);
    }
    else if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemCALL = lab_popLabelStackItem(interpreter);
        if (!itemCALL) return ErrorExitSubOutsideOfASubprogram;

        // clean local variables
        var_freeSimpleVariables(interpreter, interpreter->subLevel);
        var_freeArrayVariables(interpreter, interpreter->subLevel);

        if (itemCALL->type == LabelTypeONCALL)
        {
            // exit from interrupt
            interpreter->exitEvaluation = true;
        }
        else if (itemCALL->type == LabelTypeCALL)
        {
            // jump back
            interpreter->pc = itemCALL->token; // after CALL
        }
        else
        {
            return ErrorExitSubOutsideOfASubprogram;
        }
        interpreter->subLevel--;
    }
    return ErrorNone;
}
//
// Copyright 2017-2020 Timo Kloss
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

#include "core.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "core.h"
#include "core.h"

enum ErrorCode cmd_PRINT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	struct TextLib *lib = &interpreter->textLib;

	bool newLine = true;

	// PRINT
	++interpreter->pc;

	while (!itp_isEndOfCommand(interpreter))
	{
		struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
		if (value.type == ValueTypeError)
			return value.v.errorCode;

		if (interpreter->pass == PassRun)
		{
			if (value.type == ValueTypeString)
			{
				txtlib_printText(lib, value.v.stringValue->chars);
			}
			else if (value.type == ValueTypeFloat)
			{
				char buffer[20];
				snprintf(buffer, 20, "%0.7g", value.v.floatValue);
				txtlib_printText(lib, buffer);
			}
		}

		if (interpreter->pc->type == TokenComma)
		{
			if (interpreter->pass == PassRun)
			{
				txtlib_printText(lib, " ");
			}
			++interpreter->pc;
			newLine = false;
		}
		else if (interpreter->pc->type == TokenSemicolon)
		{
			++interpreter->pc;
			newLine = false;
		}
		else if (itp_isEndOfCommand(interpreter))
		{
			newLine = true;
		}
		else
		{
			return ErrorSyntax;
		}
	}

	if (interpreter->pass == PassRun && newLine)
	{
		txtlib_printText(lib, "\n");
	}
	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INPUT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	struct TextLib *lib = &interpreter->textLib;

	// INPUT
	++interpreter->pc;

	if (interpreter->pc->type == TokenString)
	{
		// prompt
		if (interpreter->pass == PassRun)
		{
			txtlib_printText(lib, interpreter->pc->stringValue->chars);
		}
		++interpreter->pc;

		// semicolon
		if (interpreter->pc->type != TokenSemicolon)
			return ErrorSyntax;
		++interpreter->pc;
	}

	if (interpreter->pass == PassRun)
	{
		txtlib_inputBegin(lib);
		interpreter->state = StateInput;
	}
	else
	{
		return cmd_endINPUT(core);
	}

	return ErrorNone;
}

enum ErrorCode cmd_endINPUT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// identifier
	enum ErrorCode errorCode = ErrorNone;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;

	if (interpreter->pass == PassRun)
	{
		if (valueType == ValueTypeString)
		{
			struct RCString *rcstring = rcstring_new(interpreter->textLib.inputBuffer, interpreter->textLib.inputLength);
			if (!rcstring)
				return ErrorOutOfMemory;

			if (varValue->stringValue)
			{
				rcstring_release(varValue->stringValue);
			}
			varValue->stringValue = rcstring;
		}
		else if (valueType == ValueTypeFloat)
		{
			varValue->floatValue = atof(interpreter->textLib.inputBuffer);
		}
	}
	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TEXT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// TEXT
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// string value
	struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
	if (stringValue.type == ValueTypeError)
		return stringValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		struct TextLib *lib = &interpreter->textLib;
		txtlib_writeText(lib, stringValue.v.stringValue->chars, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_NUMBER(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// NUMBER
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// number value
	struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (numberValue.type == ValueTypeError)
		return numberValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// digits value
	struct TypedValue digitsValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (digitsValue.type == ValueTypeError)
		return digitsValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		int digits = digitsValue.v.floatValue;
		struct TextLib *lib = &interpreter->textLib;
		txtlib_writeNumber(lib, numberValue.v.floatValue, digits, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CLS(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	struct TextLib *lib = &interpreter->textLib;

	// CLS
	++interpreter->pc;

	if (itp_isEndOfCommand(interpreter))
	{
		if (interpreter->pass == PassRun)
		{
			// clear all
			txtlib_clearScreen(lib);
		}
	}
	else
	{
		// bg value
		struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
		if (bgValue.type == ValueTypeError)
			return bgValue.v.errorCode;

		if (interpreter->pass == PassRun)
		{
			// clear bg
			txtlib_clearBackground(lib, bgValue.v.floatValue);
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_WINDOW(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// WINDOW
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// w value
	struct TypedValue wValue = itp_evaluateNumericExpression(core, 1, PLANE_COLUMNS);
	if (wValue.type == ValueTypeError)
		return wValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// h value
	struct TypedValue hValue = itp_evaluateNumericExpression(core, 1, PLANE_ROWS);
	if (hValue.type == ValueTypeError)
		return hValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// bg value
	struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 3);
	if (bgValue.type == ValueTypeError)
		return bgValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		core->interpreter->textLib.windowX = xValue.v.floatValue;
		core->interpreter->textLib.windowY = yValue.v.floatValue;
		core->interpreter->textLib.windowWidth = wValue.v.floatValue;
		core->interpreter->textLib.windowHeight = hValue.v.floatValue;
		core->interpreter->textLib.bg = bgValue.v.floatValue;
		core->interpreter->textLib.cursorX = 0;
		core->interpreter->textLib.cursorY = 0;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FONT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// FONT
	++interpreter->pc;

	// char value
	struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_CHARACTERS - 1);
	if (cValue.type == ValueTypeError)
		return cValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		interpreter->textLib.fontCharOffset = cValue.v.floatValue;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LOCATE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// LOCATE
	++interpreter->pc;

	// x value
	struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, 255);
	if (xValue.type == ValueTypeError)
		return xValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// y value
	struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, 255);
	if (yValue.type == ValueTypeError)
		return yValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		core->interpreter->textLib.cursorX = xValue.v.floatValue;
		core->interpreter->textLib.cursorY = yValue.v.floatValue;
	}

	return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_CURSOR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// CURSOR.?
	enum TokenType type = interpreter->pc->type;
	++interpreter->pc;

	struct TypedValue value;
	value.type = ValueTypeFloat;

	if (interpreter->pass == PassRun)
	{
		switch (type)
		{
		case TokenCURSORX:
			value.v.floatValue = interpreter->textLib.cursorX;
			break;

		case TokenCURSORY:
			value.v.floatValue = interpreter->textLib.cursorY;
			break;

		default:
			assert(0);
			break;
		}
	}
	return value;
}

enum ErrorCode cmd_CLW(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// CLW
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		txtlib_clearWindow(&interpreter->textLib);
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TRACE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct TextLib *lib = &core->overlay->textLib;
	bool debug = interpreter->debug;

	do
	{
		// TRACE or comma
		bool separate = (interpreter->pc->type == TokenComma);
		++interpreter->pc;

		struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
		if (value.type == ValueTypeError)
			return value.v.errorCode;

		if (interpreter->pass == PassRun)
		{
			if (separate && debug)
			{
				txtlib_printText(lib, " ");
			}
			if (value.type == ValueTypeString)
			{
				if (debug)
				{
					txtlib_printText(lib, value.v.stringValue->chars);
				}
				rcstring_release(value.v.stringValue);
			}
			else if (value.type == ValueTypeFloat)
			{
				if (debug)
				{
					char buffer[20];
					snprintf(buffer, 20, "%0.7g", value.v.floatValue);
					txtlib_printText(lib, buffer);
				}
			}
		}
	} while (interpreter->pc->type == TokenComma);

	if (interpreter->pass == PassRun && debug)
	{
		txtlib_printText(lib, "\n");
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_MESSAGE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct TextLib *lib = &core->overlay->textLib;

	// MESSAGE
	++interpreter->pc;

	struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		if (value.type == ValueTypeString)
		{
			overlay_message(core, value.v.stringValue->chars);
			rcstring_release(value.v.stringValue);
		}
		else
			return ErrorTypeMismatch;
	}

	return itp_endOfCommand(interpreter);
}
//
// Copyright 2017-2020 Timo Kloss
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

#include "core.h"
#include "core.h"

enum ErrorCode cmd_LET(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // LET keyword is optional
    if (interpreter->pc->type == TokenLET)
    {
        ++interpreter->pc;
        if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return ErrorSyntax;
    }

    // identifier
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (interpreter->pc->type != TokenEq) return ErrorSyntax;
    ++interpreter->pc;

    // value
    struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
    if (value.type == ValueTypeError) return value.v.errorCode;
    if (value.type != valueType) return ErrorTypeMismatch;

    if (interpreter->pass == PassRun)
    {
        if (valueType == ValueTypeString && varValue->stringValue)
        {
            rcstring_release(varValue->stringValue);
        }
        *varValue = value.v;
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DIM(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;

    bool isGlobal = false;
    struct Token *nextToken = interpreter->pc + 1;
    if (nextToken->type == TokenGLOBAL)
    {
        ++interpreter->pc;
        if (interpreter->pass == PassPrepare && interpreter->subLevel > 0) return ErrorGlobalInsideOfASubprogram;
        isGlobal = true;
    }

    do
    {
        // DIM, GLOBAL or comma
        ++interpreter->pc;

        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        ++interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
        {
            return ErrorSyntax;
        }

        int numDimensions = 0;
        int dimensionSizes[MAX_ARRAY_DIMENSIONS];

        if (interpreter->pc->type != TokenBracketOpen) return ErrorSyntax;
        ++interpreter->pc;

        for (int i = 0; i < MAX_ARRAY_DIMENSIONS; i++)
        {
            struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
            if (value.type == ValueTypeError) return value.v.errorCode;

            dimensionSizes[i] = value.v.floatValue + 1; // value is max index, so size is +1
            numDimensions++;

            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;
            }
            else
            {
                break;
            }
        }

        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;

        if (interpreter->pass == PassRun)
        {
            enum ErrorCode errorCode = ErrorNone;
            struct ArrayVariable *variable = var_dimVariable(interpreter, &errorCode, tokenIdentifier->symbolIndex, numDimensions, dimensionSizes);
            if (!variable) return errorCode;
            variable->type = (tokenIdentifier->type == TokenStringIdentifier) ? ValueTypeString : ValueTypeFloat;
            if (isGlobal)
            {
                variable->subLevel = SUB_LEVEL_GLOBAL;
            }
            interpreter->cycles += variable->numValues;
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_UBOUND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // UBOUND
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // array
    if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return val_makeError(ErrorSyntax);
    int symbolIndex = interpreter->pc->symbolIndex;
    ++interpreter->pc;

    int d = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;

        // dimension value
        struct TypedValue dValue = itp_evaluateNumericExpression(core, 1, MAX_ARRAY_DIMENSIONS);
        if (dValue.type == ValueTypeError) return val_makeError(dValue.v.errorCode);

        d = dValue.v.floatValue - 1;
    }

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        struct ArrayVariable *variable = var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel);
        if (!variable) return val_makeError(ErrorArrayNotDimensionized);

        value.v.floatValue = variable->dimensionSizes[d] - 1;
    }
    return value;
}

enum ErrorCode cmd_SWAP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SWAP
    ++interpreter->pc;

    enum ErrorCode errorCode = ErrorNone;

    // x identifier
    enum ValueType xValueType = ValueTypeNull;
    union Value *xVarValue = itp_readVariable(core, &xValueType, &errorCode, false);
    if (!xVarValue) return errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // y identifier
    enum ValueType yValueType = ValueTypeNull;
    union Value *yVarValue = itp_readVariable(core, &yValueType, &errorCode, false);
    if (!yVarValue) return errorCode;

    if (xValueType != yValueType) return ErrorTypeMismatch;

    if (interpreter->pass == PassRun)
    {
        union Value spareValue = *xVarValue;
        *xVarValue = *yVarValue;
        *yVarValue = spareValue;
    }

    return itp_endOfCommand(interpreter);
}
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

#include "core.h"
#include "core.h"

void dat_nextData(struct Interpreter *interpreter)
{
    interpreter->currentDataValueToken++;
    if (interpreter->currentDataValueToken->type == TokenComma)
    {
        // value follows
        interpreter->currentDataValueToken++;
    }
    else
    {
        // next DATA line
        interpreter->currentDataToken = interpreter->currentDataToken->jumpToken;
        if (interpreter->currentDataToken)
        {
            interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
        }
        else
        {
            interpreter->currentDataValueToken = NULL;
        }
    }
}

void dat_restoreData(struct Interpreter *interpreter, struct Token *jumpToken)
{
    if (jumpToken)
    {
        struct Token *dataToken = interpreter->firstData;
        while (dataToken && dataToken < jumpToken)
        {
            dataToken = dataToken->jumpToken;
        }
        interpreter->currentDataToken = dataToken;
    }
    else
    {
        interpreter->currentDataToken = interpreter->firstData;
    }

    if (interpreter->currentDataToken)
    {
        interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
    }
    else
    {
        interpreter->currentDataValueToken = NULL;
    }
}

struct Token* dat_reachData(struct Interpreter *interpreter, struct Token *jumpToken)
{
    struct Token *dataToken=NULL;
    if (jumpToken)
    {
        dataToken = interpreter->firstData;
        while (dataToken && dataToken < jumpToken)
        {
            dataToken = dataToken->jumpToken;
        }
    }
    return dataToken;
}

struct Token* dat_readData(struct Token *dataToken, int skip)
{
    while(dataToken)
    {
				switch(dataToken->type)
				{
        		case TokenDATA:
								dataToken+=1;
								break;

						case TokenREM:
						case TokenApostrophe:
								dataToken+=1;
								break;

        		case TokenComma:
								dataToken+=1;
								break;

        		case TokenEol:
								dataToken+=1;
								break;

        		case TokenFloat:
						case TokenMinus:
						case TokenString:
            		if(skip-->0) dataToken+=dataToken->type==TokenMinus?2:1;
            		else return dataToken;
								break;

						default:
								return NULL;
        }
    }
    return NULL;
}

float dat_readFloat(struct Token *jumpToken, int skip, float def)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return def;
    if(dataToken->type==TokenMinus) return -(dataToken+1)->floatValue;
    if(dataToken->type==TokenFloat) return dataToken->floatValue;
    else return def;
}

uint8_t dat_readU8(struct Token *jumpToken, int skip, uint8_t def)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return def;
    if(dataToken->type==TokenMinus) return -(uint8_t)((dataToken+1)->floatValue);
    if(dataToken->type==TokenFloat) return (uint8_t)dataToken->floatValue;
    else return def;
}

struct RCString *dat_readString(struct Token *jumpToken, int skip)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return NULL;
    if(dataToken->type==TokenString) return dataToken->stringValue;
    else return NULL;
}
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

#include "core.h"

const char *ErrorStrings[] = {
    "OK",

    "Could Not Open Program",
    "Too Many Tokens",
    "ROM Is Full",
    "Index Already Defined",
    "Unterminated String",
    "Unexpected Character",
    "Reserved Keyword",
    "Syntax Error",
    "Symbol Name Too Long",
    "Too Many Symbols",
    "Type Mismatch",
    "Out Of Memory",
    "ELSE Without IF",
    "END IF Without IF",
    "Expected Command",
    "NEXT Without FOR",
    "LOOP Without DO",
    "UNTIL Without REPEAT",
    "WEND Without WHILE",
    "Label Already Defined",
    "Too Many Labels",
    "ErrorExpectedLabel",
    "Undefined Label",
    "Array Not Dimensionized",
    "Array Already Dimensionized",
    "Variable Already Used",
    "Index Out Of Bounds",
    "Wrong Number Of Dimensions",
    "Invalid Parameter",
    "RETURN Without GOSUB",
    "Stack Overflow",
    "Out Of Data",
    "Illegal Memory Access",
    "Too Many CPU Cycles In Interrupt",
    "Not Allowed In Interrupt",
    "IF Without END IF",
    "FOR Without NEXT",
    "DO Without LOOP",
    "REPEAT Without UNTIL",
    "WHILE Without WEND",
    "EXIT Not Inside Loop",
    "Directory Not Loaded",
    "Division By Zero",
    "Variable Not Initialized",
    "Array Variable Without Index",
    "END SUB Without SUB",
    "SUB Without END SUB",
    "SUB Cannot Be Nested",
    "Undefined Subprogram",
    "Expected Subprogram Name",
    "Argument Count Mismatch",
    "SUB Already Defined",
    "Too Many Subprograms",
    "SHARED Outside Of A Subprogram",
    "GLOBAL Inside Of A Subprogram",
    "EXIT SUB Outside Of A Subprogram",
    "Automatic Pause Not Disabled",
    "Not Allowed Outside Of Interrupt",
		"Not enough storage space on the device",

		"Out of error"
};

const char *err_getString(enum ErrorCode errorCode)
{
    return ErrorStrings[errorCode];
}

struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition)
{
    struct CoreError error = {code, sourcePosition};
    return error;
}

struct CoreError err_noCoreError(void)
{
    struct CoreError error = {ErrorNone, 0};
    return error;
}
//
// Copyright 2016-2020 Timo Kloss
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

#include "core.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"

struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level);
struct TypedValue itp_evaluatePrimaryExpression(struct Core *core);
struct TypedValue itp_evaluateFunction(struct Core *core);
enum ErrorCode itp_evaluateCommand(struct Core *core);

void itp_init(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	interpreter->romDataManager.data = core->machine->cartridgeRom;

	// global null string
	interpreter->nullString = rcstring_new(NULL, 0);
	if (!interpreter->nullString)
		exit(EXIT_FAILURE);
}

void itp_deinit(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	itp_freeProgram(core);

	// Free null string
	if (interpreter->nullString)
	{
		rcstring_release(interpreter->nullString);
		interpreter->nullString = NULL;
	}
}

struct CoreError itp_compileProgram(struct Core *core, const char *sourceCode)
{
	struct Interpreter *interpreter = core->interpreter;

	itp_freeProgram(core);

	// Parse source code

	interpreter->sourceCode = uppercaseString(sourceCode);
	if (!interpreter->sourceCode)
		return err_makeCoreError(ErrorOutOfMemory, -1);

	struct CoreError error = tok_tokenizeUppercaseProgram(&interpreter->tokenizer, interpreter->sourceCode);
	if (error.code != ErrorNone)
	{
		return error;
	}

	struct DataManager *romDataManager = &interpreter->romDataManager;
	error = data_uppercaseImport(romDataManager, interpreter->sourceCode, false);
	if (error.code != ErrorNone)
		return error;

	// add default characters if ROM entry 0 is unused
	struct DataEntry *entry0 = &romDataManager->entries[0];
	if (entry0->length == 0 && (DATA_SIZE - data_currentSize(romDataManager)) >= 1024)
	{
		data_setEntry(romDataManager, 0, "FONT", (uint8_t *)DefaultCharacters, 1024);
	}

	// Prepare commands

	interpreter->pc = interpreter->tokenizer.tokens;
	interpreter->pass = PassPrepare;
	interpreter->exitEvaluation = false;
	interpreter->subLevel = 0;
	interpreter->numLabelStackItems = 0;
	interpreter->isSingleLineIf = false;
	interpreter->compat = false;

	enum ErrorCode errorCode;
	do
	{
		errorCode = itp_evaluateCommand(core);
	} while (errorCode == ErrorNone && interpreter->pc->type != TokenUndefined);

	if (errorCode != ErrorNone)
		return err_makeCoreError(errorCode, interpreter->pc->sourcePosition);

	if (interpreter->numLabelStackItems > 0)
	{
		struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
		errorCode = itp_labelStackError(item);
		if (errorCode != ErrorNone)
		{
			return err_makeCoreError(errorCode, item->token->sourcePosition);
		}
	}

	// prepare for run

	interpreter->pc = interpreter->tokenizer.tokens;
	interpreter->cycles = 0;
	interpreter->interruptOverCycles = 0;
	interpreter->waitTap = false;
	interpreter->pass = PassRun;
	interpreter->state = StateEvaluate;
	interpreter->mode = ModeNone;
	interpreter->handlesPause = true;
	interpreter->currentDataToken = interpreter->firstData;
	interpreter->currentDataValueToken = interpreter->firstData ? interpreter->firstData + 1 : NULL;
	interpreter->isSingleLineIf = false;
	interpreter->lastFrameIOStatus.value = 0;
	interpreter->seed = 0;

	memset(&interpreter->textLib, 0, sizeof(struct TextLib));
	memset(&interpreter->spritesLib, 0, sizeof(struct SpritesLib));
	memset(&interpreter->audioLib, 0, sizeof(struct AudioLib));
	memset(&interpreter->particlesLib, 0, sizeof(struct ParticlesLib));
	interpreter->textLib.core = core;
	interpreter->spritesLib.core = core;
	interpreter->audioLib.core = core;
	interpreter->particlesLib.core = core;

	pcg32_srandom_r(&(interpreter->defaultRng), 4715711917271117164, (intptr_t)&interpreter->defaultRng);

	return err_noCoreError();
}

void itp_runProgram(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	switch (interpreter->state)
	{
	case StateEvaluate:
	{
		if (interpreter->waitTap)
		{
			if (core->machine->ioRegisters.status.touch && !interpreter->lastFrameIOStatus.touch)
			{
				interpreter->waitTap = false;
			}
			else
			{
				interpreter->waitCount = 1;
			}
		}

		if (interpreter->waitCount > 0)
		{
			--interpreter->waitCount;
			break;
		}

		interpreter->mode = ModeMain;
		interpreter->exitEvaluation = false;
		enum ErrorCode errorCode = ErrorNone;

		while (errorCode == ErrorNone && interpreter->cycles < MAX_CYCLES_TOTAL_PER_FRAME && interpreter->state == StateEvaluate && !interpreter->exitEvaluation)
		{
			errorCode = itp_evaluateCommand(core);
		}

		if (interpreter->cycles >= MAX_CYCLES_TOTAL_PER_FRAME)
		{
			machine_suspendEnergySaving(core, 2);
		}

		interpreter->mode = ModeNone;
		if (errorCode != ErrorNone)
		{
			itp_endProgram(core);
			delegate_interpreterDidFail(core, err_makeCoreError(errorCode, interpreter->pc->sourcePosition));
		}
		break;
	}

	case StateInput:
	{
		if (txtlib_inputUpdate(&interpreter->textLib))
		{
			interpreter->state = StateEvaluate;
			cmd_endINPUT(core);
		}
		break;
	}

	case StateNoProgram:
	case StatePaused:
	case StateEnd:
	case StateWaitForDisk:
		break;
	}
}

void itp_runInterrupt(struct Core *core, enum InterruptType type)
{
	struct Interpreter *interpreter = core->interpreter;

	switch (interpreter->state)
	{
	case StateEvaluate:
	case StateInput:
	case StatePaused:
	case StateWaitForDisk:
	{
		struct Token *startToken = NULL;
		int maxCycles;

		int mainCycles = interpreter->cycles;
		interpreter->cycles = 0;

		switch (type)
		{
		case InterruptTypeRaster:
			startToken = interpreter->currentOnRasterToken;
			maxCycles = MAX_CYCLES_PER_RASTER;
			break;

		case InterruptTypeVBL:
			startToken = interpreter->currentOnVBLToken;
			maxCycles = MAX_CYCLES_PER_VBL;
			// update audio player
			audlib_update(&interpreter->audioLib);
			break;

		case InterruptTypeParticle:
			startToken = interpreter->currentOnParticleToken;
			maxCycles = MAX_CYCLES_PER_PARTICLE;
			break;

		case InterruptTypeEmitter:
			startToken = interpreter->currentOnEmitterToken;
			maxCycles = MAX_CYCLES_PER_EMITTER;
			break;
		}

		if (startToken)
		{
			interpreter->mode = ModeInterrupt;
			interpreter->exitEvaluation = false;
			struct Token *pc = interpreter->pc;
			interpreter->pc = startToken;
			interpreter->subLevel++;

			enum ErrorCode errorCode = ErrorNone;

			if (type == InterruptTypeParticle)
			{
				if (interpreter->pc->type == TokenBracketOpen)
				{
					// SUB gnagna (
					++interpreter->pc;

					// SUB gnagna ( sprite_id
					struct Token *tokenIdentifier = interpreter->pc;
					if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
						errorCode = ErrorSyntax;
					enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
					if (varType == ValueTypeFloat)
					{
						// pass by value
						struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, 1, 1, varType, NULL);
						if (variable)
						{
							if (interpreter->pass == PassRun)
								variable->v.floatValue = (float)interpreter->particlesLib.interrupt_sprite_id;
							variable->symbolIndex = tokenIdentifier->symbolIndex;
						}
						++interpreter->pc;
					}
					else
						errorCode = ErrorTypeMismatch;

					// SUB gnagna ( sprite_id, )
					if (interpreter->pc->type != TokenComma)
						errorCode = ErrorArgumentCountMismatch;
					++interpreter->pc;

					// SUB gnagna ( sprite_id, particle_addr
					tokenIdentifier = interpreter->pc;
					if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
						errorCode = ErrorSyntax;
					varType = itp_getIdentifierTokenValueType(tokenIdentifier);
					if (varType == ValueTypeFloat)
					{
						// pass by value
						struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, 1, 1, varType, NULL);
						if (variable)
						{
							if (interpreter->pass == PassRun)
								variable->v.floatValue = (float)interpreter->particlesLib.interrupt_particle_addr;
							variable->symbolIndex = tokenIdentifier->symbolIndex;
						}
						++interpreter->pc;
					}
					else
						errorCode = ErrorTypeMismatch;

					// SUB gnagna ( sprite_id, particle_addr )
					if (interpreter->pc->type != TokenBracketClose)
						errorCode = ErrorSyntax;
					++interpreter->pc;
				}
				else
					errorCode = ErrorArgumentCountMismatch;
			}

			else if (type == InterruptTypeEmitter)
			{
				if (interpreter->pc->type == TokenBracketOpen)
				{
					// SUB gnagna (
					++interpreter->pc;

					// SUB gnagna ( emitter_id
					struct Token *tokenIdentifier = interpreter->pc;
					if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
						errorCode = ErrorSyntax;
					enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
					if (varType == ValueTypeFloat)
					{
						// pass by value
						struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, 1, 1, varType, NULL);
						if (variable)
						{
							if (interpreter->pass == PassRun)
								variable->v.floatValue = (float)interpreter->particlesLib.interrupt_emitter_id;
							variable->symbolIndex = tokenIdentifier->symbolIndex;
						}
						++interpreter->pc;
					}
					else
						errorCode = ErrorTypeMismatch;

					// SUB gnagna ( emitter_id, )
					if (interpreter->pc->type != TokenComma)
						errorCode = ErrorArgumentCountMismatch;
					++interpreter->pc;

					// SUB gnagna ( emitter_id, emitter_addr
					tokenIdentifier = interpreter->pc;
					if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
						errorCode = ErrorSyntax;
					varType = itp_getIdentifierTokenValueType(tokenIdentifier);
					if (varType == ValueTypeFloat)
					{
						// pass by value
						struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, 1, 1, varType, NULL);
						if (variable)
						{
							if (interpreter->pass == PassRun)
								variable->v.floatValue = (float)interpreter->particlesLib.interrupt_emitter_addr;
							variable->symbolIndex = tokenIdentifier->symbolIndex;
						}
						++interpreter->pc;
					}
					else
						errorCode = ErrorTypeMismatch;

					// SUB gnagna ( emitter_id, emitter_addr )
					if (interpreter->pc->type != TokenBracketClose)
						errorCode = ErrorSyntax;
					++interpreter->pc;
				}
				else
					errorCode = ErrorArgumentCountMismatch;
			}

			if (errorCode != ErrorNone)
			{
				itp_endProgram(core);
				delegate_interpreterDidFail(core, err_makeCoreError(errorCode, interpreter->pc->sourcePosition));
			}

			errorCode = lab_pushLabelStackItem(interpreter, LabelTypeONCALL, NULL);

			while (errorCode == ErrorNone
						 // cycles can exceed interrupt limit (see interruptOverCycles), but there is still a hard limit for extreme cases
						 && interpreter->cycles < MAX_CYCLES_TOTAL_PER_FRAME && !interpreter->exitEvaluation)
			{
				errorCode = itp_evaluateCommand(core);
			}

			interpreter->mode = ModeNone;

			if (interpreter->cycles >= MAX_CYCLES_TOTAL_PER_FRAME)
			{
				itp_endProgram(core);
				delegate_interpreterDidFail(core, err_makeCoreError(ErrorTooManyCPUCyclesInInterrupt, interpreter->pc->sourcePosition));
			}
			else if (errorCode != ErrorNone)
			{
				itp_endProgram(core);
				delegate_interpreterDidFail(core, err_makeCoreError(errorCode, interpreter->pc->sourcePosition));
			}
			else
			{
				interpreter->pc = pc;
			}
		}

		// calculate cycles exceeding limit
		interpreter->interruptOverCycles += interpreter->cycles - maxCycles;
		if (interpreter->interruptOverCycles < 0)
		{
			interpreter->interruptOverCycles = 0;
		}

		// sum of interrupt's and main cycle count
		interpreter->cycles += mainCycles;

		break;
	}

	case StateNoProgram:
	case StateEnd:
		break;
	}
}

void itp_didFinishVBL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// // remember this frame's IO
	// for (int i = 0; i < NUM_GAMEPADS; i++)
	// {
	//     interpreter->lastFrameGamepads[i] = core->machine->ioRegisters.gamepads[i];
	// }
	interpreter->lastFrameIOStatus = core->machine->ioRegisters.status;

	// timer
	interpreter->timer++;
	if (interpreter->timer >= TIMER_WRAP_VALUE)
	{
		interpreter->timer = 0;
	}

	// pause
	if (core->machine->ioRegisters.status.pause)
	{
		if (interpreter->handlesPause && interpreter->state == StateEvaluate)
		{
			interpreter->state = StatePaused;
			overlay_updateState(core);
			core->machine->ioRegisters.status.pause = 0;
		}
		else if (interpreter->state == StatePaused)
		{
			interpreter->state = StateEvaluate;
			overlay_updateState(core);
			core->machine->ioRegisters.status.pause = 0;
		}
	}

	// CPU load (rounded up)
	int currentCpuLoad = (interpreter->cycles * 100 + MAX_CYCLES_TOTAL_PER_FRAME - 1) / MAX_CYCLES_TOTAL_PER_FRAME;
	if (currentCpuLoad > interpreter->cpuLoadMax)
	{
		interpreter->cpuLoadMax = currentCpuLoad;
	}
	++interpreter->cpuLoadTimer;
	if (interpreter->cpuLoadTimer >= 30)
	{
		interpreter->cpuLoadTimer = 0;
		interpreter->cpuLoadDisplay = interpreter->cpuLoadMax;
		interpreter->cpuLoadMax = currentCpuLoad;
	}

	// reset CPU cycles
	interpreter->cycles = interpreter->cycles - MAX_CYCLES_TOTAL_PER_FRAME;
	if (interpreter->cycles < 0)
	{
		interpreter->cycles = 0;
	}
}

void itp_endProgram(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	interpreter->state = StateEnd;
	interpreter->interruptOverCycles = 0;
}

void itp_freeProgram(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	interpreter->state = StateNoProgram;
	interpreter->firstData = NULL;
	interpreter->lastData = NULL;
	interpreter->currentDataToken = NULL;
	interpreter->currentDataValueToken = NULL;
	interpreter->currentOnRasterToken = NULL;
	interpreter->currentOnVBLToken = NULL;
	interpreter->lastVariableValue = NULL;

	var_freeSimpleVariables(interpreter, SUB_LEVEL_GLOBAL);
	var_freeArrayVariables(interpreter, SUB_LEVEL_GLOBAL);
	tok_freeTokens(&interpreter->tokenizer);

	if (interpreter->sourceCode)
	{
		free((void *)interpreter->sourceCode);
		interpreter->sourceCode = NULL;
	}
}

enum ValueType itp_getIdentifierTokenValueType(struct Token *token)
{
	if (token->type == TokenIdentifier)
	{
		return ValueTypeFloat;
	}
	else if (token->type == TokenStringIdentifier)
	{
		return ValueTypeString;
	}
	return ValueTypeNull;
}

union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode, bool forWriting)
{
	struct Interpreter *interpreter = core->interpreter;

	struct Token *tokenIdentifier = interpreter->pc;

	if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
	{
		*errorCode = ErrorSyntax;
		return NULL;
	}

	enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
	if (type)
	{
		*type = varType;
	}

	int symbolIndex = tokenIdentifier->symbolIndex;
	++interpreter->pc;
	++interpreter->cycles;

	if (interpreter->pc->type == TokenBracketOpen)
	{
		// array
		++interpreter->pc;

		struct ArrayVariable *variable = NULL;
		if (interpreter->pass == PassRun)
		{
			variable = var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel);
			if (!variable)
			{
				*errorCode = ErrorArrayNotDimensionized;
				return NULL;
			}
		}

		int indices[MAX_ARRAY_DIMENSIONS];
		int numDimensions = 0;

		for (int i = 0; i < MAX_ARRAY_DIMENSIONS; i++)
		{
			struct TypedValue indexValue = itp_evaluateExpression(core, TypeClassNumeric);
			if (indexValue.type == ValueTypeError)
			{
				*errorCode = indexValue.v.errorCode;
				return NULL;
			}

			numDimensions++;

			if (interpreter->pass == PassRun)
			{
				if (numDimensions <= variable->numDimensions && (indexValue.v.floatValue < 0 || indexValue.v.floatValue >= variable->dimensionSizes[i]))
				{
					*errorCode = ErrorIndexOutOfBounds;
					return NULL;
				}

				indices[i] = indexValue.v.floatValue;
			}

			if (interpreter->pc->type == TokenComma)
			{
				++interpreter->pc;
			}
			else
			{
				break;
			}
		}

		if (interpreter->pc->type != TokenBracketClose)
		{
			*errorCode = ErrorSyntax;
			return NULL;
		}
		++interpreter->pc;

		if (interpreter->pass == PassRun)
		{
			if (numDimensions != variable->numDimensions)
			{
				*errorCode = ErrorWrongNumberOfDimensions;
				return NULL;
			}
			return var_getArrayValue(interpreter, variable, indices);
		}
	}
	else
	{
		// simple variable
		if (interpreter->pass == PassRun)
		{
			struct SimpleVariable *variable = var_getSimpleVariable(interpreter, symbolIndex, interpreter->subLevel);
			if (!variable)
			{
				// check if variable name is already used for array
				if (var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel))
				{
					*errorCode = ErrorArrayVariableWithoutIndex;
					return NULL;
				}
				if (!forWriting)
				{
					*errorCode = ErrorVariableNotInitialized;
					return NULL;
				}
				variable = var_createSimpleVariable(interpreter, errorCode, symbolIndex, interpreter->subLevel, varType, NULL);
				if (!variable)
					return NULL;
			}
			if (variable->isReference)
			{
				return variable->v.reference;
			}
			return &variable->v;
		}
	}
	return &ValueDummy;
}

enum ErrorCode itp_checkTypeClass(struct Interpreter *interpreter, enum ValueType valueType, enum TypeClass typeClass)
{
	if (interpreter->pass == PassPrepare && valueType != ValueTypeError)
	{
		if (typeClass == TypeClassString && valueType != ValueTypeString)
		{
			return ErrorTypeMismatch;
		}
		else if (typeClass == TypeClassNumeric && valueType != ValueTypeFloat)
		{
			return ErrorTypeMismatch;
		}
	}
	return ErrorNone;
}

struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass)
{
	struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
	if (value.type != ValueTypeError)
	{
		enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, typeClass);
		if (errorCode != ErrorNone)
		{
			value.type = ValueTypeError;
			value.v.errorCode = errorCode;
		}
	}
	return value;
}

struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max)
{
	struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
	if (value.type != ValueTypeError)
	{
		enum ErrorCode errorCode = ErrorNone;
		if (core->interpreter->pass == PassPrepare)
		{
			if (value.type != ValueTypeFloat)
			{
				errorCode = ErrorTypeMismatch;
			}
		}
		else if (core->interpreter->pass == PassRun)
		{
			if ((int)value.v.floatValue < min || (int)value.v.floatValue > max)
			{
				errorCode = ErrorInvalidParameter;
			}
		}
		if (errorCode != ErrorNone)
		{
			value.type = ValueTypeError;
			value.v.errorCode = errorCode;
		}
	}
	return value;
}

struct TypedValue itp_evaluateOptionalExpression(struct Core *core, enum TypeClass typeClass)
{
	if (core->interpreter->pc->type == TokenComma || core->interpreter->pc->type == TokenBracketClose || itp_isEndOfCommand(core->interpreter))
	{
		struct TypedValue value;
		value.type = ValueTypeNull;
		return value;
	}
	return itp_evaluateExpression(core, typeClass);
}

struct TypedValue itp_evaluateOptionalNumericExpression(struct Core *core, int min, int max)
{
	if (core->interpreter->pc->type == TokenComma || core->interpreter->pc->type == TokenBracketClose || itp_isEndOfCommand(core->interpreter))
	{
		struct TypedValue value;
		value.type = ValueTypeNull;
		return value;
	}
	return itp_evaluateNumericExpression(core, min, max);
}

bool itp_isTokenLevel(enum TokenType token, int level)
{
	switch (level)
	{
	case 0:
		return token == TokenXOR || token == TokenOR;
	case 1:
		return token == TokenAND;
		//        case 2:
		//            return token == TokenNOT;
	case 3:
		return token == TokenEq || token == TokenUneq || token == TokenGr || token == TokenLe || token == TokenGrEq || token == TokenLeEq;
	case 4:
		return token == TokenPlus || token == TokenMinus;
	case 5:
		return token == TokenMOD;
	case 6:
		return token == TokenMul || token == TokenDiv || token == TokenDivInt;
		//        case 7:
		//            return token == TokenPlus || token == TokenMinus; // unary
	case 8:
		return token == TokenPow;
	}
	return false;
}

struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level)
{
	struct Interpreter *interpreter = core->interpreter;
	enum TokenType type = interpreter->pc->type;

	if (level == 2 && type == TokenNOT)
	{
		++interpreter->pc;
		++interpreter->cycles;
		struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
		if (value.type == ValueTypeError)
			return value;
		enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, TypeClassNumeric);
		if (errorCode != ErrorNone)
		{
			value.type = ValueTypeError;
			value.v.errorCode = errorCode;
		}
		else
		{
			value.v.floatValue = ~((int)value.v.floatValue);
		}
		interpreter->lastVariableValue = NULL;
		return value;
	}
	if (level == 7 && (type == TokenPlus || type == TokenMinus)) // unary
	{
		++interpreter->pc;
		++interpreter->cycles;
		struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
		if (value.type == ValueTypeError)
			return value;
		enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, TypeClassNumeric);
		if (errorCode != ErrorNone)
		{
			value.type = ValueTypeError;
			value.v.errorCode = errorCode;
		}
		else if (type == TokenMinus)
		{
			value.v.floatValue = -value.v.floatValue;
		}
		interpreter->lastVariableValue = NULL;
		return value;
	}
	if (level == 9)
	{
		return itp_evaluatePrimaryExpression(core);
	}

	struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
	if (value.type == ValueTypeError)
		return value;

	while (itp_isTokenLevel(interpreter->pc->type, level))
	{
		enum TokenType type = interpreter->pc->type;
		++interpreter->pc;
		++interpreter->cycles;
		struct TypedValue rightValue = itp_evaluateExpressionLevel(core, level + 1);
		if (rightValue.type == ValueTypeError)
			return rightValue;

		struct TypedValue newValue;
		if (value.type != rightValue.type)
		{
			newValue.type = ValueTypeError;
			newValue.v.errorCode = ErrorTypeMismatch;
			return newValue;
		}

		if (value.type == ValueTypeFloat)
		{
			newValue.type = ValueTypeFloat;
			switch (type)
			{
			case TokenXOR:
			{
				int leftInt = value.v.floatValue;
				int rightInt = rightValue.v.floatValue;
				newValue.v.floatValue = (leftInt ^ rightInt);
				break;
			}
			case TokenOR:
			{
				int leftInt = value.v.floatValue;
				int rightInt = rightValue.v.floatValue;
				newValue.v.floatValue = (leftInt | rightInt);
				break;
			}
			case TokenAND:
			{
				int leftInt = value.v.floatValue;
				int rightInt = rightValue.v.floatValue;
				newValue.v.floatValue = (leftInt & rightInt);
				break;
			}
			case TokenEq:
			{
				newValue.v.floatValue = (value.v.floatValue == rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenUneq:
			{
				newValue.v.floatValue = (value.v.floatValue != rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenGr:
			{
				newValue.v.floatValue = (value.v.floatValue > rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenLe:
			{
				newValue.v.floatValue = (value.v.floatValue < rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenGrEq:
			{
				newValue.v.floatValue = (value.v.floatValue >= rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenLeEq:
			{
				newValue.v.floatValue = (value.v.floatValue <= rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
				break;
			}
			case TokenPlus:
			{
				newValue.v.floatValue = value.v.floatValue + rightValue.v.floatValue;
				break;
			}
			case TokenMinus:
			{
				newValue.v.floatValue = value.v.floatValue - rightValue.v.floatValue;
				break;
			}
			case TokenMOD:
			{
				if (interpreter->pass == PassRun)
				{
					int rightInt = (int)rightValue.v.floatValue;
					if (rightInt == 0)
					{
						newValue.type = ValueTypeError;
						newValue.v.errorCode = ErrorDivisionByZero;
					}
					else
					{
						newValue.v.floatValue = (int)value.v.floatValue % rightInt;
					}
				}
				break;
			}
			case TokenMul:
			{
				newValue.v.floatValue = value.v.floatValue * rightValue.v.floatValue;
				break;
			}
			case TokenDiv:
			{
				if (interpreter->pass == PassRun)
				{
					if (rightValue.v.floatValue == 0.0f)
					{
						newValue.type = ValueTypeError;
						newValue.v.errorCode = ErrorDivisionByZero;
					}
					else
					{
						newValue.v.floatValue = value.v.floatValue / rightValue.v.floatValue;
					}
				}
				break;
			}
			case TokenDivInt:
			{
				if (interpreter->pass == PassRun)
				{
					int rightInt = (int)rightValue.v.floatValue;
					if (rightInt == 0)
					{
						newValue.type = ValueTypeError;
						newValue.v.errorCode = ErrorDivisionByZero;
					}
					else
					{
						newValue.v.floatValue = (int)value.v.floatValue / rightInt;
					}
				}
				break;
			}
			case TokenPow:
			{
				newValue.v.floatValue = powf(value.v.floatValue, rightValue.v.floatValue);
				break;
			}
			default:
			{
				newValue.type = ValueTypeError;
				newValue.v.errorCode = ErrorSyntax;
			}
			}
		}
		else if (value.type == ValueTypeString)
		{
			switch (type)
			{
			case TokenEq:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) == 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenUneq:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) != 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenGr:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) > 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenLe:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) < 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenGrEq:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) >= 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenLeEq:
			{
				newValue.type = ValueTypeFloat;
				if (interpreter->pass == PassRun)
				{
					newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) <= 0) ? BAS_TRUE : BAS_FALSE;
				}
				break;
			}
			case TokenPlus:
			{
				newValue.type = ValueTypeString;
				if (interpreter->pass == PassRun)
				{
					size_t len1 = strlen(value.v.stringValue->chars);
					size_t len2 = strlen(rightValue.v.stringValue->chars);
					newValue.v.stringValue = rcstring_new(NULL, len1 + len2);
					strcpy(newValue.v.stringValue->chars, value.v.stringValue->chars);
					strcpy(&newValue.v.stringValue->chars[len1], rightValue.v.stringValue->chars);
					interpreter->cycles += len1 + len2;
				}
				break;
			}
			case TokenXOR:
			case TokenOR:
			case TokenAND:
			case TokenMinus:
			case TokenMOD:
			case TokenMul:
			case TokenDiv:
			case TokenDivInt:
			case TokenPow:
			{
				newValue.type = ValueTypeError;
				newValue.v.errorCode = ErrorTypeMismatch;
				break;
			}
			default:
			{
				newValue.type = ValueTypeError;
				newValue.v.errorCode = ErrorSyntax;
			}
			}
			if (interpreter->pass == PassRun)
			{
				rcstring_release(value.v.stringValue);
				rcstring_release(rightValue.v.stringValue);
			}
		}
		else
		{
			assert(0);
			newValue.v.floatValue = 0;
		}

		value = newValue;
		interpreter->lastVariableValue = NULL;
		if (value.type == ValueTypeError)
			break;
	}
	return value;
}

struct TypedValue itp_evaluatePrimaryExpression(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// check for function
	struct TypedValue value = itp_evaluateFunction(core);
	if (value.type != ValueTypeNull)
	{
		++interpreter->cycles;
		interpreter->lastVariableValue = NULL;
		return value;
	}

	interpreter->lastVariableValue = NULL;

	// native types
	switch (interpreter->pc->type)
	{
	case TokenFloat:
	{
		value.type = ValueTypeFloat;
		value.v.floatValue = interpreter->pc->floatValue;
		++interpreter->pc;
		++interpreter->cycles;
		break;
	}
	case TokenString:
	{
		value.type = ValueTypeString;
		value.v.stringValue = interpreter->pc->stringValue;
		if (interpreter->pass == PassRun)
		{
			rcstring_retain(interpreter->pc->stringValue);
		}
		++interpreter->pc;
		++interpreter->cycles;
		break;
	}
	case TokenIdentifier:
	case TokenStringIdentifier:
	{
		enum ErrorCode errorCode = ErrorNone;
		enum ValueType valueType = ValueTypeNull;

		// is a label name
		struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, interpreter->pc->symbolIndex);
		if (item)
		{
			struct RCString *str = dat_readString(item->token, 0);
			if (str)
			{
				value.type = ValueTypeString;
				value.v.stringValue = str;
				if (interpreter->pass == PassRun) rcstring_retain(value.v.stringValue);
			}
			else
			{
				value.type = ValueTypeFloat;
				value.v.floatValue = dat_readFloat(item->token, 0, 0.0);
			}
			++interpreter->pc;
			++interpreter->cycles;
			break;
		}

		// is a variable name
		union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
		if (varValue)
		{
			value.type = valueType;
			value.v = *varValue;
			interpreter->lastVariableValue = varValue;
			if (interpreter->pass == PassRun && valueType == ValueTypeString)
			{
				rcstring_retain(varValue->stringValue);
			}
			break;
		}

		value.type = ValueTypeError;
		value.v.errorCode = errorCode;
		break;
	}
	case TokenBracketOpen:
	{
		++interpreter->pc;
		value = itp_evaluateExpression(core, TypeClassAny);
		if (value.type == ValueTypeError)
			return value;
		if (interpreter->pc->type != TokenBracketClose)
		{
			value.type = ValueTypeError;
			value.v.errorCode = ErrorSyntax;
		}
		else
		{
			++interpreter->pc;
			interpreter->lastVariableValue = NULL;
		}
		break;
	}
	default:
	{
		value.type = ValueTypeError;
		value.v.errorCode = ErrorSyntax;
	}
	}
	return value;
}

bool itp_isEndOfCommand(struct Interpreter *interpreter)
{
	enum TokenType type = interpreter->pc->type;
	return (type == TokenEol || type == TokenELSE);
}

enum ErrorCode itp_endOfCommand(struct Interpreter *interpreter)
{
	enum TokenType type = interpreter->pc->type;
	if (type == TokenEol)
	{
		interpreter->isSingleLineIf = false;
		++interpreter->pc;
		return ErrorNone;
	}
	return (type == TokenELSE) ? ErrorNone : ErrorSyntax;
}

enum TokenType itp_getNextTokenType(struct Interpreter *interpreter)
{
	return (interpreter->pc + 1)->type;
}

struct TypedValue itp_evaluateFunction(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	switch (interpreter->pc->type)
	{
	case TokenASC:
		return fnc_ASC(core);

	case TokenBIN:
	case TokenHEX:
		return fnc_BIN_HEX(core);

	case TokenCHR:
		return fnc_CHR(core);

	case TokenINSTR:
		return fnc_INSTR(core);

	case TokenLEFTStr:
	case TokenRIGHTStr:
		return fnc_LEFTStr_RIGHTStr(core);

	case TokenLEN:
		return fnc_LEN(core);

	case TokenMID:
		return fnc_MID(core);

	case TokenSTR:
		return fnc_STR(core);

	case TokenVAL:
		return fnc_VAL(core);

	case TokenPEEK:
	case TokenPEEKW:
	case TokenPEEKL:
		return fnc_PEEK(core);

	case TokenPI:
		return fnc_math0(core);

	case TokenABS:
	case TokenACOS:
	case TokenASIN:
	case TokenATAN:
	case TokenCOS:
	case TokenEXP:
	case TokenHCOS:
	case TokenHSIN:
	case TokenHTAN:
	case TokenINT:
	case TokenLOG:
	case TokenSGN:
	case TokenSIN:
	case TokenSQR:
	case TokenTAN:
		return fnc_math1(core);

	case TokenMAX:
	case TokenMIN:
		return fnc_math2(core);

	case TokenCLAMP:
		return fnc_math3(core);

	case TokenRND:
		return fnc_RND(core);

	case TokenINKEY:
		return fnc_INKEY(core);

	case TokenUBOUND:
		return fnc_UBOUND(core);

	case TokenROM:
	case TokenSIZE:
		return fnc_ROM_SIZE(core);

	case TokenCOLOR:
		return fnc_COLOR(core);

	case TokenTIMER:
	case TokenRASTER:
	// case TokenDISPLAY:
		return fnc_screen0(core);

	case TokenSCROLLX:
	case TokenSCROLLY:
		return fnc_SCROLL_X_Y(core);

	case TokenCELLA:
	case TokenCELLC:
		return fnc_CELL(core);

	case TokenMCELLA:
	case TokenMCELLC:
		return fnc_MCELL(core);

	case TokenCURSORX:
	case TokenCURSORY:
		return fnc_CURSOR(core);

		// case TokenUP:
		// case TokenDOWN:
		// case TokenLEFT:
		// case TokenRIGHT:
		//     return fnc_UP_DOWN_LEFT_RIGHT(core);

		// case TokenBUTTON:
		//     return fnc_BUTTON(core);

	case TokenSPRITEX:
	case TokenSPRITEY:
	case TokenSPRITEC:
	case TokenSPRITEA:
		return fnc_SPRITE(core);

	case TokenSPRITE:
		return fnc_SPRITE_HIT(core);

	case TokenHIT:
		return fnc_HIT(core);

	case TokenTOUCH:
		return fnc_TOUCH(core);

	case TokenTAP:
		return fnc_TAP(core);

	case TokenTOUCHX:
	case TokenTOUCHY:
		return fnc_TOUCH_X_Y(core);

	case TokenSHOWNW:
	case TokenSHOWNH:
		return fnc_SHOWN(core);

	case TokenSAFEL:
	case TokenSAFET:
	case TokenSAFER:
	case TokenSAFEB:
		return fnc_SAFE(core);

	case TokenFILE:
		return fnc_FILE(core);

	case TokenFSIZE:
		return fnc_FSIZE(core);

	case TokenPAUSE:
		return fnc_PAUSE(core);

	case TokenMUSIC:
		return fnc_MUSIC(core);

	case TokenEASE:
		return fnc_EASE(core);

	case TokenCEIL:
		return fnc_math1(core);

	case TokenFLOOR:
		return fnc_math1(core);

	case TokenKEYBOARD:
		return fnc_KEYBOARD(core);

	default:
		break;
	}
	struct TypedValue value;
	value.type = ValueTypeNull;
	return value;
}

enum ErrorCode itp_evaluateCommand(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	enum TokenType type = interpreter->pc->type;
	if (type != TokenREM && type != TokenApostrophe && type != TokenEol && type != TokenUndefined)
	{
		++interpreter->cycles;
	}
	switch (type)
	{
	case TokenUndefined:
		if (interpreter->pass == PassRun)
		{
			itp_endProgram(core);
		}
		break;

	case TokenREM:
	case TokenApostrophe:
		++interpreter->pc;
		break;

	case TokenLabel:
		++interpreter->pc;
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
		break;

	case TokenEol:
		interpreter->isSingleLineIf = false;
		++interpreter->pc;
		break;

	case TokenEND:
		switch (itp_getNextTokenType(interpreter))
		{
		case TokenIF:
			return cmd_END_IF(core);

		case TokenSUB:
			return cmd_END_SUB(core);

		default:
			return cmd_END(core);
		}
		break;

	case TokenLET:
	case TokenIdentifier:
	case TokenStringIdentifier:
		return cmd_LET(core);

	case TokenDIM:
		return cmd_DIM(core);

	case TokenPRINT:
		return cmd_PRINT(core);

	case TokenCLS:
		return cmd_CLS(core);

	case TokenINPUT:
		return cmd_INPUT(core);

	case TokenIF:
		return cmd_IF(core, false);

	case TokenELSE:
		return cmd_ELSE(core);

	case TokenFOR:
		return cmd_FOR(core);

	case TokenNEXT:
		return cmd_NEXT(core);

	case TokenGOTO:
		return cmd_GOTO(core);

	case TokenGOSUB:
		return cmd_GOSUB(core);

	case TokenRETURN:
		return cmd_RETURN(core);

	case TokenDATA:
		return cmd_DATA(core);

	case TokenREAD:
		return cmd_READ(core);

	case TokenSKIP:
		return cmd_SKIP(core);

	case TokenRESTORE:
		return cmd_RESTORE(core);

	case TokenPOKE:
	case TokenPOKEW:
	case TokenPOKEL:
		return cmd_POKE(core);

	case TokenFILL:
		return cmd_FILL(core);

	case TokenCOPY:
		return cmd_COPY(core);

	case TokenROL:
	case TokenROR:
		return cmd_ROL_ROR(core);

	case TokenWAIT:
		return cmd_WAIT(core);

	case TokenON:
		return cmd_ON(core);

	case TokenSWAP:
		return cmd_SWAP(core);

	case TokenTEXT:
		return cmd_TEXT(core);

	case TokenNUMBER:
		return cmd_NUMBER(core);

	case TokenDO:
		return cmd_DO(core);

	case TokenLOOP:
		return cmd_LOOP(core);

	case TokenREPEAT:
		return cmd_REPEAT(core);

	case TokenUNTIL:
		return cmd_UNTIL(core);

	case TokenWHILE:
		return cmd_WHILE(core);

	case TokenWEND:
		return cmd_WEND(core);

	case TokenSYSTEM:
		return cmd_SYSTEM(core);

	case TokenCOMPAT:
		return cmd_COMPAT(core);

	case TokenRANDOMIZE:
		return cmd_RANDOMIZE(core);

	case TokenADD:
		return cmd_ADD(core);

	case TokenINC:
	case TokenDEC:
		return cmd_INC_DEC(core);

	case TokenLEFTStr:
	case TokenRIGHTStr:
		return cmd_LEFT_RIGHT(core);

	case TokenMID:
		return cmd_MID(core);

	case TokenWINDOW:
		return cmd_WINDOW(core);

	case TokenFONT:
		return cmd_FONT(core);

	case TokenLOCATE:
		return cmd_LOCATE(core);

	case TokenCLW:
		return cmd_CLW(core);

	case TokenBG:
		switch (itp_getNextTokenType(interpreter))
		{
		case TokenSOURCE:
			return cmd_BG_SOURCE(core);

		case TokenCOPY:
			return cmd_BG_COPY(core);

		case TokenSCROLL:
			return cmd_BG_SCROLL(core);

		case TokenFILL:
			return cmd_BG_FILL(core);

		case TokenTINT:
			return cmd_BG_TINT(core);

		case TokenVIEW:
			return cmd_BG_VIEW(core);

		default:
			return cmd_BG(core);
		}
		break;

	case TokenATTR:
		return cmd_ATTR(core);

	case TokenPAL:
		return cmd_PAL(core);

	case TokenFLIP:
		return cmd_FLIP(core);

	case TokenPRIO:
		return cmd_PRIO(core);

	case TokenCELL:
		// switch (itp_getNextTokenType(interpreter))
		// {
		//     case TokenSIZE:
		//         return cmd_CELL_SIZE(core);

		//     default:
		return cmd_CELL(core);
		// }
		break;

	case TokenTINT:
		return cmd_TINT(core);

	case TokenMCELL:
		return cmd_MCELL(core);

	case TokenPALETTE:
		return cmd_PALETTE(core);

	case TokenSCROLL:
		return cmd_SCROLL(core);

		// case TokenDISPLAY:
		//     return cmd_DISPLAY(core);

	case TokenSPRITEA:
		return cmd_SPRITE_A(core);

	case TokenSPRITE:
		switch (itp_getNextTokenType(interpreter))
		{
		case TokenOFF:
			return cmd_SPRITE_OFF(core);

		case TokenVIEW:
			return cmd_SPRITE_VIEW(core);

		default:
			return cmd_SPRITE(core);
		}
		break;

	case TokenSAVE:
		return cmd_SAVE(core);

	case TokenLOAD:
		return cmd_LOAD(core);

	case TokenFILES:
		return cmd_FILES(core);

		// case TokenGAMEPAD:
		//     return cmd_GAMEPAD(core);

	case TokenKEYBOARD:
		return cmd_KEYBOARD(core);

		// case TokenTOUCHSCREEN:
		//     return cmd_TOUCHSCREEN(core);

	case TokenTRACE:
		return cmd_TRACE(core);

	case TokenCALL:
		return cmd_CALL(core);

	case TokenSUB:
		return cmd_SUB(core);

		//        case TokenSHARED:
		//            return cmd_SHARED(core);

	case TokenGLOBAL:
		return cmd_GLOBAL(core);

	case TokenEXIT:
		switch (itp_getNextTokenType(interpreter))
		{
		case TokenSUB:
			return cmd_EXIT_SUB(core);
		default:
			return cmd_EXIT(core);
		}
		break;

	case TokenPAUSE:
		return cmd_PAUSE(core);

	case TokenSOUND:
		switch (itp_getNextTokenType(interpreter))
		{
			//                case TokenCOPY:
			//                    return cmd_SOUND_COPY(core);

		case TokenSOURCE:
			return cmd_SOUND_SOURCE(core);

		default:
			return cmd_SOUND(core);
		}
		break;

	case TokenVOLUME:
		return cmd_VOLUME(core);

	case TokenENVELOPE:
		return cmd_ENVELOPE(core);

	case TokenLFO:
		switch (itp_getNextTokenType(interpreter))
		{
		case TokenWAVE:
			return cmd_LFO_WAVE(core);

		default:
			return cmd_LFO(core);
		}
		break;

	case TokenLFOA:
		return cmd_LFO_A(core);

	case TokenPLAY:
		return cmd_PLAY(core);

	case TokenSTOP:
		return cmd_STOP(core);

	case TokenMUSIC:
		return cmd_MUSIC(core);

	case TokenTRACK:
		return cmd_TRACK(core);

	case TokenPARTICLE:
		return cmd_PARTICLE(core);

	case TokenEMITTER:
		return cmd_EMITTER(core);

	case TokenMESSAGE:
		return cmd_MESSAGE(core);

	case TokenDMA:
		return cmd_DMA_COPY(core);

	default:
		printf("Command not implemented: %s\n", TokenStrings[interpreter->pc->type]);
		return ErrorSyntax;
	}
	return ErrorNone;
}

enum ErrorCode itp_labelStackError(struct LabelStackItem *item)
{
	switch (item->type)
	{
	case LabelTypeIF:
	case LabelTypeELSEIF:
	case LabelTypeELSE:
		return ErrorIfWithoutEndIf;

	case LabelTypeFOR:
		return ErrorForWithoutNext;

	case LabelTypeDO:
		return ErrorDoWithoutLoop;

	case LabelTypeREPEAT:
		return ErrorRepeatWithoutUntil;

	case LabelTypeWHILE:
		return ErrorWhileWithoutWend;

	case LabelTypeSUB:
		return ErrorSubWithoutEndSub;

	case LabelTypeFORVar:
	case LabelTypeFORLimit:
	case LabelTypeGOSUB:
	case LabelTypeCALL:
	case LabelTypeONCALL:
		// should not happen in compile time
		return ErrorSyntax;
	}
}
//
// Copyright 2017-2018 Timo Kloss
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

#include "core.h"
#include "core.h"

enum ErrorCode itp_evaluateSimpleAttributes(struct Core *core, struct SimpleAttributes *attrs)
{
    struct Interpreter *interpreter = core->interpreter;

    attrs->pal = -1;
    attrs->flipX = -1;
    attrs->flipY = -1;
    attrs->prio = -1;
    attrs->size = -1;

    bool changed = false;
    bool checked = false;

    do
    {
        checked = false;

        // PAL
        if (interpreter->pc->type == TokenPAL && attrs->pal == -1)
        {
            ++interpreter->pc;

            struct TypedValue value = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->pal = value.v.floatValue;

            checked = true;
        }

        // FLIP
        if (interpreter->pc->type == TokenFLIP && attrs->flipX == -1)
        {
            ++interpreter->pc;

            struct TypedValue fxValue = itp_evaluateNumericExpression(core, -1, 1);
            if (fxValue.type == ValueTypeError) return fxValue.v.errorCode;
            attrs->flipX = fxValue.v.floatValue ? 1 : 0;

            // comma
            if (interpreter->pc->type != TokenComma) return ErrorSyntax;
            ++interpreter->pc;

            struct TypedValue fyValue = itp_evaluateNumericExpression(core, -1, 1);
            if (fyValue.type == ValueTypeError) return fyValue.v.errorCode;
            attrs->flipY = fyValue.v.floatValue ? 1 : 0;

            checked = true;
        }

        // PRIO
        if (interpreter->pc->type == TokenPRIO && attrs->prio == -1)
        {
            ++interpreter->pc;

            struct TypedValue value = itp_evaluateNumericExpression(core, -1, 1);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->prio = value.v.floatValue ? 1 : 0;

            checked = true;
        }

        // SIZE
        if (interpreter->pc->type == TokenSIZE && attrs->size == -1)
        {
            ++interpreter->pc;

            struct TypedValue value = itp_evaluateNumericExpression(core, 0, 3);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->size = value.v.floatValue;

            checked = true;
        }

        changed |= checked;
    }
    while (checked);

    if (!changed) return ErrorSyntax;

    return ErrorNone;
}

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;

        // obsolete syntax!

        union CharacterAttributes resultAttr = oldAttr;

        struct TypedValue palValue = {ValueTypeNull, 0};
        struct TypedValue fxValue = {ValueTypeNull, 0};
        struct TypedValue fyValue = {ValueTypeNull, 0};
        struct TypedValue priValue = {ValueTypeNull, 0};
        struct TypedValue sValue = {ValueTypeNull, 0};

        // palette value
        palValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_PALETTES - 1);
        if (palValue.type == ValueTypeError) return palValue;

        // comma
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;

            // flip x value
            fxValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
            if (fxValue.type == ValueTypeError) return fxValue;

            // comma
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;

                // flip y value
                fyValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                if (fyValue.type == ValueTypeError) return fyValue;

                // comma
                if (interpreter->pc->type == TokenComma)
                {
                    ++interpreter->pc;

                    // priority value
                    priValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                    if (priValue.type == ValueTypeError) return priValue;

                    // comma
                    if (interpreter->pc->type == TokenComma)
                    {
                        ++interpreter->pc;

                        // size value
                        sValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
                        if (sValue.type == ValueTypeError) return sValue;
                    }
                }
            }
        }

        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        interpreter->pc++;

        if (interpreter->pass == PassRun)
        {
            if (palValue.type != ValueTypeNull) resultAttr.palette = palValue.v.floatValue;
            if (fxValue.type != ValueTypeNull) resultAttr.flipX = fxValue.v.floatValue;
            if (fyValue.type != ValueTypeNull) resultAttr.flipY = fyValue.v.floatValue;
            if (priValue.type != ValueTypeNull) resultAttr.priority = priValue.v.floatValue;
            if (sValue.type != ValueTypeNull) resultAttr.size = sValue.v.floatValue;
        }

        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

// struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr)
// {
//     struct Interpreter *interpreter = core->interpreter;
//     if (interpreter->pc->type == TokenBracketOpen)
//     {
//         // bracket open
//         interpreter->pc++;

//         union DisplayAttributes resultAttr = oldAttr;

//         struct TypedValue sValue = {ValueTypeNull, 0};
//         struct TypedValue bg0Value = {ValueTypeNull, 0};
//         struct TypedValue bg1Value = {ValueTypeNull, 0};
//         struct TypedValue bg2Value = {ValueTypeNull, 0};
//         struct TypedValue bg3Value = {ValueTypeNull, 0};
//         struct TypedValue bg0SizeValue = {ValueTypeNull, 0};
//         struct TypedValue bg1SizeValue = {ValueTypeNull, 0};
//         struct TypedValue bg2SizeValue = {ValueTypeNull, 0};
//         struct TypedValue bg3SizeValue = {ValueTypeNull, 0};

//         // sprites value
//         sValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
//         if (sValue.type == ValueTypeError) return sValue;

//         // comma
//         if (interpreter->pc->type == TokenComma)
//         {
//             ++interpreter->pc;

//             // bg0 value
//             bg0Value = itp_evaluateOptionalNumericExpression(core, -1, 1);
//             if (bg0Value.type == ValueTypeError) return bg0Value;

//             // comma
//             if (interpreter->pc->type == TokenComma)
//             {
//                 ++interpreter->pc;

//                 // bg1 value
//                 bg1Value = itp_evaluateOptionalNumericExpression(core, -1, 1);
//                 if (bg1Value.type == ValueTypeError) return bg1Value;

//                 // comma
//                 if (interpreter->pc->type == TokenComma)
//                 {
//                     ++interpreter->pc;

//                     // bg0 cell size value
//                     bg0SizeValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
//                     if (bg0SizeValue.type == ValueTypeError) return bg0SizeValue;

//                     // comma
//                     if (interpreter->pc->type == TokenComma)
//                     {
//                         ++interpreter->pc;

//                         // bg1 cell size value
//                         bg1SizeValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
//                         if (bg1SizeValue.type == ValueTypeError) return bg1SizeValue;

//                         // comma
//                         if (interpreter->pc->type == TokenComma)
//                         {
//                             ++interpreter->pc;

//                             // bg2 cell size value
//                             bg1SizeValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
//                             if (bg1SizeValue.type == ValueTypeError) return bg1SizeValue;
//                         }
//                     }
//                 }
//             }
//         }

//         // bracket close
//         if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
//         interpreter->pc++;

//         if (interpreter->pass == PassRun)
//         {
//             if (sValue.type != ValueTypeNull) resultAttr.spritesEnabled = sValue.v.floatValue;
//             if (bg0Value.type != ValueTypeNull) resultAttr.planeAEnabled = bg0Value.v.floatValue;
//             if (bg1Value.type != ValueTypeNull) resultAttr.planeBEnabled = bg1Value.v.floatValue;
//             if (bg0SizeValue.type != ValueTypeNull) resultAttr.planeACellSize = bg0SizeValue.v.floatValue;
//             if (bg1SizeValue.type != ValueTypeNull) resultAttr.planeBCellSize = bg1SizeValue.v.floatValue;
//         }

//         struct TypedValue resultValue;
//         resultValue.type = ValueTypeFloat;
//         resultValue.v.floatValue = resultAttr.value;
//         return resultValue;
//     }
//     else
//     {
//         return itp_evaluateNumericExpression(core, 0, 255);
//     }
// }

struct TypedValue itp_evaluateLFOAttributes(struct Core *core, union LFOAttributes oldAttr)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;

        union LFOAttributes resultAttr = oldAttr;

        struct TypedValue wavValue = {ValueTypeNull, 0};
        struct TypedValue invValue = {ValueTypeNull, 0};
        struct TypedValue envValue = {ValueTypeNull, 0};
        struct TypedValue triValue = {ValueTypeNull, 0};

        // wave value
        wavValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
        if (wavValue.type == ValueTypeError) return wavValue;

        // comma
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;

            // invert value
            invValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
            if (invValue.type == ValueTypeError) return invValue;

            // comma
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;

                // env mode value
                envValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                if (envValue.type == ValueTypeError) return envValue;

                // comma
                if (interpreter->pc->type == TokenComma)
                {
                    ++interpreter->pc;

                    // trigger value
                    triValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                    if (triValue.type == ValueTypeError) return triValue;
                }
            }
        }

        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        interpreter->pc++;

        if (interpreter->pass == PassRun)
        {
            if (wavValue.type != ValueTypeNull) resultAttr.wave = wavValue.v.floatValue;
            if (invValue.type != ValueTypeNull) resultAttr.invert = invValue.v.floatValue;
            if (envValue.type != ValueTypeNull) resultAttr.envMode = envValue.v.floatValue;
            if (triValue.type != ValueTypeNull) resultAttr.trigger = triValue.v.floatValue;
        }

        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

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

#include "core.h"
#include "core.h"

enum ErrorCode lab_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token)
{
    if (interpreter->numLabelStackItems >= MAX_LABEL_STACK_ITEMS) return ErrorStackOverflow;
    struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems];
    item->type = type;
    item->token = token;
    interpreter->numLabelStackItems++;
    return ErrorNone;
}

struct LabelStackItem *lab_popLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        interpreter->numLabelStackItems--;
        return &interpreter->labelStackItems[interpreter->numLabelStackItems];
    }
    return NULL;
}

struct LabelStackItem *lab_peekLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        return &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
    }
    return NULL;
}

struct LabelStackItem *lab_searchLabelStackItem(struct Interpreter *interpreter, enum LabelType types[], int numTypes)
{
    int i = interpreter->numLabelStackItems - 1;
    while (i >= 0)
    {
        struct LabelStackItem *item = &interpreter->labelStackItems[i];
        for (int j = 0; j < numTypes; j++)
        {
            if (item->type == types[j])
            {
                return item;
            }
        }
        --i;
    }
    return NULL;
}
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

#include "core.h"
#include <stdlib.h>
#include <string.h>

struct RCString *rcstring_new(const char *chars, size_t len)
{
    size_t size = sizeof(struct RCString) + len;
    struct RCString *string = malloc(size);
    if (string)
    {
        string->refCount = 1; // retain
        if (chars)
        {
            memcpy(string->chars, chars, len);
        }
        string->chars[len] = 0; // end of string
    }
    return string;
}

void rcstring_retain(struct RCString *string)
{
    string->refCount++;
}

void rcstring_release(struct RCString *string)
{
    string->refCount--;
    if (string->refCount == 0)
    {
        free((void *)string);
    }
}
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

#include "core.h"
#include <stdlib.h>
#include <string.h>

const char *uppercaseString(const char *source)
{
    size_t len = strlen(source);
    char *buffer = malloc(len + 1);
    if (buffer)
    {
        const char *sourceChar = source;
        char *destChar = buffer;
        char finalChar = 0;
        while (*sourceChar)
        {
            finalChar = *sourceChar++;
            if (finalChar >= 'a' && finalChar <= 'z')
            {
                finalChar -= 32;
            }
            *destChar++ = finalChar;
        }
        *destChar = 0;
    }
    return buffer;
}

const char *lineString(const char *source, int pos)
{
    const char *start = &source[pos];
    const char *end = &source[pos];
    while (start - 1 >= source && *(start - 1) != '\n')
    {
        start--;
    }
    while (*(end + 1) != 0 && *end != '\n' && *end != 0)
    {
        end++;
    }
    if (end > start)
    {
        size_t len = end - start;
        char *buffer = malloc(len + 1);
        if (buffer)
        {
            strncpy(buffer, start, len);
            buffer[len] = 0;
            return buffer;
        }
    }
    return NULL;
}

int lineNumber(const char *source, int pos)
{
    int line = 1;
    for (int i = 0; i < pos; i++)
    {
        if (source[i] == '\n')
        {
            line++;
        }
    }
    return line;
}

void stringConvertCopy(char *dest, const char *source, size_t length)
{
    char *currDstChar = dest;
    for (int i = 0; i < length; i++)
    {
        char currSrcChar = source[i];
        if (currSrcChar != '\r')
        {
            *currDstChar = currSrcChar;
            currDstChar++;
        }
    }
    *currDstChar = 0;
}
//
// Copyright 2017-2020 Timo Kloss
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

#include "core.h"

const char *TokenStrings[] = {
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    // Signs
    ":",
    ",",
    ";",
    "'",
    NULL,

    // Operators
    "=",
    ">=",
    "<=",
    "<>",
    ">",
    "<",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "\\",
    "^",
    "AND",
    "NOT",
    "OR",
    "XOR",
    "MOD",

    // Commands/Functions
    "ABS",
    "ACOS",
    "ADD",
    "ASC",
    "ASIN",
    "ATAN",
    "ATTR",
    "BG",
    "BIN$",
    // "BUTTON",
    "CALL",
    "CELL.A",
    "CELL.C",
    "CELL",
    "CHAR",
    "CHR$",
    "CLS",
    "CLW",
    "COLOR",
    "COPY",
    "COS",
    "CURSOR.X",
    "CURSOR.Y",
    "DATA",
    "DEC",
    "DIM",
    // "DISPLAY",
    //"DOWN",
    "DO",
    "ELSE",
    "END",
    "ENVELOPE",
    "EXIT",
    "EXP",
    "FILE$",
    "FILES",
    "FILL",
    "FLIP",
    "FONT",
    "FOR",
    "FSIZE",
    //"GAMEPAD",
    "GLOBAL",
    "GOSUB",
    "GOTO",
    "HEX$",
    "HCOS",
    "HIT",
    "HSIN",
    "HTAN",
    "IF",
    "INC",
    "INKEY$",
    "INPUT",
    "INSTR",
    "INT",
    "KEYBOARD",
    "LEFT$",
    //"LEFT",
    "LEN",
    "LET",
    "LFO.A",
    "LFO",
    "LOAD",
    "LOCATE",
    "LOG",
    "LOOP",
    "MAX",
    "MCELL.A",
    "MCELL.C",
    "MCELL",
    "MID$",
    "MIN",
    "CLAMP",
    "MUSIC",
    "NEXT",
    "NUMBER",
    "OFF",
    "ON",
    "PALETTE",
    "PAL",
    "PAUSE",
    "PEEKL",
    "PEEKW",
    "PEEK",
    "PI",
    "PLAY",
    "POKEL",
    "POKEW",
    "POKE",
    "PRINT",
    "PRIO",
    "RANDOMIZE",
    "RASTER",
    "READ",
    "SKIP",
    "REM",
    "REPEAT",
    "RESTORE",
    "RETURN",
    "RIGHT$",
    //"RIGHT",
    "RND",
    "ROL",
    "ROM",
    "ROR",
    "SAVE",
    "SCROLL.X",
    "SCROLL.Y",
    "SCROLL",
    "SGN",
    "SIN",
    "SIZE",
    "SOUND",
    "SOURCE",
    "SPRITE.A",
    "SPRITE.C",
    "SPRITE.X",
    "SPRITE.Y",
    "SPRITE",
    "SQR",
    "STEP",
    "STOP",
    "STR$",
    "SUB",
    "SWAP",
    "SYSTEM",
    "TAN",
    "TAP",
    "TEXT",
    "THEN",
    "TIMER",
    "TINT",
    //"TOUCHSCREEN",
    "TOUCH.X",
    "TOUCH.Y",
    "TOUCH",
    "TO",
    "TRACE",
    "TRACK",
    "UBOUND",
    "UNTIL",
    //"UP",
    "VAL",
    "VBL",
    "VIEW",
    "VOLUME",
    "WAIT",
    "WAVE",
    "WEND",
    "WHILE",
    "WINDOW",

    "SHOWN.W",
    "SHOWN.H",
    "SAFE.L",
    "SAFE.T",
    "SAFE.R",
    "SAFE.B",

    "PARTICLE",
    "EMITTER",
    "AT",
    "COMPAT",
    "EASE",
    "MESSAGE",
    "DMA",
		"CEIL",

    // Reserved Keywords
    NULL,
    // "ANIM",
    // "CLOSE",
    // "DECLARE",
    // "DEF",
    // "FLASH",
    // "FN",
    // "FUNCTION",
    // "LBOUND",
    // "OPEN",
    // "OUTPUT",
    // "SHARED",
    // "STATIC",
    // "TEMPO",
    // "VOICE",
    // "WRITE",

    NULL
};
//
// Copyright 2016-2017 Timo Kloss
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

#include "core.h"
#include "core.h"
#include "core.h"
#include <string.h>
#include <stdlib.h>
#include "core.h"

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode)
{
    const char *uppercaseSourceCode = uppercaseString(sourceCode);
    if (!uppercaseSourceCode) return err_makeCoreError(ErrorOutOfMemory, -1);

    struct CoreError error = tok_tokenizeUppercaseProgram(tokenizer, uppercaseSourceCode);
    free((void *)uppercaseSourceCode);

    return error;
}

struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode)
{
    const char *character = sourceCode;

		// TODO: I'm not sure I need this
		int allowMultipleStatements = 1;

    // PROGRAM

    while (*character && *character != '#')
    {
        int tokenSourcePosition = (int)(character - sourceCode);
        if (tokenizer->numTokens >= MAX_TOKENS - 1)
        {
            return err_makeCoreError(ErrorTooManyTokens, tokenSourcePosition);
        }
        struct Token *token = &tokenizer->tokens[tokenizer->numTokens];
        token->sourcePosition = tokenSourcePosition;

        // line break \n or \n\r
        if (*character == '\n')
        {
            token->type = TokenEol;
						 allowMultipleStatements = 1;
            tokenizer->numTokens++;
            character++;
            if (*character == '\r') { character++; }
            continue;
        }

        // line break \r or \r\n
        if (*character == '\r')
        {
            token->type = TokenEol;
						allowMultipleStatements = 1;
            tokenizer->numTokens++;
            character++;
            if (*character == '\n') { character++; }
            continue;
        }

        // space
        if (*character == ' ' || *character == '\t')
        {
            character++;
            continue;
        }

				if (*character == ':') // && allowMultipleStatements == 1)
				{
						token->type = TokenEol;
						tokenizer->numTokens++;
						character++;
						continue;
				}

        // string
        if (*character == '"')
        {
            character++;
            const char *firstCharacter = character;
            while (*character && *character != '"')
            {
                if (*character == '\n')
                {
                    return err_makeCoreError(ErrorUnterminatedString, (int)(character - sourceCode));
                }
                else if (*character < 0)
                {
                    return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - sourceCode));
                }
                character++;
            }
            int len = (int)(character - firstCharacter);
            struct RCString *string = rcstring_new(firstCharacter, len);
            if (!string) return err_makeCoreError(ErrorOutOfMemory, tokenSourcePosition);
            token->type = TokenString;
            token->stringValue = string;
            tokenizer->numTokens++;
            character++;
            continue;
        }

        // number
        if (strchr(CharSetDigits, *character))
        {
            float number = 0;
            int afterDot = 0;
            while (*character)
            {
                if (strchr(CharSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    if (afterDot == 0)
                    {
                        number *= 10;
                        number += digit;
                    }
                    else
                    {
                        number += (float)digit / afterDot;
                        afterDot *= 10;
                    }
                    character++;
                }
                else if (*character == '.' && afterDot == 0)
                {
                    afterDot = 10;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            tokenizer->numTokens++;
            continue;
        }

        // hex number
        if (*character == '$')
        {
            character++;
            int number = 0;
            while (*character)
            {
                char *spos = strchr(CharSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - CharSetHex);
                    number <<= 4;
                    number += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            tokenizer->numTokens++;
            continue;
        }

        // bin number
        if (*character == '%')
        {
            character++;
            int number = 0;
            while (*character)
            {
                if (*character == '0' || *character == '1')
                {
                    int digit = *character - '0';
                    number <<= 1;
                    number += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            tokenizer->numTokens++;
            continue;
        }

        // Keyword
        enum TokenType foundKeywordToken = TokenUndefined;
        for (int i = 0; i < Token_count; i++)
        {
            const char *keyword = TokenStrings[i];
            if (keyword)
            {
                size_t keywordLen = strlen(keyword);
                int keywordIsAlphaNum = strchr(CharSetAlphaNum, keyword[0]) != NULL;
                for (int pos = 0; pos <= keywordLen; pos++)
                {
                    char textCharacter = character[pos];

                    if (pos < keywordLen)
                    {
                        char symbCharacter = keyword[pos];
                        if (symbCharacter != textCharacter)
                        {
                            // not matching
                            break;
                        }
                    }
                    else if (keywordIsAlphaNum && textCharacter && strchr(CharSetAlphaNum, textCharacter))
                    {
                        // matching, but word is longer, so seems to be an identifier
                        break;
                    }
                    else
                    {
                        // symbol found!
                        foundKeywordToken = i;
                        character += keywordLen;
                        break;
                    }
                }
                if (foundKeywordToken != TokenUndefined)
                {
                    break;
                }
            }
        }
        if (foundKeywordToken != TokenUndefined)
        {
            if (foundKeywordToken == TokenREM || foundKeywordToken == TokenApostrophe)
            {
                // REM comment, skip until end of line
                while (*character)
                {
                    if (*character < 0)
                    {
                        return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - sourceCode));
                    }
                    if (*character == '\n')
                    {
                        character++;
                        break;
                    }
                    character++;
                }
            }
            else if (foundKeywordToken > Token_reserved)
            {
                return err_makeCoreError(ErrorReservedKeyword, tokenSourcePosition);
            }
            token->type = foundKeywordToken;
            tokenizer->numTokens++;
            continue;
        }

        // Symbol
        if (strchr(CharSetLetters, *character))
        {
            const char *firstCharacter = character;
            char isString = 0;
            while (*character)
            {
                if (strchr(CharSetAlphaNum, *character))
                {
                    character++;
                }
                else
                {
                    if (*character == '$')
                    {
                        isString = 1;
                        character++;
                    }
                    break;
                }
            }
            if (tokenizer->numSymbols >= MAX_SYMBOLS)
            {
                return err_makeCoreError(ErrorTooManySymbols, tokenSourcePosition);
            }
            int len = (int)(character - firstCharacter);
            if (len >= SYMBOL_NAME_SIZE)
            {
                return err_makeCoreError(ErrorSymbolNameTooLong, tokenSourcePosition);
            }
            char symbolName[SYMBOL_NAME_SIZE];
            memcpy(symbolName, firstCharacter, len);
            symbolName[len] = 0;
            int symbolIndex = -1;
            // find existing symbol
            for (int i = 0; i < MAX_SYMBOLS && tokenizer->symbols[i].name[0] != 0; i++)
            {
                if (strcmp(symbolName, tokenizer->symbols[i].name) == 0)
                {
                    symbolIndex = i;
                    break;
                }
            }
            if (symbolIndex == -1)
            {
                // add new symbol
                strcpy(tokenizer->symbols[tokenizer->numSymbols].name, symbolName);
                symbolIndex = tokenizer->numSymbols++;
            }
            if (isString)
            {
                token->type = TokenStringIdentifier;
            }
            else if (*character == ':')
            {
								token->type = TokenLabel;
								allowMultipleStatements = 0;
								character++;
								enum ErrorCode errorCode = tok_setJumpLabel(tokenizer, symbolIndex, token + 1);
								if (errorCode != ErrorNone) return err_makeCoreError(errorCode, tokenSourcePosition);

								token->symbolIndex = symbolIndex;
								tokenizer->numTokens++;

								int tokenSourcePosition = (int)(character - sourceCode);
								if (tokenizer->numTokens >= MAX_TOKENS - 1)
								{
										return err_makeCoreError(ErrorTooManyTokens, tokenSourcePosition);
								}
								token = &tokenizer->tokens[tokenizer->numTokens];
								token->sourcePosition = tokenSourcePosition;
								token->type = TokenEol;
								tokenizer->numTokens++;
								continue;
            }
            else
            {
                token->type = TokenIdentifier;
                if (tokenizer->numTokens > 0 && tokenizer->tokens[tokenizer->numTokens - 1].type == TokenSUB)
                {
                    enum ErrorCode errorCode = tok_setSub(tokenizer, symbolIndex, token + 1);
                    if (errorCode != ErrorNone) return err_makeCoreError(errorCode, tokenSourcePosition);
                }
            }
            token->symbolIndex = symbolIndex;
            tokenizer->numTokens++;
            continue;
        }

        // Unexpected character
        return err_makeCoreError(ErrorUnexpectedCharacter, tokenSourcePosition);
    }

    // add EOL to the end
    struct Token *token = &tokenizer->tokens[tokenizer->numTokens];
    token->sourcePosition = (int)(character - sourceCode);
    token->type = TokenEol;
    tokenizer->numTokens++;

    return err_noCoreError();
}

void tok_freeTokens(struct Tokenizer *tokenizer)
{
    // Free string tokens
    for (int i = 0; i < tokenizer->numTokens; i++)
    {
        struct Token *token = &tokenizer->tokens[i];
        if (token->type == TokenString)
        {
            rcstring_release(token->stringValue);
        }
    }
    memset(tokenizer, 0, sizeof(struct Tokenizer));
}

struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex)
{
    struct JumpLabelItem *item;
    for (int i = 0; i < tokenizer->numJumpLabelItems; i++)
    {
        item = &tokenizer->jumpLabelItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token)
{
    if (tok_getJumpLabel(tokenizer, symbolIndex) != NULL)
    {
        return ErrorLabelAlreadyDefined;
    }
    if (tokenizer->numJumpLabelItems >= MAX_JUMP_LABEL_ITEMS)
    {
        return ErrorTooManyLabels;
    }
    struct JumpLabelItem *item = &tokenizer->jumpLabelItems[tokenizer->numJumpLabelItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    tokenizer->numJumpLabelItems++;
    return ErrorNone;
}

struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex)
{
    struct SubItem *item;
    for (int i = 0; i < tokenizer->numSubItems; i++)
    {
        item = &tokenizer->subItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token)
{
    if (tok_getSub(tokenizer, symbolIndex) != NULL)
    {
        return ErrorSubAlreadyDefined;
    }
    if (tokenizer->numSubItems >= MAX_SUB_ITEMS)
    {
        return ErrorTooManySubprograms;
    }
    struct SubItem *item = &tokenizer->subItems[tokenizer->numSubItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    tokenizer->numSubItems++;
    return ErrorNone;
}
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

#include "core.h"

union Value ValueDummy = {0};

struct TypedValue val_makeError(enum ErrorCode errorCode)
{
    struct TypedValue value;
    value.type = ValueTypeError;
    value.v.errorCode = errorCode;
    return value;
}
//
// Copyright 2017-2020 Timo Kloss
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

#include "core.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>

struct SimpleVariable *var_getSimpleVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel)
{
    struct SimpleVariable *variable = NULL;
    for (int i = interpreter->numSimpleVariables - 1; i >= 0; i--)
    {
        variable = &interpreter->simpleVariables[i];
        if (variable->symbolIndex == symbolIndex && (variable->subLevel == subLevel || variable->subLevel == SUB_LEVEL_GLOBAL))
        {
            // variable found
            return variable;
        }
    }
    return NULL;
}

struct SimpleVariable *var_createSimpleVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, enum ValueType type, union Value *valueReference)
{
    if (interpreter->numSimpleVariables >= MAX_SIMPLE_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    if (subLevel > 127)
    {
        *errorCode = ErrorStackOverflow;
        return NULL;
    }
    struct SimpleVariable *variable = &interpreter->simpleVariables[interpreter->numSimpleVariables];
    interpreter->numSimpleVariables++;
    memset(variable, 0, sizeof(struct SimpleVariable));
    variable->symbolIndex = symbolIndex;
    variable->subLevel = subLevel;
    variable->type = type;
    if (valueReference)
    {
        variable->isReference = -1;
        variable->v.reference = valueReference;
    }
    else
    {
        variable->isReference = 0;
        if (type == ValueTypeString)
        {
            // assign global NullString
            variable->v.stringValue = interpreter->nullString;
            rcstring_retain(variable->v.stringValue);
        }
    }
    return variable;
}

void var_freeSimpleVariables(struct Interpreter *interpreter, int minSubLevel)
{
    for (int i = interpreter->numSimpleVariables - 1; i >= 0; i--)
    {
        struct SimpleVariable *variable = &interpreter->simpleVariables[i];
        if (variable->subLevel < minSubLevel)
        {
            break;
        }
        else
        {
            if (!variable->isReference && variable->type == ValueTypeString)
            {
                rcstring_release(variable->v.stringValue);
            }
            interpreter->numSimpleVariables--;
        }
    }
}

struct ArrayVariable *var_getArrayVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel)
{
    struct ArrayVariable *variable = NULL;
    for (int i = interpreter->numArrayVariables - 1; i >= 0; i--)
    {
        variable = &interpreter->arrayVariables[i];
        if (variable->symbolIndex == symbolIndex && (variable->subLevel == subLevel || variable->subLevel == SUB_LEVEL_GLOBAL))
        {
            // variable found
            return variable;
        }
    }
    return NULL;
}

union Value *var_getArrayValue(struct Interpreter *interpreter, struct ArrayVariable *variable, int *indices)
{
    int offset = 0;
    int factor = 1;
    for (int i = variable->numDimensions - 1; i >= 0; i--)
    {
        offset += indices[i] * factor;
        factor *= variable->dimensionSizes[i];
    }
    union Value *value = &variable->values[offset];
    if (variable->type == ValueTypeString && !value->stringValue)
    {
        // string variable was still uninitialized, assign global NullString
        value->stringValue = interpreter->nullString;
        rcstring_retain(value->stringValue);
    }
    return value;
}

struct ArrayVariable *var_dimVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int numDimensions, int *dimensionSizes)
{
    if (var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel))
    {
        *errorCode = ErrorArrayAlreadyDimensionized;
        return NULL;
    }
    if (var_getSimpleVariable(interpreter, symbolIndex, interpreter->subLevel))
    {
        *errorCode = ErrorVariableAlreadyUsed;
        return NULL;
    }
    if (interpreter->numArrayVariables >= MAX_ARRAY_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    struct ArrayVariable *variable = &interpreter->arrayVariables[interpreter->numArrayVariables];
    interpreter->numArrayVariables++;
    memset(variable, 0, sizeof(struct ArrayVariable));
    variable->symbolIndex = symbolIndex;
    variable->subLevel = interpreter->subLevel;
    variable->isReference = 0;
    variable->numDimensions = numDimensions;
    size_t size = 1;
    for (int i = 0; i < numDimensions; i++)
    {
        size *= dimensionSizes[i];
        variable->dimensionSizes[i] = dimensionSizes[i];
    }
    if (size > MAX_ARRAY_SIZE)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    variable->values = calloc(size, sizeof(union Value));
    if (!variable->values) exit(EXIT_FAILURE);

    variable->numValues = (int)size;
    return variable;
}

struct ArrayVariable *var_createArrayVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, struct ArrayVariable *arrayReference)
{
    if (interpreter->numArrayVariables >= MAX_ARRAY_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    if (subLevel > 127)
    {
        *errorCode = ErrorStackOverflow;
        return NULL;
    }
    struct ArrayVariable *variable = &interpreter->arrayVariables[interpreter->numArrayVariables];
    interpreter->numArrayVariables++;
    memset(variable, 0, sizeof(struct ArrayVariable));
    variable->symbolIndex = symbolIndex;
    variable->subLevel = subLevel;
    variable->isReference = -1;
    variable->type = arrayReference->type;
    int numDimensions = arrayReference->numDimensions;
    variable->numDimensions = numDimensions;
    for (int i = 0; i < numDimensions; i++)
    {
        variable->dimensionSizes[i] = arrayReference->dimensionSizes[i];
    }
    variable->values = arrayReference->values;
    return variable;
}

void var_freeArrayVariables(struct Interpreter *interpreter, int minSubLevel)
{
    for (int i = interpreter->numArrayVariables - 1; i >= 0; i--)
    {
        struct ArrayVariable *variable = &interpreter->arrayVariables[i];
        if (variable->subLevel < minSubLevel)
        {
            break;
        }
        else
        {
            if (!variable->isReference)
            {
                if (variable->type == ValueTypeString)
                {
                    int numElements = 1;
                    for (int di = 0; di < variable->numDimensions; di++)
                    {
                        numElements *= variable->dimensionSizes[di];
                    }
                    for (int ei = 0; ei < numElements; ei++)
                    {
                        union Value *value = &variable->values[ei];
                        if (value->stringValue)
                        {
                            rcstring_release(value->stringValue);
                        }
                    }
                }
                free(variable->values);
                variable->values = NULL;
            }
            interpreter->numArrayVariables--;
        }
    }
}
//
// Copyright 2018 Timo Kloss
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

#include "core.h"
#include "core.h"
#include <math.h>

#define SOUND_SIZE 8
#define PATTERN_SIZE 4
#define ROW_SIZE 3

#define PATTERNS_OFFSET (NUM_SOUNDS * SOUND_SIZE)
#define TRACKS_OFFSET (PATTERNS_OFFSET + NUM_PATTERNS * PATTERN_SIZE)

struct TrackRow {
    int note;
    int sound;
    int volume;
    int command;
    int parameter;
};

void audlib_updateMusic(struct AudioLib *lib);
void audlib_updateTrack(struct AudioLib *lib, int voiceIndex);
void audlib_setPitch(struct Voice *voice, float pitch);
bool audlib_isPatternEmpty(struct AudioLib *lib, int sourceAddress, int pattern);
int audlib_getLoopStart(struct AudioLib *lib, int sourceAddress, int pattern);
int audlib_getLoop(struct AudioLib *lib, int sourceAddress, int pattern, int param);
int audlib_getTrack(struct AudioLib *lib, int sourceAddress, int pattern, int voice);
struct TrackRow audlib_getTrackRow(struct AudioLib *lib, int sourceAddress, int track, int row);
void audlib_playRow(struct AudioLib *lib, struct ComposerPlayer *player, int track, int voice);
void audlib_command(struct AudioLib *lib, struct Voice *voice, struct ComposerPlayer *player, int command, int parameter);


void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound)
{
    struct Core *core = lib->core;
    struct Voice *voice = &core->machine->audioRegisters.voices[voiceIndex];

    audlib_setPitch(voice, pitch);

    if (sound != -1)
    {
        audlib_copySound(lib, lib->sourceAddress, sound, voiceIndex);
    }

    if (len != -1)
    {
        voice->length = len;
        voice->attr.timeout = (len > 0) ? 1 : 0;
    }
    voice->status.init = 1;
    voice->status.gate = 1;

    machine_enableAudio(core);
}

void audlib_copySound(struct AudioLib *lib, int sourceAddress, int sound, int voiceIndex)
{
    int addr = sourceAddress + sound * 8;
    int dest = 0xFF40 + voiceIndex * sizeof(struct Voice) + 4;
    for (int i = 0; i < 8; i++)
    {
        int peek = machine_peek(lib->core, addr++);
        machine_poke(lib->core, dest++, peek);
    }
    lib->core->interpreter->cycles += 8;
}

void audlib_playMusic(struct AudioLib *lib, int startPattern)
{
    struct ComposerPlayer *player = &lib->musicPlayer;
    player->sourceAddress = lib->sourceAddress;
    player->index = startPattern;
    player->tick = -1;
    player->row = 0;
    player->speed = 8;
    player->willBreak = false;

    machine_enableAudio(lib->core);
}

void audlib_playTrack(struct AudioLib *lib, int track, int voiceIndex)
{
    struct ComposerPlayer *player = &lib->trackPlayers[voiceIndex];
    player->sourceAddress = lib->sourceAddress;
    player->index = track;
    player->tick = -1;
    player->row = 0;
    player->speed = 8;
    player->willBreak = false;

    machine_enableAudio(lib->core);
}

void audlib_stopAll(struct AudioLib *lib)
{
    lib->musicPlayer.speed = 0;
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct Voice *voice = &lib->core->machine->audioRegisters.voices[i];
        voice->status.gate = 0;
        lib->trackPlayers[i].speed = 0;
    }
}

void audlib_stopVoice(struct AudioLib *lib, int voiceIndex)
{
    struct Voice *voice = &lib->core->machine->audioRegisters.voices[voiceIndex];
    voice->status.gate = 0;
    lib->trackPlayers[voiceIndex].speed = 0;
}

void audlib_update(struct AudioLib *lib)
{
    if (lib->musicPlayer.speed)
    {
        audlib_updateMusic(lib);
    }
    for (int v = 0; v < NUM_VOICES; v++)
    {
        if (lib->trackPlayers[v].speed)
        {
            audlib_updateTrack(lib, v);
        }
    }

}


void audlib_updateMusic(struct AudioLib *lib)
{
    struct ComposerPlayer *player = &lib->musicPlayer;
    if (player->tick == -1)
    {
        player->tick = 0;
    }
    else if (player->tick == 0)
    {
        if (player->willBreak)
        {
            player->row = 0;
            player->willBreak = false;
        }
        else
        {
            player->row = (player->row + 1) % NUM_TRACK_ROWS;
        }
        if (player->row == 0 && player->speed)
        {
            if (audlib_getLoop(lib, player->sourceAddress, player->index, 2) == 1)
            {
                player->speed = 0;
                return;
            }
            if (audlib_getLoop(lib, player->sourceAddress, player->index, 1) == 1)
            {
                player->index = audlib_getLoopStart(lib, player->sourceAddress, player->index);
            }
            else
            {
                int p = player->index + 1;
                if (p < NUM_PATTERNS)
                {
                    if (audlib_isPatternEmpty(lib, player->sourceAddress, p))
                    {
                        player->speed = 0;
                        return;
                    }
                    else
                    {
                        player->index = p;
                    }
                }
                else
                {
                    player->speed = 0;
                    return;
                }
            }
        }
    }
    if (player->tick == 0)
    {
        for (int v = 0; v < NUM_VOICES; v++)
        {
            // play only if no other track is playing on that voice
            if (lib->trackPlayers[v].speed == 0)
            {
                int track = audlib_getTrack(lib, player->sourceAddress, player->index, v);
                if (track >= 0)
                {
                    audlib_playRow(lib, player, track, v);
                }
            }
        }
        if (player->speed == 0)
        {
            return;
        }
    }
    player->tick = (player->tick + 1) % player->speed;
}

void audlib_updateTrack(struct AudioLib *lib, int voiceIndex)
{
    struct ComposerPlayer *player = &lib->trackPlayers[voiceIndex];
    if (player->tick == -1)
    {
        player->tick = 0;
    }
    else if (player->tick == 0)
    {
        player->row = (player->row + 1) % NUM_TRACK_ROWS;
        if (player->row == 0 || player->willBreak)
        {
            player->willBreak = false;
            player->speed = 0;
            return;
        }
    }
    if (player->tick == 0)
    {
        audlib_playRow(lib, player, player->index, voiceIndex);
        if (player->speed == 0)
        {
            return;
        }
    }
    player->tick = (player->tick + 1) % player->speed;
}

void audlib_setPitch(struct Voice *voice, float pitch)
{
    int f = 16.0 * 440.0 * pow(2.0, (pitch - 58.0) / 12.0);
    voice->frequencyLow = f & 0xFF;
    voice->frequencyHigh = f >> 8;
}

bool audlib_isPatternEmpty(struct AudioLib *lib, int sourceAddress, int pattern)
{
    for (int v = 0; v < NUM_VOICES; v++)
    {
        if (audlib_getTrack(lib, sourceAddress, pattern, v) >= 0)
        {
            return false;
        }
    }
    return true;
}

int audlib_getLoopStart(struct AudioLib *lib, int sourceAddress, int pattern)
{
    for (int p = pattern; p >= 0; p--)
    {
        if (audlib_getLoop(lib, sourceAddress, p, 0) == 1)
        {
            return p;
        }
    }
    return 0;
}

int audlib_getLoop(struct AudioLib *lib, int sourceAddress, int pattern, int param)
{
    int a = sourceAddress + PATTERNS_OFFSET + pattern * PATTERN_SIZE + param;
    return machine_peek(lib->core, a) >> 7;
}

int audlib_getTrack(struct AudioLib *lib, int sourceAddress, int pattern, int voice)
{
    int a = sourceAddress + PATTERNS_OFFSET + pattern * PATTERN_SIZE + voice;
    int track = machine_peek(lib->core, a) & 0x7F;
    if (track == 0x40)
    {
        track = -1;
    }
    return track;
}

struct TrackRow audlib_getTrackRow(struct AudioLib *lib, int sourceAddress, int track, int row)
{
    struct TrackRow trackRow;
    int a = sourceAddress + TRACKS_OFFSET + track * NUM_TRACK_ROWS * ROW_SIZE + row * ROW_SIZE;
    struct Core *core = lib->core;
    trackRow.note = machine_peek(core, a);
    int peek1 = machine_peek(core, a + 1);
    trackRow.sound = peek1 >> 4;
    trackRow.volume = peek1 & 0x0F;
    int peek2 = machine_peek(core, a + 2);
    trackRow.command = peek2 >> 4;
    trackRow.parameter = peek2 & 0x0F;
    return trackRow;
}

void audlib_playRow(struct AudioLib *lib, struct ComposerPlayer *player, int track, int voiceIndex)
{
    struct Core *core = lib->core;
    struct Voice *voice = &core->machine->audioRegisters.voices[voiceIndex];

    struct TrackRow trackRow = audlib_getTrackRow(lib, player->sourceAddress, track, player->row);
    if (trackRow.note > 0 && trackRow.note < 255)
    {
        audlib_copySound(lib, player->sourceAddress, trackRow.sound, voiceIndex);
    }
    if (trackRow.volume > 0)
    {
        voice->status.volume = trackRow.volume;
    }
    audlib_command(lib, voice, player, trackRow.command, trackRow.parameter);
    if (trackRow.note == 255)
    {
        voice->status.gate = 0;
    }
    else if (trackRow.note > 0)
    {
        audlib_setPitch(voice, trackRow.note);
        voice->status.init = 1;
        voice->status.gate = 1;
    }
}

void audlib_command(struct AudioLib *lib, struct Voice *voice, struct ComposerPlayer *player, int command, int parameter)
{
    if (command == 0 && parameter == 0) return;
    switch (command)
    {
        case 0x00:
            voice->status.mix = parameter & 0x03;
            break;
        case 0x01:
            voice->envA = parameter;
            break;
        case 0x02:
            voice->envD = parameter;
            break;
        case 0x03:
            voice->envS = parameter;
            break;
        case 0x04:
            voice->envR = parameter;
            break;
        case 0x05:
            voice->lfoFrequency = parameter;
            break;
        case 0x06:
            voice->lfoOscAmount = parameter;
            break;
        case 0x07:
            voice->lfoVolAmount = parameter;
            break;
        case 0x08:
            voice->lfoPWAmount = parameter;
            break;
        case 0x09:
            voice->attr.pulseWidth = parameter;
            break;
        case 0x0D:
            player->speed = 0x10 | parameter;
            break;
        case 0x0E:
            player->speed = parameter;
            break;
        case 0x0F:
            switch (parameter)
            {
                case 0:
                    player->willBreak = true;
                    break;
                case 1:
                    voice->status.gate = 0;
                    voice->status.volume = 0;
                    break;
            }
            break;
    }
}
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

#include "core.h"

uint8_t DefaultCharacters[][16] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x18, 0x14, 0x04, 0x04, 0x0C, 0x10, 0x0C, 0x00, 0x00, 0x0C, 0x1C, 0x1C, 0x0C, 0x08, 0x0C},
    {0x00, 0x48, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x24, 0x7E, 0x36, 0x12, 0x00, 0x00, 0x00},
    {0x00, 0x24, 0x60, 0x1B, 0x12, 0x40, 0x1B, 0x12, 0x00, 0x00, 0x1E, 0x3F, 0x36, 0x3E, 0x3F, 0x12},
    {0x00, 0x08, 0x30, 0x27, 0x10, 0x21, 0x17, 0x04, 0x00, 0x00, 0x0E, 0x1F, 0x1E, 0x1F, 0x1F, 0x04},
    {0x00, 0x40, 0x11, 0x32, 0x04, 0x0C, 0x11, 0x23, 0x00, 0x22, 0x75, 0x3A, 0x14, 0x2A, 0x57, 0x23},
    {0x00, 0x10, 0x2A, 0x02, 0x10, 0x13, 0x00, 0x1D, 0x00, 0x0C, 0x1E, 0x3A, 0x7E, 0x77, 0x3A, 0x1D},
    {0x00, 0x10, 0x04, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x3C, 0x18, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x16, 0x0C, 0x08, 0x00, 0x00, 0x06, 0x00, 0x04, 0x0E, 0x3C, 0x38, 0x18, 0x0C, 0x06},
    {0x00, 0x30, 0x00, 0x00, 0x02, 0x06, 0x0C, 0x18, 0x00, 0x00, 0x18, 0x0C, 0x0E, 0x1E, 0x3C, 0x18},
    {0x00, 0x00, 0x20, 0x12, 0x40, 0x27, 0x08, 0x12, 0x00, 0x00, 0x04, 0x0A, 0x3E, 0x3F, 0x2C, 0x12},
    {0x00, 0x00, 0x18, 0x14, 0x40, 0x27, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x0C, 0x3E, 0x3F, 0x1C, 0x0C},
    {0x00, 0x00, 0x00, 0x00, 0x10, 0x04, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x3C, 0x18},
    {0x00, 0x00, 0x00, 0x00, 0x60, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x3F, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x0C},
    {0x00, 0x04, 0x0B, 0x06, 0x0C, 0x18, 0x30, 0x20, 0x00, 0x02, 0x07, 0x1E, 0x3C, 0x78, 0x70, 0x20},
    {0x00, 0x20, 0x58, 0x11, 0x01, 0x19, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x7F, 0x77, 0x7F, 0x3F, 0x1E},
    {0x00, 0x10, 0x24, 0x04, 0x04, 0x04, 0x40, 0x3F, 0x00, 0x08, 0x1C, 0x1C, 0x1C, 0x1C, 0x3E, 0x3F},
    {0x00, 0x20, 0x58, 0x33, 0x06, 0x0C, 0x00, 0x3F, 0x00, 0x1C, 0x3E, 0x3F, 0x1E, 0x3C, 0x7E, 0x3F},
    {0x00, 0x20, 0x58, 0x33, 0x00, 0x41, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x3F, 0x06, 0x27, 0x3F, 0x1E},
    {0x00, 0x66, 0x55, 0x01, 0x39, 0x01, 0x01, 0x03, 0x00, 0x00, 0x33, 0x7F, 0x3F, 0x07, 0x07, 0x03},
    {0x00, 0x60, 0x5F, 0x00, 0x38, 0x01, 0x43, 0x3E, 0x00, 0x1E, 0x3F, 0x7C, 0x3E, 0x07, 0x3F, 0x3E},
    {0x00, 0x10, 0x2E, 0x00, 0x18, 0x11, 0x03, 0x1E, 0x00, 0x0C, 0x1E, 0x7C, 0x7E, 0x77, 0x3F, 0x1E},
    {0x00, 0x60, 0x39, 0x03, 0x06, 0x0C, 0x08, 0x18, 0x00, 0x1E, 0x3F, 0x0F, 0x1E, 0x3C, 0x38, 0x18},
    {0x00, 0x20, 0x58, 0x03, 0x18, 0x11, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x3F, 0x7E, 0x77, 0x3F, 0x1E},
    {0x00, 0x20, 0x58, 0x01, 0x19, 0x41, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x3F, 0x1F, 0x27, 0x3F, 0x1E},
    {0x00, 0x00, 0x00, 0x10, 0x0C, 0x10, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0C, 0x08, 0x0C, 0x00},
    {0x00, 0x00, 0x00, 0x10, 0x0C, 0x10, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x08, 0x0C, 0x08, 0x3C, 0x18},
    {0x00, 0x00, 0x08, 0x16, 0x0C, 0x00, 0x00, 0x06, 0x00, 0x00, 0x04, 0x0E, 0x3C, 0x18, 0x0C, 0x06},
    {0x00, 0x00, 0x00, 0x60, 0x3F, 0x40, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x3F, 0x3E, 0x3F, 0x00},
    {0x00, 0x00, 0x30, 0x00, 0x00, 0x06, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x18, 0x0C, 0x1E, 0x3C, 0x18},
    {0x00, 0x20, 0x58, 0x33, 0x06, 0x0C, 0x10, 0x0C, 0x00, 0x1C, 0x3E, 0x3F, 0x1E, 0x0C, 0x08, 0x0C},
    {0x00, 0x20, 0x58, 0x19, 0x11, 0x17, 0x00, 0x1E, 0x00, 0x1C, 0x3E, 0x77, 0x7F, 0x77, 0x3C, 0x1E},
    {0x00, 0x10, 0x20, 0x18, 0x01, 0x19, 0x11, 0x33, 0x00, 0x08, 0x1C, 0x7E, 0x7F, 0x7F, 0x77, 0x33},
    {0x00, 0x60, 0x58, 0x03, 0x18, 0x11, 0x03, 0x3E, 0x00, 0x1C, 0x3E, 0x7F, 0x7E, 0x77, 0x7F, 0x3E},
    {0x00, 0x20, 0x58, 0x13, 0x10, 0x10, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x73, 0x70, 0x76, 0x3F, 0x1E},
    {0x00, 0x60, 0x50, 0x10, 0x11, 0x13, 0x06, 0x3C, 0x00, 0x18, 0x3C, 0x76, 0x77, 0x7F, 0x7E, 0x3C},
    {0x00, 0x60, 0x5F, 0x00, 0x1C, 0x10, 0x00, 0x3F, 0x00, 0x1E, 0x3F, 0x78, 0x7C, 0x70, 0x7E, 0x3F},
    {0x00, 0x60, 0x5F, 0x00, 0x1C, 0x10, 0x10, 0x30, 0x00, 0x1E, 0x3F, 0x78, 0x7C, 0x70, 0x70, 0x30},
    {0x00, 0x20, 0x5E, 0x1C, 0x11, 0x11, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x72, 0x77, 0x77, 0x3F, 0x1E},
    {0x00, 0x66, 0x55, 0x01, 0x19, 0x11, 0x11, 0x33, 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x77, 0x77, 0x33},
    {0x00, 0x30, 0x06, 0x04, 0x04, 0x04, 0x00, 0x1E, 0x00, 0x0C, 0x1E, 0x1C, 0x1C, 0x1C, 0x3C, 0x1E},
    {0x00, 0x1C, 0x09, 0x01, 0x01, 0x41, 0x03, 0x1E, 0x00, 0x02, 0x0F, 0x07, 0x07, 0x27, 0x3F, 0x1E},
    {0x00, 0x64, 0x5B, 0x06, 0x04, 0x10, 0x10, 0x33, 0x00, 0x02, 0x37, 0x7E, 0x7C, 0x7C, 0x76, 0x33},
    {0x00, 0x60, 0x50, 0x10, 0x10, 0x10, 0x00, 0x3F, 0x00, 0x00, 0x30, 0x70, 0x70, 0x70, 0x7E, 0x3F},
    {0x00, 0x42, 0x45, 0x09, 0x01, 0x19, 0x11, 0x33, 0x00, 0x00, 0x23, 0x77, 0x7F, 0x7F, 0x77, 0x33},
    {0x00, 0x66, 0x45, 0x01, 0x11, 0x11, 0x11, 0x33, 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x77, 0x77, 0x33},
    {0x00, 0x20, 0x58, 0x11, 0x11, 0x11, 0x03, 0x1E, 0x00, 0x1C, 0x3E, 0x77, 0x77, 0x77, 0x3F, 0x1E},
    {0x00, 0x60, 0x58, 0x03, 0x1E, 0x10, 0x10, 0x30, 0x00, 0x1C, 0x3E, 0x7F, 0x7E, 0x70, 0x70, 0x30},
    {0x00, 0x20, 0x58, 0x11, 0x11, 0x11, 0x00, 0x1F, 0x00, 0x1C, 0x3E, 0x77, 0x7B, 0x7D, 0x3E, 0x1F},
    {0x00, 0x60, 0x58, 0x03, 0x06, 0x10, 0x10, 0x33, 0x00, 0x1C, 0x3E, 0x7F, 0x7E, 0x7C, 0x76, 0x33},
    {0x00, 0x20, 0x5F, 0x00, 0x18, 0x01, 0x03, 0x3E, 0x00, 0x1E, 0x3F, 0x3C, 0x1E, 0x07, 0x7F, 0x3E},
    {0x00, 0x70, 0x27, 0x04, 0x04, 0x04, 0x04, 0x0C, 0x00, 0x0E, 0x3F, 0x1C, 0x1C, 0x1C, 0x1C, 0x0C},
    {0x00, 0x66, 0x55, 0x11, 0x11, 0x11, 0x03, 0x1E, 0x00, 0x00, 0x33, 0x77, 0x77, 0x77, 0x3F, 0x1E},
    {0x00, 0x66, 0x55, 0x11, 0x11, 0x03, 0x06, 0x0C, 0x00, 0x00, 0x33, 0x77, 0x77, 0x3F, 0x1E, 0x0C},
    {0x00, 0x66, 0x55, 0x15, 0x01, 0x19, 0x31, 0x21, 0x00, 0x00, 0x33, 0x6B, 0x7F, 0x7F, 0x73, 0x21},
    {0x00, 0x64, 0x0B, 0x06, 0x00, 0x18, 0x11, 0x33, 0x00, 0x02, 0x37, 0x1E, 0x3C, 0x7E, 0x77, 0x33},
    {0x00, 0x66, 0x55, 0x0B, 0x06, 0x04, 0x04, 0x0C, 0x00, 0x00, 0x33, 0x37, 0x1E, 0x1C, 0x1C, 0x0C},
    {0x00, 0x60, 0x33, 0x06, 0x0C, 0x58, 0x00, 0x3F, 0x00, 0x1E, 0x3F, 0x1E, 0x3C, 0x38, 0x7E, 0x3F},
    {0x00, 0x30, 0x2E, 0x08, 0x08, 0x08, 0x00, 0x1E, 0x00, 0x0C, 0x1E, 0x38, 0x38, 0x38, 0x3C, 0x1E},
    {0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01},
    {0x00, 0x38, 0x12, 0x02, 0x02, 0x02, 0x02, 0x1E, 0x00, 0x04, 0x1E, 0x0E, 0x0E, 0x0E, 0x3E, 0x1E},
    {0x00, 0x10, 0x20, 0x18, 0x33, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x7E, 0x33, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x3F},
};
#include "core.h"

#include "core.h"
#include "core.h"
#include "core.h"

#define _USE_MATH_DEFINES
#ifndef __USE_MISC
  #define __USE_MISC
#endif
#include <math.h>

static pcg32_random_t pcg;

void prtclib_setupPool(struct ParticlesLib *lib,int firstSprite,int poolCount,int particleAddr)
{
    lib->first_sprite_id=firstSprite;
    lib->pool_count=poolCount;
    lib->pool_next_id=0;
    lib->particles_data_addr=particleAddr;

    for(int particle_id=0;particle_id<lib->pool_count;++particle_id)
    {
			int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

      machine_poke(lib->core, particle+EMITTER_MEM_DELAY, -1);
    }

    pcg32_srandom_r(&pcg, 20321911116532, (intptr_t)&pcg);
}

void prtclib_setApperanceLabel(struct ParticlesLib *lib,int apperanceId,struct Token *label)
{
    lib->apperances_label[apperanceId]=label;
}

void prtclib_setupEmitter(struct ParticlesLib *lib,int emitterCount,int emitterAddr)
{
    lib->emitters_count=emitterCount;
    lib->emitters_data_addr=emitterAddr;
}

void prtclib_setSpawnerLabel(struct ParticlesLib *lib,int emitterId,struct Token *label)
{
    lib->emitters_label[emitterId]=label;
}

void prtclib_spawn(struct ParticlesLib *lib,int emitterId,float posX,float posY)
{
    if(emitterId<0 && emitterId>=lib->emitters_count) return;

    int emitter = lib->emitters_data_addr + emitterId*EMITTER_MEM_SIZE; // 6 bytes

    machine_poke_short(lib->core, emitter+EMITTER_MEM_X, ((int16_t)posX*16)&0x1FFF);
    machine_poke_short(lib->core, emitter+EMITTER_MEM_Y, ((int16_t)posY*16)&0x1FFF);

    machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, 0); // start with no delay

    uint8_t repeat = 1 + dat_readU8(lib->emitters_label[emitterId],EMITTER_DATA_REPEAT,0);
    machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, repeat);
}

void prtclib_stop(struct ParticlesLib *lib,int emitterId)
{
    if(emitterId<0 && emitterId>=lib->emitters_count) return;

    int emitter = lib->emitters_data_addr + emitterId*EMITTER_MEM_SIZE; // 6 bytes

    machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, 0);
}

void prtclib_update(struct Core *core, struct ParticlesLib *lib)
{
    struct Interpreter *interpreter = core->interpreter;

    struct Token *dataToken;

    // update emitters
    for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
        int emitter = lib->emitters_data_addr + emitter_id*EMITTER_MEM_SIZE; // 6 bytes

        float emitter_pos_x=(float)machine_peek_short(lib->core, emitter+EMITTER_MEM_X)/16;
        float emitter_pos_y=(float)machine_peek_short(lib->core, emitter+EMITTER_MEM_Y)/16;

        // wait for delay to end
        int delay=machine_peek(lib->core, emitter+EMITTER_MEM_DELAY);
        if(delay>0)
        {
          machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, delay-1);
          continue;
        }

        // is there more to repeat?
        int repeat=machine_peek(lib->core, emitter+EMITTER_MEM_REPEAT);
        if(repeat>0)
        {
          // emitter data
          uint8_t apperance_id = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_APPEARANCE,255);
          if (apperance_id>APPEARANCE_MAX) continue;

					float shape = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SHAPE,1);
					float outer = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_OUTER,0);
          float inner = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_INNER,0);
					float arc = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_ARC,0);
					float rotation = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_ROTATION,0);
          float speed_x = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SPEED_X,0);
          float speed_y = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SPEED_Y,0);
					float gravity = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_GRAVITY,0);
          uint8_t count = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_COUNT,0);
          uint8_t delay = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_DELAY,0);

          // reduce repeat
          machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, repeat-1);

          // reset delay
          machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, delay);

          for(int i=0; i<count; ++i)
          {
            // spawn a particle
            int particle_id = lib->pool_next_id;
            lib->pool_next_id = (lib->pool_next_id+1) % lib->pool_count;

            int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

            int sprite_id=lib->first_sprite_id+particle_id;
            struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

						float sprite_distance_x;
						float sprite_distance_y;
						int sign;

            // inner, outer ring
            if(outer>0 && outer>inner && shape!=0)
            {
							if(shape>0)
							{
									float angle=(float)ldexp(pcg32_random_r(&pcg),-32);
									float r=(float)sqrt(ldexp(pcg32_random_r(&pcg),-32));
									float diff=outer-inner;
									float space_x=r*(diff)+inner;
									float space_y=r*(diff)+inner;
									sprite_distance_x=cosf(angle*M_PI*2)*space_x;
									sprite_distance_y=sinf(angle*M_PI*2)*space_y*shape;
							}
							else if(inner==0)
							{
									sign=pcg32_boundedrand_r(&pcg,2)*2-1;
									sprite_distance_x=(float)pcg32_boundedrand_r(&pcg,outer)*sign;
									sign=pcg32_boundedrand_r(&pcg,2)*2-1;
									sprite_distance_y=(float)pcg32_boundedrand_r(&pcg,outer)*sign*-shape;
							}
							else
							{
									float x,y;
									int sign;
									float spectral=outer+outer*-shape;
									double choose=ldexp(pcg32_random_r(&pcg),-32)*spectral;
									if(choose<outer)
									{
										// horizontal
										sign=pcg32_boundedrand_r(&pcg,2)*2-1;
										sprite_distance_x=(float)pcg32_boundedrand_r(&pcg,outer*2)-outer;
										sprite_distance_y=((float)pcg32_boundedrand_r(&pcg,outer-inner)+inner)*sign*-shape;
									}
									else
									{
										// vertical
										sign=pcg32_boundedrand_r(&pcg,2)*2-1;
										sprite_distance_x=((float)pcg32_boundedrand_r(&pcg,outer-inner)+inner)*sign;
										sprite_distance_y=((float)pcg32_boundedrand_r(&pcg,outer*2)-outer)*-shape;
									}
							}
            }
						else
						{
							sprite_distance_x=0;
							sprite_distance_y=0;
						}

						if(gravity>0)
						{
							int a=0;
						}

						float vector_len = sqrtf(sprite_distance_x*sprite_distance_x+sprite_distance_y*sprite_distance_y);
						float sprite_norm_x=sprite_distance_x/vector_len;
						float sprite_norm_y=sprite_distance_y/vector_len;

						// position
						spr->x=(int)((emitter_pos_x + sprite_distance_x + SPRITE_OFFSET_X)*16) & 0x1FFF;
						spr->y=(int)((emitter_pos_y + sprite_distance_y + SPRITE_OFFSET_Y)*16) & 0x1FFF;

            // speed x,y
            machine_poke_short(lib->core, particle+PARTICLE_MEM_X, speed_x + sprite_norm_x*gravity);
            machine_poke_short(lib->core, particle+PARTICLE_MEM_Y, speed_y + sprite_norm_y*gravity);

            // apperance
            machine_poke(lib->core, particle+PARTICLE_MEM_APPEARANCE, apperance_id);
            machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, 0);
          }
        }
    }

    // update particles
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+PARTICLE_MEM_APPEARANCE);
        if(apperance_id>APPEARANCE_MAX) continue;

        // sprite
        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        // position x
        float speed_x=(float)machine_peek_short(lib->core, particle+PARTICLE_MEM_X);
        spr->x=(int)(spr->x+speed_x)&0x1FFF;

        // position y
        float speed_y=(float)machine_peek_short(lib->core, particle+PARTICLE_MEM_Y);
        spr->y=(int)(spr->y+speed_y)&0x1FFF;

        // character

        int step_id=machine_peek(lib->core, particle+PARTICLE_MEM_FRAME);

        float character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
        if(character>=0)
        {
          spr->character=(uint8_t)character;
          machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, (step_id+1)&0x1FFF);
        }
        else
        {
          step_id=step_id+(int)character;
          character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
          if(character>=0)
          {
            spr->character=(uint8_t)character;
            machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, (step_id+1)&0x1FFF);
          }
        }
    }
}

void prtclib_interrupt(struct Core *core,struct ParticlesLib *lib)
{
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+PARTICLE_MEM_APPEARANCE);
        if(apperance_id>APPEARANCE_MAX) continue;

        lib->interrupt_sprite_id = lib->first_sprite_id+particle_id;
        lib->interrupt_particle_addr = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE;

        itp_runInterrupt(core, InterruptTypeParticle);
    }

		for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
				int emitter = lib->emitters_data_addr + emitter_id*EMITTER_MEM_SIZE; // 6 bytes

				int repeat=machine_peek(lib->core, emitter+EMITTER_MEM_REPEAT);
				if(repeat>0)
				{
						uint8_t count = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_COUNT,0);
						if(count>0)
						{
							lib->interrupt_emitter_id = emitter_id;
							lib->interrupt_emitter_addr = emitter;
							itp_runInterrupt(core, InterruptTypeEmitter);
						}
				}
		}
}


void prtclib_clear(struct Core *core,struct ParticlesLib *lib)
{
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes
        machine_poke(lib->core, particle+PARTICLE_MEM_APPEARANCE, 255);

        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        spr->x = 0;
        spr->y = 0;
    }
}
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

#include "core.h"
#include "core.h"
#include <stdint.h>

bool sprlib_isSpriteOnScreen(struct Sprite *sprite)
{
    int size = (sprite->attr.size + 1) << 3;
    return (   (sprite->x/16) < SCREEN_WIDTH + SPRITE_OFFSET_X
            && (sprite->y/16) < SCREEN_HEIGHT + SPRITE_OFFSET_Y
            && (sprite->x/16) + size > SPRITE_OFFSET_X
            && (sprite->y/16) + size > SPRITE_OFFSET_Y);
}

bool sprlib_checkSingleCollision(struct SpritesLib *lib, struct Sprite *sprite, struct Sprite *otherSprite)
{
    if (sprlib_isSpriteOnScreen(otherSprite))
    {
        int ax1 = (sprite->x/16);
        int ay1 = (sprite->y/16);

        int ax2 = (otherSprite->x/16);
        int ay2 = (otherSprite->y/16);

        int s1 = (sprite->attr.size + 1) << 3;
        int s2 = (otherSprite->attr.size + 1) << 3;

        int bx1 = ax1 + s1;
        int by1 = ay1 + s1;
        int bx2 = ax2 + s2;
        int by2 = ay2 + s2;

        // rectangle check
        if (bx1 > ax2 && by1 > ay2 && ax1 < bx2 && ay1 < by2)
        {
            // pixel exact check
            int diffX = ax2 - ax1;
            int diffY = ay2 - ay1;

            struct Character *characters = lib->core->machine->videoRam.characters;
            int c1 = sprite->character;
            int c2 = otherSprite->character;

            for (int line = 0; line < s1; line++)
            {
                if (line - diffY >= 0 && line - diffY < s2)
                {
                    int line1 = sprite->attr.flipY ? (s1 - line - 1) : line;
                    int line2 = otherSprite->attr.flipY ? (s2 - (line - diffY) - 1) : (line - diffY);
                    bool flx1 = sprite->attr.flipX;
                    bool flx2 = otherSprite->attr.flipX;

                    uint32_t source1 = 0;
                    int chLine1 = line1 & 7;
                    int rc1 = c1 + (line1 >> 3 << 4);
                    for (int i = 0; i <= sprite->attr.size; i++)
                    {
                        uint8_t *data = characters[flx1 ? (rc1 + sprite->attr.size - i) : (rc1 + i)].data;
                        uint32_t val = (data[chLine1] | data[chLine1 + 8]);
                        if (flx1)
                        {
                            // reverse bits
                            val = (((val * 0x0802LU & 0x22110LU) | (val * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
                        }
                        source1 |= val << (24 - (i << 3));
                    }

                    uint32_t source2 = 0;
                    int chLine2 = line2 & 7;
                    int rc2 = c2 + (line2 >> 3 << 4);
                    for (int i = 0; i <= otherSprite->attr.size; i++)
                    {
                        uint8_t *data = characters[flx2 ? (rc2 + otherSprite->attr.size - i) : (rc2 + i)].data;
                        uint32_t val = (data[chLine2] | data[chLine2 + 8]);
                        if (flx2)
                        {
                            // reverse bits
                            val = (((val * 0x0802LU & 0x22110LU) | (val * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
                        }

                        int shift = (24 - (i << 3) - diffX);
                        if (shift >= 0 && shift < 32)
                        {
                            source2 |= val << shift;
                        }
                        else if (shift > -32 && shift < 0)
                        {
                            source2 |= val >> -shift;
                        }
                    }

                    if (source1 & source2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool sprlib_checkCollision(struct SpritesLib *lib, int checkIndex, int firstIndex, int lastIndex)
{
    struct Sprite *sprites = lib->core->machine->spriteRegisters.sprites;
    struct Sprite *sprite = &sprites[checkIndex];

    if (sprlib_isSpriteOnScreen(sprite))
    {
        for (int i = firstIndex; i <= lastIndex; i++)
        {
            if (i != checkIndex)
            {
                if (sprlib_checkSingleCollision(lib, sprite, &sprites[i]))
                {
                    lib->lastHit = i;
                    return true;
                }
            }
        }
    }
    return false;
}
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

#include "core.h"
#include "core.h"
#include <string.h>
#include <stdint.h>
#include "core.h"

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

    colors[0] = 2;
    colors[1] = 5;
    colors[2] = 7;
    colors[3] = 3;

    colors[4] = 2;
    colors[5] = 61;
    colors[6] = 62;
    colors[7] = 3;

    colors[8] = 2;
    colors[9] = 18;
    colors[10] = 19;
    colors[11] = 3;

    colors[12] = 2;
    colors[13] = 35;
    colors[14] = 34;
    colors[15] = 3;

    colors[16] = 2;
    colors[17] = 53;
    colors[18] = 54;
    colors[19] = 3;

    colors[20] = 2;
    colors[21] = 13;
    colors[22] = 14;
    colors[23] = 3;

    colors[24] = 2;
    colors[25] = 45;
    colors[26] = 46;
    colors[27] = 3;

    colors[28] = 2;
    colors[29] = 26;
    colors[30] = 27;
    colors[31] = 3;

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

#include "core.h"
#include "core.h"
#include <string.h>
#include <assert.h>

struct Plane *txtlib_getBackground(struct TextLib *lib, int bg)
{
	switch (bg)
	{
	case 0:
		return &lib->core->machine->videoRam.planeA;

	case 1:
		return &lib->core->machine->videoRam.planeB;

	case 2:
		return &lib->core->machine->videoRam.planeC;

	case 3:
		return &lib->core->machine->videoRam.planeD;

	case OVERLAY_BG:
		return &lib->core->overlay->plane;

	default:
		assert(0);
		return NULL;
	}
}

void txtlib_setCellAt(struct Plane *plane, int x, int y, int character, union CharacterAttributes attr)
{
	struct Cell *cell = &plane->cells[y & ROWS_MASK][x & COLS_MASK];
	if (character >= 0)
	{
		cell->character = character;
	}
	cell->attr = attr;
}

void txtlib_scrollRow(struct Plane *plane, int fromX, int toX, int y, int deltaX, int deltaY)
{
	if (deltaX > 0)
	{
		for (int x = toX; x > fromX; x--)
		{
			plane->cells[y][x] = plane->cells[(y - deltaY) & ROWS_MASK][(x - deltaX) & COLS_MASK];
		}
	}
	else if (deltaX < 0)
	{
		for (int x = fromX; x < toX; x++)
		{
			plane->cells[y][x] = plane->cells[(y - deltaY) & ROWS_MASK][(x - deltaX) & COLS_MASK];
		}
	}
	else
	{
		for (int x = fromX; x <= toX; x++)
		{
			plane->cells[y][x] = plane->cells[(y - deltaY) & ROWS_MASK][(x - deltaX) & COLS_MASK];
		}
	}
}

void txtlib_scroll(struct Plane *plane, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY)
{
	if (deltaY > 0)
	{
		for (int y = toY; y > fromY; y--)
		{
			txtlib_scrollRow(plane, fromX, toX, y, deltaX, deltaY);
		}
	}
	else if (deltaY < 0)
	{
		for (int y = fromY; y < toY; y++)
		{
			txtlib_scrollRow(plane, fromX, toX, y, deltaX, deltaY);
		}
	}
	else
	{
		for (int y = fromY; y <= toY; y++)
		{
			txtlib_scrollRow(plane, fromX, toX, y, deltaX, deltaY);
		}
	}
}

void txtlib_scrollWindowIfNeeded(struct TextLib *lib)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->windowBg);

	if (lib->cursorY >= lib->windowHeight)
	{
		// scroll
		txtlib_scroll(plane, lib->windowX, lib->windowY, lib->windowX + lib->windowWidth - 1, lib->windowY + lib->windowHeight - 1, 0, -1);

		// clear bottom line
		int py = lib->windowY + lib->windowHeight - 1;
		for (int x = 0; x < lib->windowWidth; x++)
		{
			int px = x + lib->windowX;
			txtlib_setCellAt(plane, px, py, lib->fontCharOffset, lib->charAttr); // space
		}

		lib->cursorY = lib->windowHeight - 1;

		struct Interpreter *interpreter = lib->core->interpreter;
		if (interpreter->state == StateEvaluate && lib->windowBg != OVERLAY_BG)
		{
			interpreter->waitCount = 1;
			interpreter->exitEvaluation = true;
			interpreter->cycles += lib->windowWidth * lib->windowHeight * 2;
		}
	}
}

void txtlib_printText(struct TextLib *lib, const char *text)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->windowBg);
	const char *letter = text;
	while (*letter)
	{
		txtlib_scrollWindowIfNeeded(lib);

		if (*letter >= 32)
		{
			char printableLetter = *letter;
			if (printableLetter >= 'a' && printableLetter <= 'z')
			{
				printableLetter -= 32;
			}
			txtlib_setCellAt(plane, lib->cursorX + lib->windowX, lib->cursorY + lib->windowY, lib->fontCharOffset + (printableLetter - 32), lib->charAttr);
			if (lib->windowBg != OVERLAY_BG)
			{
				lib->core->interpreter->cycles += 2;
			}
			lib->cursorX++;
		}
		else if (*letter == '\n')
		{
			lib->cursorX = 0;
			lib->cursorY++;
		}

		if (lib->cursorX >= lib->windowWidth)
		{
			lib->cursorX = 0;
			lib->cursorY++;
		}

		letter++;
	}
}

bool txtlib_deleteBackward(struct TextLib *lib)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->windowBg);

	// clear cursor
	txtlib_setCellAt(plane, lib->cursorX + lib->windowX, lib->cursorY + lib->windowY, lib->fontCharOffset, lib->charAttr);

	// move back cursor
	if (lib->cursorX > 0)
	{
		lib->cursorX--;
	}
	else if (lib->cursorY > 0)
	{
		lib->cursorX = lib->windowX + lib->windowWidth - 1;
		lib->cursorY--;
	}
	else
	{
		return false;
	}

	// clear cell
	txtlib_setCellAt(plane, lib->cursorX + lib->windowX, lib->cursorY + lib->windowY, lib->fontCharOffset, lib->charAttr);

	lib->core->interpreter->cycles += 4;
	return true;
}

void txtlib_writeText(struct TextLib *lib, const char *text, int x, int y)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	const char *letter = text;
	while (*letter)
	{
		if (*letter >= 32)
		{
			char printableLetter = *letter;
			if (printableLetter >= 'a' && printableLetter <= 'z')
			{
				printableLetter -= 32;
			}
			txtlib_setCellAt(plane, x, y, lib->fontCharOffset + (printableLetter - 32), lib->charAttr);
			if (lib->windowBg != OVERLAY_BG)
			{
				lib->core->interpreter->cycles += 2;
			}
			x++;
		}
		letter++;
	}
}

void txtlib_writeNumber(struct TextLib *lib, int number, int digits, int x, int y)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);

	if (number < 0)
	{
		// negative number
		number *= -1;
		txtlib_setCellAt(plane, x, y, lib->fontCharOffset + 13, lib->charAttr); // "-"
		x += digits;
		digits--;
	}
	else
	{
		x += digits;
	}

	int div = 1;
	for (int i = 0; i < digits; i++)
	{
		x--;
		txtlib_setCellAt(plane, x, y, lib->fontCharOffset + ((number / div) % 10 + 16), lib->charAttr);
		div *= 10;
	}

	if (lib->windowBg != OVERLAY_BG)
	{
		lib->core->interpreter->cycles += digits * 2;
	}
}

void txtlib_inputBegin(struct TextLib *lib)
{
	lib->inputBuffer[0] = 0;
	lib->inputLength = 0;
	lib->blink = 0;
	lib->core->machine->ioRegisters.key = 0;

	lib->core->machine->ioRegisters.status.keyboardEnabled = 1;
	delegate_controlsDidChange(lib->core);

	txtlib_scrollWindowIfNeeded(lib);
}

bool txtlib_inputUpdate(struct TextLib *lib)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->windowBg);

	char key = lib->core->machine->ioRegisters.key;
	bool done = false;
	if (key)
	{
		if (key == CoreInputKeyBackspace)
		{
			if (lib->inputLength > 0)
			{
				if (txtlib_deleteBackward(lib))
				{
					lib->inputBuffer[--lib->inputLength] = 0;
				}
			}
		}
		else if (key == CoreInputKeyReturn)
		{
			// clear cursor
			txtlib_setCellAt(plane, lib->cursorX + lib->windowX, lib->cursorY + lib->windowY, lib->fontCharOffset, lib->charAttr);
			txtlib_printText(lib, "\n");
			done = true;
		}
		else if (key >= 32)
		{
			if (lib->inputLength < INPUT_BUFFER_SIZE - 2)
			{
				char text[2] = {key, 0};
				txtlib_printText(lib, text);
				lib->inputBuffer[lib->inputLength++] = key;
				lib->inputBuffer[lib->inputLength] = 0;

				txtlib_scrollWindowIfNeeded(lib);
			}
		}
		lib->blink = 0;
		lib->core->machine->ioRegisters.key = 0;
	}
	if (!done)
	{
		txtlib_setCellAt(plane, lib->cursorX + lib->windowX, lib->cursorY + lib->windowY, lib->fontCharOffset + (lib->blink++ < 30 ? 63 : 0), lib->charAttr);
		if (lib->blink == 60)
		{
			lib->blink = 0;
		}
	}
	return done;
}

void txtlib_clearWindow(struct TextLib *lib)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->windowBg);

	lib->cursorX = 0;
	lib->cursorY = 0;
	for (int y = 0; y < lib->windowHeight; y++)
	{
		int py = y + lib->windowY;
		for (int x = 0; x < lib->windowWidth; x++)
		{
			int px = x + lib->windowX;
			txtlib_setCellAt(plane, px, py, lib->fontCharOffset, lib->charAttr);
		}
	}
	lib->core->interpreter->cycles += lib->windowWidth * lib->windowHeight * 2;
}

void txtlib_clearScreen(struct TextLib *lib)
{
	struct VideoRegisters *reg = &lib->core->machine->videoRegisters;

	memset(&lib->core->machine->videoRam.planeA, 0, sizeof(struct Plane));
	memset(&lib->core->machine->videoRam.planeB, 0, sizeof(struct Plane));
	memset(&lib->core->machine->videoRam.planeC, 0, sizeof(struct Plane));
	memset(&lib->core->machine->videoRam.planeD, 0, sizeof(struct Plane));

	reg->scrollAX = 0;
	reg->scrollAY = 0;
	reg->scrollBX = 0;
	reg->scrollBY = 0;
	reg->scrollCX = 0;
	reg->scrollCY = 0;
	reg->scrollDX = 0;
	reg->scrollDY = 0;
	reg->attr.spritesEnabled = 1;
	reg->attr.planeAEnabled = 1;
	reg->attr.planeBEnabled = 1;
	reg->attr.planeCEnabled = 1;
	reg->attr.planeDEnabled = 1;

	lib->windowX = 0;
	lib->windowY = 0;
	lib->windowWidth = 27;
	lib->windowHeight = 48;
	lib->cursorX = 0;
	lib->cursorY = 0;
	lib->bg = 0;

	lib->core->interpreter->cycles += PLANE_COLUMNS * PLANE_ROWS * 2 * 2;
}

void txtlib_clearBackground(struct TextLib *lib, int bg)
{
	struct Plane *plane = txtlib_getBackground(lib, bg);
	memset(plane, 0, sizeof(struct Plane));
	lib->core->interpreter->cycles += PLANE_COLUMNS * PLANE_ROWS * 2;
}

struct Cell *txtlib_getCell(struct TextLib *lib, int x, int y)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	return &plane->cells[y & ROWS_MASK][x & COLS_MASK];
}

void txtlib_setCell(struct TextLib *lib, int x, int y, int character)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	txtlib_setCellAt(plane, x, y, character, lib->charAttr);
}

void txtlib_setCells(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int character)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	for (int y = fromY; y <= toY; y++)
	{
		for (int x = fromX; x <= toX; x++)
		{
			txtlib_setCellAt(plane, x, y, character, lib->charAttr);
		}
	}
	lib->core->interpreter->cycles += (toX - fromX + 1) * (toY - fromY + 1) * 2;
}

void txtlib_setCellsAttr(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int pal, int flipX, int flipY, int prio)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	for (int y = fromY; y <= toY; y++)
	{
		for (int x = fromX; x <= toX; x++)
		{
			struct Cell *cell = &plane->cells[y & ROWS_MASK][x & COLS_MASK];
			if (pal >= 0)
				cell->attr.palette = pal;
			if (flipX >= 0)
				cell->attr.flipX = flipX;
			if (flipY >= 0)
				cell->attr.flipY = flipY;
			if (prio >= 0)
				cell->attr.priority = prio;
		}
	}
	lib->core->interpreter->cycles += (toX - fromX + 1) * (toY - fromY + 1) * 2;
}

void txtlib_scrollBackground(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);
	txtlib_scroll(plane, fromX, fromY, toX, toY, deltaX, deltaY);
	lib->core->interpreter->cycles += (toX - fromX + 1) * (toY - fromY + 1) * 2;
}

void txtlib_copyBackground(struct TextLib *lib, int srcX, int srcY, int width, int height, int dstX, int dstY)
{
	struct Plane *plane = txtlib_getBackground(lib, lib->bg);

	for (int y = 0; y < height; y++)
	{
		int py = dstY + y;
		int addr = lib->sourceAddress + ((srcY + y) * lib->sourceWidth + srcX) * 2;
		for (int x = 0; x < width; x++)
		{
			int px = dstX + x;
			struct Cell *cell = &plane->cells[py & ROWS_MASK][px & COLS_MASK];
			cell->character = machine_peek(lib->core, addr++);
			cell->attr.value = machine_peek(lib->core, addr++);
		}
	}
	lib->core->interpreter->cycles += width * height * 2;
}

int txtlib_getSourceCell(struct TextLib *lib, int x, int y, bool getAttrs)
{
	if (x >= 0 && y >= 0 && x < lib->sourceWidth && y < lib->sourceHeight)
	{
		int address = lib->sourceAddress + ((y * lib->sourceWidth) + x) * 2;
		if (getAttrs)
		{
			return machine_peek(lib->core, address + 1);
		}
		else
		{
			return machine_peek(lib->core, address);
		}
	}
	return -1;
}

bool txtlib_setSourceCell(struct TextLib *lib, int x, int y, int character)
{
	int address = lib->sourceAddress + ((y * lib->sourceWidth) + x) * 2;
	// only working RAM is allowed
	if (address < 0x9000 || address + 1 >= 0xE000)
	{
		return false;
	}

	if (character >= 0)
	{
		machine_poke(lib->core, address, character);
	}
	machine_poke(lib->core, address + 1, lib->charAttr.value);
	return true;
}

void txtlib_itobin(char *buffer, size_t buffersize, size_t width, int value)
{
	if (width < 1)
	{
		width = 1;
	}
	unsigned int mask = 1 << 15;
	int p = 0;
	bool active = false;
	while (mask && p < buffersize - 1)
	{
		if (active || (value & mask) || mask < (1 << width))
		{
			buffer[p++] = (value & mask) ? '1' : '0';
			active = true;
		}
		mask = (mask >> 1) & 0x7FFF;
	}
	buffer[p] = 0;
}
//
// Copyright 2016-2020 Timo Kloss
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

#include "core.h"
#include "core.h"
#include <math.h>
#include <string.h>

const double envRates[16] = {
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

const double lfoRates[16] = {
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

const int lfoAmounts[16] = {
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

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency, int volume);


void audio_reset(struct Core *core)
{
    struct AudioInternals *internals = &core->machineInternals->audioInternals;

    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct VoiceInternals *voiceIn = &internals->voices[i];
        voiceIn->noiseRandom = 0xABCD;
        voiceIn->lfoRandom = 0xABCD;
    }
    internals->writeBufferIndex = -1;
}

void audio_bufferRegisters(struct Core *core)
{
    struct AudioRegisters *registers = &core->machine->audioRegisters;
    struct AudioInternals *internals = &core->machineInternals->audioInternals;

    // next buffer
    int writeBufferIndex = internals->writeBufferIndex;
    if (writeBufferIndex >= 0)
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
    for (int v = 0; v < NUM_VOICES; v++)
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

    while (offset < numSamples)
    {
        if (offset + numSamplesPerUpdate > numSamples)
        {
            numSamplesPerUpdate = numSamples - offset;
        }
        int readBufferIndex = internals->readBufferIndex;
        audio_renderAudioBuffer(lifeRegisters, &internals->buffers[readBufferIndex], internals, &stereoOutput[offset], numSamplesPerUpdate, outputFrequency, volume);
        if (internals->writeBufferIndex != -1 && internals->writeBufferIndex != readBufferIndex)
        {
            internals->readBufferIndex = (readBufferIndex + 1) % NUM_AUDIO_BUFFERS;
        }

        offset += numSamplesPerUpdate;
    }
}

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency, int volume)
{
    double overflow = 0xFFFFFF;

    for (int v = 0; v < NUM_VOICES; v++)
    {
        struct Voice *voice = &registers->voices[v];
        if (voice->status.init)
        {
            voice->status.init = 0;

            struct VoiceInternals *voiceIn = &internals->voices[v];
            voiceIn->envState = EnvStateAttack;
            voiceIn->lfoHold = false;
            voiceIn->timeoutCounter = voice->length;
            if (voice->lfoAttr.envMode || voice->lfoAttr.trigger)
            {
                voiceIn->lfoAccumulator = 0.0;
            }
        }
    }

    int i = 0;
    while (i < numSamples)
    {
        int16_t leftOutput = 0;
        int16_t rightOutput = 0;

        if (internals->audioEnabled)
        {
            for (int v = 0; v < NUM_VOICES; v++)
            {
                struct Voice *voice = &registers->voices[v];
                struct VoiceInternals *voiceIn = &internals->voices[v];

                int freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
                if (freq == 0) continue;

                int volume = voice->status.volume << 4;
                int pulseWidth = voice->attr.pulseWidth << 4;

                // --- LFO ---

                uint8_t lfoAccu8Last = voiceIn->lfoAccumulator;
                if (!voiceIn->lfoHold)
                {
                    double lfoRate = lfoRates[voice->lfoFrequency];
                    double lfoAccumulator = voiceIn->lfoAccumulator + lfoRate / (double)outputFrequency;
                    if (voice->lfoAttr.envMode && lfoAccumulator >= 255.0)
                    {
                        lfoAccumulator = 255.0;
                        voiceIn->lfoHold = true;
                    }
                    else if (lfoAccumulator >= 256.0)
                    {
                        // avoid overflow and loss of precision
                        lfoAccumulator -= 256.0;
                    }
                    voiceIn->lfoAccumulator = lfoAccumulator;
                }
                uint8_t lfoAccu8 = voiceIn->lfoAccumulator;
                uint8_t lfoSample = 0;

                enum LFOWaveType lfoWaveType = voice->lfoAttr.wave;
                switch (lfoWaveType)
                {
                    case LFOWaveTypeTriangle:
                    {
                        lfoSample = ((lfoAccu8 & 0x80) ? ~(lfoAccu8 << 1) : (lfoAccu8 << 1));
                        break;
                    }
                    case LFOWaveTypeSawtooth:
                    {
                        lfoSample = ~lfoAccu8;
                        break;
                    }
                    case LFOWaveTypeSquare:
                    {
                        lfoSample = (lfoAccu8 & 0x80) ? 0x00 : 0xFF;
                        break;
                    }
                    case LFOWaveTypeRandom:
                    {
                        if ((lfoAccu8 & 0x80) != (lfoAccu8Last & 0x80))
                        {
                            uint16_t r = voiceIn->lfoRandom;
                            uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
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
                if (voice->lfoAttr.invert) freq -= freqMod; else freq += freqMod;
                if (freq < 1) freq = 1;
                if (freq > 65535) freq = 65535;

                if (voice->lfoAttr.invert)
                {
                    volume -= volume * lfoSample * volAmount >> 12;
                }
                else
                {
                    volume -= volume * (~lfoSample & 0xFF) * volAmount >> 12;
                }
                if (volume < 0) volume = 0;
                if (volume > 255) volume = 255;

                int pwMod = lfoSample * pwAmount >> 4;
                if (voice->lfoAttr.invert) pulseWidth -= pwMod; else pulseWidth += pwMod;
                if (pulseWidth < 0) pulseWidth = 0;
                if (pulseWidth > 254) pulseWidth = 254;

//                if (i == 0 && v == 0) printf("pulseWidth %d\n", pulseWidth);

                // --- WAVEFORM GENERATOR ---

                uint16_t accu16Last = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;
                double accumulator = voiceIn->accumulator + (double)freq * 65536.0 / (double)outputFrequency;
                if (accumulator >= overflow)
                {
                    // avoid overflow and loss of precision
                    accumulator -= overflow;
                }
                voiceIn->accumulator = accumulator;
                uint16_t accu16 = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;

                uint16_t sample = 0x7FFF; // silence

                enum WaveType waveType = voice->attr.wave;
                switch (waveType)
                {
                    case WaveTypeSawtooth:
                    {
                        sample = accu16;
                        break;
                    }
                    case WaveTypePulse:
                    {
                        sample = ((accu16 >> 8) > pulseWidth) ? 0xFFFF : 0x0000;
                        break;
                    }
                    case WaveTypeTriangle:
                    {
                        sample = ((accu16 & 0x8000) ? ~(accu16 << 1) : (accu16 << 1));
                        break;
                    }
                    case WaveTypeNoise:
                    {
                        if ((accu16 & 0x1000) != (accu16Last & 0x1000))
                        {
                            uint16_t r = voiceIn->noiseRandom;
                            uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
                            voiceIn->noiseRandom = (r >> 1) | (bit << 15);
                        }
                        sample = voiceIn->noiseRandom & 0xFFFF;
                        break;
                    }
                }

                // --- TIMEOUT ---

                if (voice->attr.timeout)
                {
                    voiceIn->timeoutCounter -= 60.0 / outputFrequency;
                    if (voiceIn->timeoutCounter <= 0.0)
                    {
                        voiceIn->timeoutCounter = 0.0;
                        voice->status.gate = 0;
                    }
                }

                // --- ENVELOPE GENERATOR ---

                if (!voice->status.gate)
                {
                    voiceIn->envState = EnvStateRelease;
                }

                switch (voiceIn->envState) {
                    case EnvStateAttack:
                        voiceIn->envCounter += envRates[voice->envA] / outputFrequency;
                        if (voiceIn->envCounter >= 255.0)
                        {
                            voiceIn->envCounter = 255.0;
                            voiceIn->envState = EnvStateDecay;
                        }
                        break;

                    case EnvStateDecay:
                        if (voiceIn->envCounter > voice->envS * 16.0)
                        {
                            voiceIn->envCounter -= envRates[voice->envD] / outputFrequency;
                        }
                        break;

                    case EnvStateRelease:
                        if (voiceIn->envCounter > 0.0)
                        {
                            voiceIn->envCounter -= envRates[voice->envR] / outputFrequency;
                            if (voiceIn->envCounter < 0.0)
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

                int16_t voiceSample = (((int32_t)(sample - 0x7FFF)) * volume) >> 10; // 8 bit for volume, 2 bit for global
                if (voice->status.mix & 0x01)
                {
                    leftOutput += voiceSample;
                }
                if (voice->status.mix & 0x02)
                {
                    rightOutput += voiceSample;
                }
            }

            // filter

            int32_t *filterBufferL = internals->filterBuffer[0];
            int32_t *filterBufferR = internals->filterBuffer[1];

            for (int f = AUDIO_FILTER_BUFFER_SIZE - 1; f > 0; f--)
            {
                filterBufferL[f] = filterBufferL[f - 1];
                filterBufferR[f] = filterBufferR[f - 1];
            }
            filterBufferL[0] = leftOutput;
            filterBufferR[0] = rightOutput;

            leftOutput  = ((filterBufferL[0] >> 4) + (filterBufferL[1] >> 1) + (filterBufferL[2] >> 4));
            rightOutput = ((filterBufferR[0] >> 4) + (filterBufferR[1] >> 1) + (filterBufferR[2] >> 4));
        }

        stereoOutput[i++] = leftOutput >> volume;
        stereoOutput[i++] = rightOutput >> volume;
    }
}
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

#include "core.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "core.h"
#include "core.h"
#include "core.h"
#include "core.h"

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
    // if (address == 0xFF76) // IO attributes
    // {
    //     // check for illegal input change (gamepad <-> touch)
    //     union IOAttributes currAttr = core->machine->ioRegisters.attr;
    //     union IOAttributes newAttr;
    //     newAttr.value = value;
    // }
    // else
		if (address >= 0x0E000 && address < 0x0F800) // persistent
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
//
// Copyright 2016-2020 Timo Kloss
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

#include "core.h"
#include "core.h"
#include <string.h>

#define OVERLAY_FLAG (1<<6)

// // FAMICUBE
// uint32_t better_palette[]={ 0x000000, 0xe03c28, 0xffffff, 0xd7d7d7, 0xa8a8a8, 0x7b7b7b, 0x343434, 0x151515, 0x0d2030, 0x415d66, 0x71a6a1, 0xbdffca, 0x25e2cd, 0x0a98ac, 0x005280, 0x00604b, 0x20b562, 0x58d332, 0x139d08, 0x004e00, 0x172808, 0x376d03, 0x6ab417, 0x8cd612, 0xbeeb71, 0xeeffa9, 0xb6c121, 0x939717, 0xcc8f15, 0xffbb31, 0xffe737, 0xf68f37, 0xad4e1a, 0x231712, 0x5c3c0d, 0xae6c37, 0xc59782, 0xe2d7b5, 0x4f1507, 0x823c3d, 0xda655e, 0xe18289, 0xf5b784, 0xffe9c5, 0xff82ce, 0xcf3c71, 0x871646, 0xa328b3, 0xcc69e4, 0xd59cfc, 0xfec9ed, 0xe2c9ff, 0xa675fe, 0x6a31ca, 0x5a1991, 0x211640, 0x3d34a5, 0x6264dc, 0x9ba0ef, 0x98dcff, 0x5ba8ff, 0x0a89ff, 0x024aca, 0x00177d, };

// FAMICUBE
uint32_t better_palette[]={ 0xff000000, 0xffe03c28, 0xffffffff, 0xffd7d7d7, 0xffa8a8a8, 0xff7b7b7b, 0xff343434, 0xff151515, 0xff0d2030, 0xff415d66, 0xff71a6a1, 0xffbdffca, 0xff25e2cd, 0xff0a98ac, 0xff005280, 0xff00604b, 0xff20b562, 0xff58d332, 0xff139d08, 0xff004e00, 0xff172808, 0xff376d03, 0xff6ab417, 0xff8cd612, 0xffbeeb71, 0xffeeffa9, 0xffb6c121, 0xff939717, 0xffcc8f15, 0xffffbb31, 0xffffe737, 0xfff68f37, 0xffad4e1a, 0xff231712, 0xff5c3c0d, 0xffae6c37, 0xffc59782, 0xffe2d7b5, 0xff4f1507, 0xff823c3d, 0xffda655e, 0xffe18289, 0xfff5b784, 0xffffe9c5, 0xffff82ce, 0xffcf3c71, 0xff871646, 0xffa328b3, 0xffcc69e4, 0xffd59cfc, 0xfffec9ed, 0xffe2c9ff, 0xffa675fe, 0xff6a31ca, 0xff5a1991, 0xff211640, 0xff3d34a5, 0xff6264dc, 0xff9ba0ef, 0xff98dcff, 0xff5ba8ff, 0xff0a89ff, 0xff024aca, 0xff00177d, };

int video_getCharacterPixel(struct Character *character, int x, int y)
{
    int b0 = (character->data[y] >> (7 - x)) & 0x01;
    int b1 = (character->data[y | 8] >> (7 - x)) & 0x01;
    return b0 | (b1 << 1);
}

void video_renderPlane(struct Character *characters, struct Plane *plane, int sizeMode, int y, int scrollX, int scrollY, int pixelFlag, uint8_t *scanlineBuffer, bool opaque)
{
    int divShift = sizeMode ? 4 : 3;
    int planeY = y + scrollY;
    int row = (planeY >> divShift) & ROWS_MASK;
    int cellY = planeY & 7;

    int x = 0;
    int b = 0;
    uint8_t d0 = 0;
    uint8_t d1 = 0;
    uint8_t pal = 0;
    uint8_t pri = 0;

    int pre = scrollX & 7;

    while (x < SCREEN_WIDTH)
    {
        if (!b)
        {
            int planeX = x + scrollX;
            int column = (planeX >> divShift) & COLS_MASK;
            struct Cell *cell = &plane->cells[row][column];

            int index = cell->character;
            if (sizeMode)
            {
                index += (cell->attr.flipX ? (planeX >> 3) + 1 : planeX >> 3) & 1;
                index += ((cell->attr.flipY ? (planeY >> 3) + 1 : planeY >> 3) & 1) << 4;
            }

            struct Character *character = &characters[index];
            pal = cell->attr.palette << 2;
            pri = cell->attr.priority << 7;

            int fcy = cell->attr.flipY ? (7 - cellY) : cellY;
            d0 = character->data[fcy];
            d1 = character->data[fcy | 8];
            if (cell->attr.flipX)
            {
                // reverse bits hack from http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
                d0 = ((d0 * 0x0802LU & 0x22110LU) | (d0 * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
                d1 = ((d1 * 0x0802LU & 0x22110LU) | (d1 * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
            }
        }

        if (pre)
        {
            --pre;
        }
        else
        {
            if (pri >= (*scanlineBuffer >> 7))
            {
                int pixel = ((d0 >> 7) & 1) | ((d1 >> 6) & 2);
								// if color is not zero, replace it by the new color, I guess
								if (pixel || (pixel==0 && opaque))
                {
                    *scanlineBuffer = pixel | pal | pri | pixelFlag;
                }
            }
            ++scanlineBuffer;
            ++x;
        }

        d0 <<= 1;
        d1 <<= 1;
        b = (b + 1) & 7;
    }
}

void video_renderSprites(struct SpriteRegisters *reg, struct VideoRam *ram, int y, uint8_t *scanlineBuffer, uint8_t *scanlineSpriteBuffer)
{
    for (int i = NUM_SPRITES - 1; i >= 0; i--)
    {
        struct Sprite *sprite = &reg->sprites[i];
        if ((sprite->x/16) != 0 || (sprite->y/16) != 0)
        {
            int spriteY = y - (sprite->y/16) + SPRITE_OFFSET_Y;
            int size = (sprite->attr.size + 1) << 3;
            if (spriteY >= 0 && spriteY < size)
            {
                if (sprite->attr.flipY)
                {
                    spriteY = size - spriteY - 1;
                }
                int charIndex = sprite->character + ((spriteY >> 3) << 4);
                if (sprite->attr.flipX)
                {
                    charIndex += sprite->attr.size;
                }
                int minX = (sprite->x/16) - SPRITE_OFFSET_X;
                int maxX = minX + size;
                if (minX < 0)
                {
                    int skip = -minX >> 3;
                    if (sprite->attr.flipX)
                    {
                        charIndex -= skip;
                    }
                    else
                    {
                        charIndex += skip;
                    }
                }
                if (minX < 0) minX = 0;
                if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
                uint8_t *buffer = &scanlineSpriteBuffer[minX];
                int spriteX = minX - (int)(sprite->x/16) + SPRITE_OFFSET_X;
                if (sprite->attr.flipX)
                {
                    spriteX = size - spriteX - 1;
                }
                struct Character *character = &ram->characters[charIndex];
                for (int x = minX; x < maxX; x++)
                {
                    int pixel = video_getCharacterPixel(character, spriteX & 0x07, spriteY & 0x07);
                    if (pixel)
                    {
                        *buffer = pixel | (sprite->attr.palette << 2) | (sprite->attr.priority << 7);
                    }
                    buffer++;
                    if (sprite->attr.flipX)
                    {
                        if (!(spriteX & 0x07))
                        {
                            character--;
                        }
                        spriteX--;
                    }
                    else
                    {
                        spriteX++;
                        if (!(spriteX & 0x07))
                        {
                            character++;
                        }
                    }
                }
            }
        }
    }
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int pixel = *scanlineSpriteBuffer;
        if (pixel && (pixel >> 7) >= (*scanlineBuffer >> 7))
        {
            *scanlineBuffer = pixel;
        }
        scanlineSpriteBuffer++;
        scanlineBuffer++;
    }
}

void video_renderScreen(struct Core *core, uint32_t *outputRGB)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t scanlineSpriteBuffer[SCREEN_WIDTH];
    uint32_t *outputPixel = outputRGB;

    struct VideoRam *ram = &core->machine->videoRam;
    struct VideoRegisters *reg = &core->machine->videoRegisters;
    struct SpriteRegisters *sreg = &core->machine->spriteRegisters;
    struct ColorRegisters *creg = &core->machine->colorRegisters;
    struct IORegisters *io = &core->machine->ioRegisters;
		struct MachineInternals *mi = core->machineInternals;

    int width=SCREEN_WIDTH;
    int height=SCREEN_HEIGHT;
    int skip_before=0;
    int skip_after=0;
    if (core->interpreter->compat)
    {
        int sw=io->shown.width!=0?io->shown.width:SCREEN_WIDTH;
        int sh=io->shown.height!=0?io->shown.height:SCREEN_HEIGHT;
        width=160;
        height=128;
        skip_before=(sw-width)/2;
        skip_after=SCREEN_WIDTH-width-skip_before;
        outputPixel+=SCREEN_WIDTH*(sh-height)/2;
    }
    for (int y = 0; y<height; y++)
    {
        reg->rasterLine = y;
        if (core->interpreter->compat && y>=0 && y<120)
        {
            itp_runInterrupt(core, InterruptTypeRaster);
        }
        else
        {
            itp_runInterrupt(core, InterruptTypeRaster);
        }
        memset(scanlineBuffer, 0, sizeof(scanlineBuffer));

        bool skip = (core->interpreter->interruptOverCycles > 0);
        if (!skip)
        {
            if (reg->attr.planeDEnabled)
            {
                int scrollX = reg->scrollDX; // | (reg->scrollMSB.dX << 8);
                int scrollY = reg->scrollDY; // | (reg->scrollMSB.dY << 8);
                video_renderPlane(ram->characters, &ram->planeD,
								0, //reg->attr.planeDCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[3]);
            }
            if (reg->attr.planeCEnabled)
            {
                int scrollX = reg->scrollCX; // | (reg->scrollMSB.cX << 8);
                int scrollY = reg->scrollCY; // | (reg->scrollMSB.cY << 8);
                video_renderPlane(ram->characters, &ram->planeC,
								0, //reg->attr.planeCCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[2]);
            }
            if (reg->attr.planeBEnabled)
            {
                int scrollX = reg->scrollBX; // | (reg->scrollMSB.bX << 8);
                int scrollY = reg->scrollBY; // | (reg->scrollMSB.bY << 8);
                video_renderPlane(ram->characters, &ram->planeB,
								0, //reg->attr.planeBCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[1]);
            }
            if (reg->attr.planeAEnabled)
            {
                int scrollX = reg->scrollAX; // | (reg->scrollMSB.aX << 8);
                int scrollY = reg->scrollAY; // | (reg->scrollMSB.aY << 8);
                video_renderPlane(ram->characters, &ram->planeA,
								0, //reg->attr.planeACellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[0]);
            }
            if (reg->attr.spritesEnabled)
            {
                memset(scanlineSpriteBuffer, 0, sizeof(scanlineSpriteBuffer));
                video_renderSprites(sreg, ram, y, scanlineBuffer, scanlineSpriteBuffer);
            }
        }

        // overlay
        video_renderPlane((struct Character *)overlayCharacters, &core->overlay->plane, 0, y, 0, 0, OVERLAY_FLAG, scanlineBuffer, 0);

        outputPixel+=skip_before;

        for (int x = 0; x < width; x++)
        {
            int colorIndex = scanlineBuffer[x] & 0x1F;
            int color = (scanlineBuffer[x] & OVERLAY_FLAG) ? overlayColors[colorIndex] : skip ? 0 : creg->colors[colorIndex];

            uint32_t c = better_palette[color & 63];

#if BGR
            uint32_t a=(c>>24)&0xff;
            uint32_t r=(c>>16)&0xff;
            uint32_t g=(c>>8)&0xff;
            uint32_t b=c&0xff;
            c=(a<<24)|(b<<16)|(g<<8)|r;
#endif
            *outputPixel = c;
            ++outputPixel;
        }

        outputPixel+=skip_after;
    }
}
//
// Copyright 2017-2018 Timo Kloss
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

#include "core.h"
#include "core.h"
#include "core.h"
#include <math.h>

void overlay_clear(struct Core *core);


void overlay_init(struct Core *core)
{
    struct IORegisters *io = &core->machine->ioRegisters;
    struct TextLib *lib = &core->overlay->textLib;
    lib->core = core;
    lib->bg = OVERLAY_BG;
    lib->windowBg = OVERLAY_BG;
    lib->charAttr.priority = 1;
    lib->charAttr.palette = 1;
    lib->fontCharOffset = 0;
    lib->windowX = 0;
    lib->windowY = 0;
    lib->windowWidth = 216/8;
    lib->windowHeight = 384/8;
    lib->cursorX = 0;
    lib->cursorY = 0;
}

void overlay_updateLayout(struct Core *core, struct CoreInput *input)
{
    struct IORegisters *io = &core->machine->ioRegisters;
    struct TextLib *lib = &core->overlay->textLib;
    lib->windowX = (io->safe.left+7)/8;
    lib->windowY = (io->safe.top+7)/8;
    lib->windowWidth = io->shown.width/8 - (io->safe.left+7)/8 - (io->safe.right+7)/8;
    lib->windowHeight = io->shown.height/8 - (io->safe.top+7)/8 - (io->safe.bottom+7)/8;
}

void overlay_reset(struct Core *core)
{
    overlay_clear(core);
    core->overlay->textLib.cursorX = 0;
    core->overlay->textLib.cursorY = 0;
}

void overlay_updateState(struct Core *core)
{
    overlay_clear(core);

    if (core->interpreter->state == StatePaused)
    {
        core->overlay->timer = 0;
    }

    if (!core->interpreter->debug)
    {
        core->overlay->textLib.cursorX = 0;
        core->overlay->textLib.cursorY = 0;
    }
}

void overlay_message(struct Core *core, const char *message)
{
    struct TextLib *lib = &core->overlay->textLib;
    txtlib_setCells(lib,
        0, lib->windowHeight-1+lib->windowY,
        lib->windowWidth-1, lib->windowHeight-1+lib->windowY,
        0);
    txtlib_writeText(lib, message, lib->windowX, lib->windowHeight-1+lib->windowY);
    core->overlay->messageTimer = 127;
    machine_suspendEnergySaving(core, 127);
}

void overlay_draw(struct Core *core, bool ingame)
{
    struct TextLib *lib = &core->overlay->textLib;

    if (core->overlay->messageTimer > 0)
    {
        core->overlay->messageTimer--;
        if (core->overlay->messageTimer < 27)
        {
            txtlib_scrollBackground(lib,
                0, lib->windowHeight-1+lib->windowY,
                lib->windowWidth-1, lib->windowHeight-1+lib->windowY,
                -1, 0);
            txtlib_setCell(lib,
                lib->windowWidth-1+lib->windowX, lib->windowHeight-1+lib->windowY,
                0);
        }
    }

    if (ingame)
    {
        if (core->interpreter->state == StatePaused)
        {
            if (core->overlay->timer % 60 < 40)
            {
                txtlib_writeText(lib, "PAUSED", 7, 7);
            }
            else
            {
                // XXX: What is this 12?
                txtlib_setCells(lib, 7, 7, 12, 7, 0);
            }
        }

        if (core->interpreter->debug)
        {
            txtlib_writeText(lib, "CPU", lib->windowWidth-3+lib->windowX, lib->windowY);
            int cpuLoad = core->interpreter->cpuLoadDisplay;
            if (cpuLoad < 100)
            {
                txtlib_writeNumber(lib, cpuLoad, 2, lib->windowWidth-3+lib->windowX, 1+lib->windowY);
                txtlib_writeText(lib, "%", lib->windowWidth-1+lib->windowX, 1+lib->windowY);
            }
            else
            {
                txtlib_writeText(lib, "MAX", lib->windowWidth-3+lib->windowX, 1+lib->windowY);
            }
        }
    }

    core->overlay->timer++;
}

void overlay_clear(struct Core *core)
{
    struct Plane *plane = &core->overlay->plane;
    for (int y = 0; y < PLANE_ROWS; y++)
    {
        for (int x = 0; x < PLANE_COLUMNS; x++)
        {
            struct Cell *cell = &plane->cells[y][x];
            cell->character = 0;
            cell->attr.palette = 0;
            cell->attr.priority = 1;
        }
    }
    core->overlay->messageTimer = 0;
}

//
// Copyright 2017-2018 Timo Kloss
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

#include "core.h"

uint8_t overlayColors[] = {
    // gamepads
    0,
    2,
    3,
    4,
    // paused text
    0,
    2,
    0,
    0
};

uint8_t overlayCharacters[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x24, 0x24, 0x24, 0x24, 0x3C, 0x24, 0x3C,
    0xFE, 0xFE, 0xFE, 0xFE, 0x7E, 0x00, 0x00, 0x00, 0xFE, 0x92, 0x92, 0xDA, 0x7E, 0x00, 0x00, 0x00,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xDB, 0x81, 0xDB, 0xDB, 0x81, 0xDB, 0x7E,
    0x1C, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x1C, 0x1C, 0x77, 0x41, 0x47, 0x71, 0x41, 0x77, 0x1C,
    0xF7, 0xFF, 0xFF, 0xFE, 0x7F, 0xFF, 0xFF, 0xEF, 0xF7, 0x9D, 0x9B, 0xF6, 0x6F, 0xD9, 0xB9, 0xEF,
    0x3E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3E, 0x62, 0x4A, 0xC7, 0x91, 0x9B, 0xC5, 0x7F,
    0x3C, 0x3C, 0x7C, 0x7C, 0x78, 0x00, 0x00, 0x00, 0x3C, 0x24, 0x64, 0x4C, 0x78, 0x00, 0x00, 0x00,
    0x1E, 0x3E, 0x7E, 0x7C, 0x7C, 0x7E, 0x3E, 0x1E, 0x1E, 0x32, 0x66, 0x4C, 0x4C, 0x66, 0x32, 0x1E,
    0x78, 0x7C, 0x7E, 0x3E, 0x3E, 0x7E, 0x7C, 0x78, 0x78, 0x4C, 0x66, 0x32, 0x32, 0x66, 0x4C, 0x78,
    0x00, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x00, 0x7E, 0x5A, 0xE7, 0x81, 0xE7, 0x5A, 0x7E,
    0x00, 0x3C, 0x3C, 0xFF, 0xFF, 0xFF, 0x3C, 0x3C, 0x00, 0x3C, 0x24, 0xE7, 0x81, 0xE7, 0x24, 0x3C,
    0x00, 0x00, 0x00, 0x3C, 0x3C, 0x7C, 0x7C, 0x78, 0x00, 0x00, 0x00, 0x3C, 0x24, 0x64, 0x4C, 0x78,
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x24, 0x24, 0x3C,
    0x0F, 0x1F, 0x3F, 0x7E, 0xFC, 0xF8, 0xF0, 0xE0, 0x0F, 0x19, 0x33, 0x66, 0xCC, 0x98, 0xB0, 0xE0,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0x91, 0x89, 0x99, 0xC3, 0x7E,
    0x3C, 0x7C, 0x7C, 0x7C, 0x3C, 0xFF, 0xFF, 0xFF, 0x3C, 0x64, 0x44, 0x64, 0x24, 0xE7, 0x81, 0xFF,
    0x7E, 0xFF, 0xFF, 0xFF, 0x7E, 0xFF, 0xFF, 0xFF, 0x7E, 0xC3, 0x99, 0xF3, 0x66, 0xCF, 0x81, 0xFF,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0xF3, 0xF9, 0x99, 0xC3, 0x7E,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0xFF, 0x99, 0x99, 0x81, 0xF9, 0x09, 0x09, 0x0F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0x81, 0x9F, 0x83, 0xF9, 0xF9, 0x83, 0xFE,
    0x3E, 0x7E, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3E, 0x62, 0xCE, 0x83, 0x99, 0x99, 0xC3, 0x7E,
    0xFF, 0xFF, 0xFF, 0x3F, 0x7E, 0x7C, 0x78, 0x78, 0xFF, 0x81, 0xF9, 0x33, 0x66, 0x4C, 0x48, 0x78,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0xC3, 0x99, 0x99, 0xC3, 0x7E,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0xC1, 0xF9, 0x99, 0xC3, 0x7E,
    0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x3C, 0x24, 0x3C, 0x24, 0x3C, 0x00,
    0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x7C, 0x7C, 0x78, 0x00, 0x00, 0x3C, 0x24, 0x3C, 0x64, 0x4C, 0x78,
    0x00, 0x1E, 0x3E, 0x7E, 0x7C, 0x7E, 0x3E, 0x1E, 0x00, 0x1E, 0x32, 0x66, 0x4C, 0x66, 0x32, 0x1E,
    0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xFF, 0x81, 0xFF, 0x00,
    0x00, 0x78, 0x7C, 0x7E, 0x3E, 0x7E, 0x7C, 0x78, 0x00, 0x78, 0x4C, 0x66, 0x32, 0x66, 0x4C, 0x78,
    0x7E, 0xFF, 0xFF, 0xFF, 0x3E, 0x3C, 0x3C, 0x3C, 0x7E, 0xC3, 0x99, 0xF3, 0x26, 0x3C, 0x24, 0x3C,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x7E, 0x7E, 0xC3, 0x99, 0x91, 0x91, 0x9F, 0xC2, 0x7E,
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3C, 0x66, 0xC3, 0x99, 0x81, 0x99, 0x99, 0xFF,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0x83, 0x99, 0x83, 0x99, 0x99, 0x83, 0xFE,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0x9F, 0x9F, 0x99, 0xC3, 0x7E,
    0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0xFC, 0x86, 0x93, 0x99, 0x99, 0x93, 0x86, 0xFC,
    0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, 0x9F, 0x84, 0x9C, 0x9F, 0x81, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xF0, 0xF0, 0xF0, 0xFF, 0x81, 0x9F, 0x84, 0x9C, 0x90, 0x90, 0xF0,
    0x7E, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC2, 0x9F, 0x91, 0x99, 0x99, 0xC3, 0x7E,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0x99, 0x81, 0x99, 0x99, 0x99, 0xFF,
    0x7E, 0x7E, 0x7E, 0x3C, 0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x42, 0x66, 0x24, 0x24, 0x66, 0x42, 0x7E,
    0x3F, 0x3F, 0x3F, 0x0F, 0xFF, 0xFF, 0xFF, 0x7E, 0x3F, 0x21, 0x39, 0x09, 0xF9, 0x99, 0xC3, 0x7E,
    0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0x93, 0x86, 0x86, 0x93, 0x99, 0xFF,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x9F, 0x81, 0xFF,
    0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xBD, 0x99, 0x81, 0x81, 0x99, 0x99, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0x89, 0x81, 0x91, 0x99, 0x99, 0xFF,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0x99, 0x99, 0x99, 0xC3, 0x7E,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 0xF0, 0xF0, 0xF0, 0xFE, 0x83, 0x99, 0x83, 0x9E, 0x90, 0x90, 0xF0,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7E, 0xC3, 0x99, 0x99, 0x95, 0x93, 0xC1, 0x7F,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFE, 0x83, 0x99, 0x83, 0x86, 0x93, 0x99, 0xFF,
    0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFE, 0x7F, 0xC1, 0x9F, 0xC3, 0x79, 0xF9, 0x83, 0xFE,
    0xFF, 0xFF, 0xFF, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0xFF, 0x81, 0xE7, 0x24, 0x24, 0x24, 0x24, 0x3C,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0xFF, 0x99, 0x99, 0x99, 0x99, 0x99, 0xC3, 0x7E,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0xFF, 0x99, 0x99, 0x99, 0x99, 0xC3, 0x66, 0x3C,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xFF, 0x99, 0x99, 0x81, 0x81, 0x99, 0xBD, 0xE7,
    0xFF, 0xFF, 0xFF, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xC3, 0x66, 0xC3, 0x99, 0x99, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x3C, 0x3C, 0xFF, 0x99, 0x99, 0xC3, 0x66, 0x24, 0x24, 0x3C,
    0xFF, 0xFF, 0xFF, 0x7E, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, 0xF3, 0x66, 0xCC, 0x9F, 0x81, 0xFF,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0x99, 0xA5, 0xBD, 0xA5, 0xC3, 0x7E,
    0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0xC3, 0xA1, 0xB9, 0xA5, 0xB9, 0xC3, 0x7E,
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x3C, 0x66, 0xE7, 0x81, 0x81, 0xE7, 0x66, 0x3C,
    0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x3C, 0x66, 0xC3, 0x99, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xFF,
};
/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

/*
 * This code is derived from the full C implementation, which is in turn
 * derived from the canonical C++ PCG implementation. The C++ version
 * has many additional features and is preferable if you can use C++ in
 * your project.
 */

#include "core.h"

// state for global RNGs

static pcg32_random_t pcg32_global = PCG32_INITIALIZER;

// pcg32_srandom(initstate, initseq)
// pcg32_srandom_r(rng, initstate, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initstate;
    pcg32_random_r(rng);
}

void pcg32_srandom(uint64_t seed, uint64_t seq)
{
    pcg32_srandom_r(&pcg32_global, seed, seq);
}

// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint32_t pcg32_random()
{
    return pcg32_random_r(&pcg32_global);
}


// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound

uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound)
{
    // To avoid bias, we need to make the range of the RNG a multiple of
    // bound, which we do by dropping output less than a threshold.
    // A naive scheme to calculate the threshold would be to do
    //
    //     uint32_t threshold = 0x100000000ull % bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     uint32_t threshold = (0x100000000ull-bound) % bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.

    uint32_t threshold = -bound % bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    for (;;) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold)
            return r % bound;
    }
}


uint32_t pcg32_boundedrand(uint32_t bound)
{
    return pcg32_boundedrand_r(&pcg32_global, bound);
}

