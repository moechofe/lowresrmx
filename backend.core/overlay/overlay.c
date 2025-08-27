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

#include "overlay.h"
#include "core.h"
#include "io_chip.h"
#include "overlay_debugger.h"
#include <math.h>
#include <string.h>

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
	lib->windowWidth = 216 / 8;
	lib->windowHeight = 384 / 8;
	lib->cursorX = 0;
	lib->cursorY = 0;
}

void overlay_updateLayout(struct Core *core, struct CoreInput *input)
{
	struct IORegisters *io = &core->machine->ioRegisters;
	struct TextLib *lib = &core->overlay->textLib;
	int b = io->safe.bottom > io->keyboardHeight ? io->safe.bottom : io->keyboardHeight;
	lib->windowX = (io->safe.left + 7) / 8;
	lib->windowY = (io->safe.top + 7) / 8;
	lib->windowWidth = io->shown.width / 8 - (io->safe.left + 7) / 8 - (io->safe.right + 7) / 8;
	lib->windowHeight = io->shown.height / 8 - (io->safe.top + 7) / 8 - (b + 7) / 8;
}

void overlay_reset(struct Core *core)
{
	overlay_clear(core);
	core->overlay->textLib.cursorX = core->overlay->textLib.windowX;
	core->overlay->textLib.cursorY = core->overlay->textLib.windowY;
	memset(core->overlay->commandLine, 0, 27);
}

void overlay_updateState(struct Core *core)
{
	if (core->interpreter->state == StatePaused)
	{
		core->overlay->timer = 0;
	}

	if (!core->interpreter->debug)
	{
		core->overlay->textLib.cursorX = core->overlay->textLib.windowX;
		core->overlay->textLib.cursorY = core->overlay->textLib.windowY;
	}
}

void overlay_message(struct Core *core, const char *message)
{
	struct TextLib *lib = &core->overlay->textLib;
	txtlib_setCells(lib,
									0, lib->windowHeight - 1 + lib->windowY,
									lib->windowWidth - 1, lib->windowHeight - 1 + lib->windowY,
									0);
	txtlib_writeText(lib, message, lib->windowX, lib->windowHeight - 1 + lib->windowY);
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
															0, lib->windowHeight - 1 + lib->windowY,
															lib->windowWidth - 1, lib->windowHeight - 1 + lib->windowY,
															-1, 0);
			txtlib_setCell(lib,
										 lib->windowWidth - 1 + lib->windowX, lib->windowHeight - 1 + lib->windowY,
										 0);
		}
	}

	if (ingame)
	{
		if (core->interpreter->state == StatePaused)
		{
			if (core->overlay->timer % 40 < 20)
			{
				txtlib_writeText(lib, "_", lib->windowX + lib->cursorX, lib->windowY + lib->cursorY);
			}
			else
			{
				char c[2] = {' ', 0};
				if (lib->cursorX >= 0 && lib->cursorX < 26 && core->overlay->commandLine[lib->cursorX] != 0)
					*c = core->overlay->commandLine[lib->cursorX];
				txtlib_writeText(lib, c, lib->windowX + lib->cursorX, lib->windowY + lib->cursorY);
			}
		}

		if (core->interpreter->debug)
		{
			txtlib_writeText(lib, "CPU", lib->windowWidth - 3 + lib->windowX, lib->windowY);
			int cpuLoad = core->interpreter->cpuLoadDisplay;
			if (cpuLoad < 100)
			{
				txtlib_writeNumber(lib, cpuLoad, 2, lib->windowWidth - 3 + lib->windowX, 1 + lib->windowY);
				txtlib_writeText(lib, "%", lib->windowWidth - 1 + lib->windowX, 1 + lib->windowY);
			}
			else
			{
				txtlib_writeText(lib, "MAX", lib->windowWidth - 3 + lib->windowX, 1 + lib->windowY);
			}
		}

		if (core->interpreter->state == StatePaused && core->interpreter->debug)
		{
			overlay_debugger(core);
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
