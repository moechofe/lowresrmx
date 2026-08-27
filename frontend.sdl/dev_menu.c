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

#include "config.h"

#if DEV_MENU

#include "core.h"
#include "dev_menu.h"
#include "dev_menu_data.h"
#include "libraries/text_lib.h"
#include "main.h"
#include "string_utils.h"
#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// pixels of vertical movement before a press turns into a scroll
#define DEV_DRAG_THRESHOLD 4.0f

// widest "[KEY] LABEL" in devShortcuts, plus one space
#define DEV_SHORTCUT_WIDTH 15

extern struct CoreInput coreInput;

struct DevShortcut
{
	const char *key;
	const char *label; // NULL for the debug toggle, whose label carries its state
};

// Keep in sync with the dev menu shortcut block in main.c update().
static const struct DevShortcut devShortcuts[] = {
	{"ESC", "RUN"},
	{"R", "RELOAD"},
	{"D", NULL},
	{"E", "EJECT"},
	{"S", "SCREENSHOT"},
	{"Z", "ZOOM"},
	{"+/-", "VOLUME"},
};

static void dev_draw(struct DevMenu *devMenu);
static void dev_setScroll(struct DevMenu *devMenu, int scrollY);
static void dev_clearPersistentRam(void);

void dev_init(struct DevMenu *devMenu, struct Runner *runner, struct Settings *settings)
{
	memset(devMenu, 0, sizeof(struct DevMenu));
	devMenu->runner = runner;
	devMenu->settings = settings;
	devMenu->pressedItem = -1;
}

// ================ Drawing helpers ================

// txtlib wraps coordinates around the 64x64 plane instead of clipping, so every
// string has to be truncated to the space it is given.
static void dev_write(struct DevMenu *devMenu, const char *text, int x, int y, int maxX)
{
	char buffer[PLANE_COLUMNS + 1];
	int max = maxX - x + 1;

	if(max <= 0)
	{
		return;
	}
	if(max > PLANE_COLUMNS)
	{
		max = PLANE_COLUMNS;
	}
	strncpy(buffer, text, max);
	buffer[max] = 0;
	txtlib_writeText(&devMenu->textLib, buffer, x, y);
}

// paints a whole row in the current palette, so a differently-papered palette
// reads as a band instead of colouring only the glyph cells
static void dev_fillRow(struct DevMenu *devMenu, int y, int fromX, int toX)
{
	txtlib_setCells(&devMenu->textLib, fromX, y, toX, y, DEV_FONT_OFFSET);
}

static void dev_writeRight(struct DevMenu *devMenu, const char *text, int minX, int y, int rightX)
{
	int x = rightX - (int)strlen(text) + 1;

	if(x < minX)
	{
		x = minX;
	}
	dev_write(devMenu, text, x, y, rightX);
}

// word wraps text into x..maxX, returns the number of rows used (at least one).
// bandFromX > bandToX skips the row banding, see dev_fillRow.
static int dev_writeWrapped(struct DevMenu *devMenu, const char *text, int x, int y, int maxX, int bandFromX, int bandToX)
{
	int width = maxX - x + 1;
	int rows = 0;

	if(width <= 0)
	{
		return 0;
	}
	while(*text)
	{
		int take = 0, lastSpace = -1;
		while(text[take] && take < width)
		{
			if(text[take] == ' ')
			{
				lastSpace = take;
			}
			take++;
		}
		if(text[take] && lastSpace > 0)
		{
			take = lastSpace;
		}
		{
			char buffer[PLANE_COLUMNS + 1];
			if(bandFromX <= bandToX)
			{
				dev_fillRow(devMenu, y + rows, bandFromX, bandToX);
			}
			strncpy(buffer, text, take);
			buffer[take] = 0;
			txtlib_writeText(&devMenu->textLib, buffer, x, y + rows);
		}
		rows++;
		text += take;
		while(*text == ' ')
		{
			text++;
		}
	}
	return rows > 0 ? rows : 1;
}

static void dev_addItem(struct DevMenu *devMenu, enum DevItemType type, int index, int fromX, int toX, int row, int height, int palette)
{
	struct DevItem *item;

	if(devMenu->numItems >= (int)(sizeof(devMenu->items) / sizeof(devMenu->items[0])))
	{
		return;
	}
	item = &devMenu->items[devMenu->numItems++];
	item->type = type;
	item->index = index;
	item->fromX = fromX;
	item->toX = toX;
	item->row = row;
	item->height = height;
	item->palette = palette;
}

// Three lines of source around sourcePosition, the offending one marked with ">".
// Same idea as print_code in backend.core/overlay/overlay_debugger.c, but anchored
// on a compile error instead of the interpreter's program counter.
static int dev_drawCodeExtract(struct DevMenu *devMenu, int sourcePosition, int x, int y, int maxX)
{
	const char *source = devMenu->runner->core->interpreter->sourceCode;
	int positions[3];
	int pos, i;

	if(!source)
	{
		return y;
	}

	pos = sourcePosition;
	if(pos > 0 && (source[pos] == 0 || source[pos] == '\n'))
	{
		pos--;
	}
	while(pos > 0 && source[pos - 1] != '\n')
	{
		pos--;
	}

	// the line before, the offending line, the line after
	positions[1] = pos;
	positions[0] = -1;
	positions[2] = -1;
	if(pos > 0)
	{
		int prev = pos - 1;
		while(prev > 0 && source[prev - 1] != '\n')
		{
			prev--;
		}
		positions[0] = prev;
	}
	{
		int next = pos;
		while(source[next] != 0 && source[next] != '\n')
		{
			next++;
		}
		if(source[next] == '\n' && source[next + 1] != 0)
		{
			positions[2] = next + 1;
		}
	}

	for(i = 0; i < 3; i++)
	{
		const char *line;

		if(positions[i] < 0)
		{
			continue;
		}
		line = lineString(source, positions[i]);
		if(!line)
		{
			continue;
		}
		{
			char buffer[PLANE_COLUMNS + 1];
			devMenu->textLib.charAttr.palette = (i == 1) ? DEV_PAL_WARNING : DEV_PAL_BODY;
			if(i == 1)
			{
				dev_fillRow(devMenu, y, x, maxX);
			}
			snprintf(buffer,
				sizeof(buffer),
				"%c%4d %s",
				(i == 1) ? '>' : ' ',
				lineNumber(source, positions[i]),
				line);
			dev_write(devMenu, buffer, x, y, maxX);
		}
		free((void *)line);
		y++;
	}
	return y;
}

// ================ Layout ================

static void dev_draw(struct DevMenu *devMenu)
{
	struct Core *core = devMenu->runner->core;
	struct TextLib *textLib = &devMenu->textLib;
	struct IORegisters *io = &core->machine->ioRegisters;
	char info[PLANE_COLUMNS + 1];
	int shownWidth = io->shown.width ? io->shown.width : SCREEN_WIDTH;
	int x0 = (io->safe.left + 7) / 8;
	int y0 = (io->safe.top + 7) / 8;
	int x1 = shownWidth / 8 - (io->safe.right + 7) / 8 - 1;
	int row, i, perLine;

	if(x1 > PLANE_COLUMNS - 1)
	{
		x1 = PLANE_COLUMNS - 1;
	}
	if(x1 < x0 + 7)
	{
		// never lay out narrower than 8 cells, rather let it clip
		x1 = x0 + 7;
	}

	devMenu->numItems = 0;
	devMenu->lastShownWidth = io->shown.width;
	devMenu->lastShownHeight = io->shown.height;
	devMenu->lastSafeLeft = io->safe.left;
	devMenu->lastSafeTop = io->safe.top;
	devMenu->lastSafeRight = io->safe.right;
	devMenu->lastSafeBottom = io->safe.bottom;

	// paper over the whole plane height, so scrolling past the content still
	// shows the page and not the transparent border colour
	txtlib_clearBackground(textLib, 0);
	textLib->charAttr.palette = DEV_PAL_BODY;
	txtlib_setCells(textLib, x0, y0, x1, PLANE_ROWS - 1, DEV_FONT_OFFSET);

	row = y0;

	// ---- compile error ----
	if(devMenu->lastError.code != ErrorNone)
	{
		textLib->charAttr.palette = DEV_PAL_ERROR;
		row += dev_writeWrapped(devMenu, err_getString(devMenu->lastError.code), x0, row, x1, x0, x1);
		if(devMenu->lastError.sourcePosition >= 0)
		{
			row = dev_drawCodeExtract(devMenu, devMenu->lastError.sourcePosition, x0, row, x1);
		}
		row++;
	}

	// ---- program info ----
	devMenu->titleRow = row;
	if(devMenu->dropZone == DevDropZoneMainProgram)
	{
		textLib->charAttr.palette = DEV_PAL_WARNING;
		dev_fillRow(devMenu, row, x0, x1);
		dev_write(devMenu, "DROP TO RUN IT", x0, row, x1);
	}
	else
	{
		textLib->charAttr.palette = DEV_PAL_TITLE;
		dev_fillRow(devMenu, row, x0, x1);
		displayName(getMainProgramFilename(), info, sizeof(info));
		dev_write(devMenu, info, x0, row, x1);
	}
	row++;

	textLib->charAttr.palette = DEV_PAL_LABEL;
	dev_write(devMenu, "TOKENS", x0, row, x1);
	textLib->charAttr.palette = DEV_PAL_BODY;
	snprintf(info, sizeof(info), "%d/%d", core->interpreter->tokenizer.numTokens, MAX_TOKENS);
	dev_writeRight(devMenu, info, x0, row, x1);
	row++;

	textLib->charAttr.palette = DEV_PAL_LABEL;
	dev_write(devMenu, "ROM", x0, row, x1);
	textLib->charAttr.palette = DEV_PAL_BODY;
	snprintf(info, sizeof(info), "%d/%d", data_currentSize(&core->interpreter->romDataManager), DATA_SIZE);
	dev_writeRight(devMenu, info, x0, row, x1);
	row++;

	if(devMenu->lastError.code == ErrorNone)
	{
		textLib->charAttr.palette = DEV_PAL_OK;
		dev_write(devMenu, "READY TO RUN", x0, row, x1);
		row++;
	}
	row++;

	// ---- shortcuts ----
	textLib->charAttr.palette = DEV_PAL_LABEL;
	dev_write(devMenu, "SHORTCUTS", x0, row, x1);
	row++;

	textLib->charAttr.palette = DEV_PAL_BODY;
	perLine = (x1 - x0 + 1) / DEV_SHORTCUT_WIDTH;
	if(perLine < 1)
	{
		perLine = 1;
	}
	for(i = 0; i < (int)(sizeof(devShortcuts) / sizeof(devShortcuts[0])); i++)
	{
		int column = i % perLine;

		if(devShortcuts[i].label)
		{
			snprintf(info, sizeof(info), "[%s] %s", devShortcuts[i].key, devShortcuts[i].label);
		}
		else
		{
			snprintf(info, sizeof(info), "[%s] DEBUG %s", devShortcuts[i].key, core->interpreter->debug ? "ON" : "OFF");
		}
		dev_write(devMenu, info, x0 + column * DEV_SHORTCUT_WIDTH, row, x1);
		if(column == perLine - 1 || i == (int)(sizeof(devShortcuts) / sizeof(devShortcuts[0])) - 1)
		{
			row++;
		}
	}
	row++;

	// ---- editor programs ----
	// this heading is also where the editors drop zone starts, see dev_dropZoneAt
	devMenu->editorsRow = row;
	if(devMenu->dropZone == DevDropZoneEditors)
	{
		textLib->charAttr.palette = DEV_PAL_WARNING;
		dev_fillRow(devMenu, row, x0, x1);
		dev_write(devMenu, "DROP TO ADD IT", x0, row, x1);
	}
	else
	{
		textLib->charAttr.palette = DEV_PAL_LABEL;
		dev_fillRow(devMenu, row, x0, x1);
		dev_write(devMenu, "EDITOR PROGRAMS", x0, row, x1);
	}
	row++;

	for(i = 0; i < devMenu->settings->numTools; i++)
	{
		int removeX = x1 - 2;

		textLib->charAttr.palette = DEV_PAL_BUTTON;
		dev_fillRow(devMenu, row, x0, removeX - 1);
		dev_write(devMenu, devMenu->settings->toolNames[i], x0 + 1, row, removeX - 2);
		textLib->charAttr.palette = DEV_PAL_ERROR;
		dev_fillRow(devMenu, row, removeX, x1);
		dev_write(devMenu, "[X]", removeX, row, x1);
		dev_addItem(devMenu, DevItemTool, i, x0, removeX - 1, row, 1, DEV_PAL_BUTTON);
		dev_addItem(devMenu, DevItemToolRemove, i, removeX, x1, row, 1, DEV_PAL_ERROR);
		row++;
	}
	textLib->charAttr.palette = DEV_PAL_LABEL;
	if(devMenu->settings->numTools == 0)
	{
		dev_write(devMenu, "NONE", x0 + 1, row, x1);
		row++;
	}
	if(devMenu->settings->numTools < MAX_TOOLS)
	{
		row += dev_writeWrapped(devMenu, "DROP A PROGRAM ANYWHERE BELOW TO ADD IT", x0 + 1, row, x1, -1, -1);
	}
	row++;

	// ---- persistent ram ----
	{
		int palette = devMenu->clearRamArmed ? DEV_PAL_WARNING : DEV_PAL_BUTTON;
		int rows;

		textLib->charAttr.palette = palette;
		rows = dev_writeWrapped(devMenu,
			devMenu->clearRamArmed ? "CLICK AGAIN TO CONFIRM" : "CLEAR PERSISTENT RAM",
			x0 + 1,
			row,
			x1,
			x0,
			x1);
		dev_addItem(devMenu, DevItemClearRam, -1, x0, x1, row, rows, palette);
		row += rows;
	}

	textLib->charAttr.palette = DEV_PAL_LABEL;
	row += dev_writeWrapped(devMenu, "MAY DELETE GAME STATE OR HIGH SCORES OF THIS PROGRAM", x0 + 2, row, x1, -1, -1);
	row++;

	devMenu->contentHeight = row;
	textLib->charAttr.palette = DEV_PAL_BODY;
}

// ================ Scrolling ================

static void dev_setScroll(struct DevMenu *devMenu, int scrollY)
{
	struct Core *core = devMenu->runner->core;
	struct IORegisters *io = &core->machine->ioRegisters;
	int shownHeight = io->shown.height ? io->shown.height : SCREEN_HEIGHT;
	int max = (devMenu->contentHeight + (io->safe.bottom + 7) / 8) * 8 - shownHeight;

	if(max < 0)
	{
		max = 0;
	}
	if(scrollY > max)
	{
		scrollY = max;
	}
	if(scrollY < 0)
	{
		scrollY = 0;
	}
	devMenu->scrollY = scrollY;
	core->machine->videoRegisters.scrollAY = (uint16_t)scrollY;
	machine_suspendEnergySaving(core, 2);
}

void dev_scroll(struct DevMenu *devMenu, int deltaPixels)
{
	dev_setScroll(devMenu, devMenu->scrollY + deltaPixels);
}

void dev_relayout(struct DevMenu *devMenu)
{
	dev_draw(devMenu);
	dev_setScroll(devMenu, devMenu->scrollY);
}

// ================ Interaction ================

static int dev_hitTest(struct DevMenu *devMenu, float touchX, float touchY)
{
	int cx, cy, i;

	if(touchX < 0 || touchY < 0)
	{
		return -1;
	}
	cx = (int)touchX / 8;
	cy = ((int)touchY + devMenu->scrollY) / 8;
	for(i = 0; i < devMenu->numItems; i++)
	{
		struct DevItem *item = &devMenu->items[i];

		if(cx >= item->fromX && cx <= item->toX && cy >= item->row && cy < item->row + item->height)
		{
			return i;
		}
	}
	return -1;
}

static void dev_highlightItem(struct DevMenu *devMenu, int index, bool pressed)
{
	struct DevItem *item = &devMenu->items[index];

	txtlib_setCellsAttr(&devMenu->textLib,
		item->fromX,
		item->row,
		item->toX,
		item->row + item->height - 1,
		pressed ? DEV_PAL_TITLE : item->palette,
		-1,
		-1,
		-1);
}

// returns false when the menu is gone and devMenu must not be touched again
static bool dev_activate(struct DevMenu *devMenu, int index)
{
	struct DevItem *item = &devMenu->items[index];

	if(item->type != DevItemClearRam && devMenu->clearRamArmed)
	{
		devMenu->clearRamArmed = false;
	}

	switch(item->type)
	{
	case DevItemTool:
		runToolProgram(devMenu->settings->tools[item->index]);
		return false;

	case DevItemToolRemove:
		settings_removeTool(devMenu->settings, item->index);
		settings_save(devMenu->settings);
		dev_relayout(devMenu);
		return true;

	case DevItemClearRam:
		if(devMenu->clearRamArmed)
		{
			devMenu->clearRamArmed = false;
			dev_clearPersistentRam();
			dev_relayout(devMenu);
			overlay_message(devMenu->runner->core, "PERSISTENT RAM CLEARED");
		}
		else
		{
			devMenu->clearRamArmed = true;
			dev_relayout(devMenu);
		}
		return true;
	}
	return true;
}

void dev_show(struct DevMenu *devMenu, bool reload)
{
	struct Core *core = devMenu->runner->core;
	struct TextLib *textLib = &devMenu->textLib;

	if(reload)
	{
		devMenu->lastError = runner_loadProgram(devMenu->runner, getMainProgramFilename());
	}

	textLib->core = core;

	itp_endProgram(core);
	machine_reset(core, true);
	overlay_reset(core);

	// The menu owns the whole screen. A COMPAT program leaves interpreter->compat set,
	// which crops rendering to a centred 160x128 box (video_renderScreen) and offsets
	// every touch coordinate by the same amount (core_handleInput) -- so the page would
	// be clipped to 20x16 cells and mis-hit-tested. Clear it before handling input;
	// leaving the menu recompiles and reruns the program, which sets it again.
	core->interpreter->compat = false;
	delegate_controlsDidChange(core);

	core_handleInput(core, &coreInput);
	overlay_updateLayout(core, &coreInput);

	core->machineInternals->isEnergySaving = true;

	txtlib_clearScreen(textLib);
	textLib->bg = 0;
	textLib->windowBg = 0;
	textLib->fontCharOffset = DEV_FONT_OFFSET;

	memcpy(&core->machine->colorRegisters, dev_colors, sizeof(dev_colors));
	memcpy(&core->machine->videoRam.characters[DEV_FONT_OFFSET], dev_font, sizeof(dev_font));

	devMenu->scrollY = 0;
	devMenu->pressedItem = -1;
	devMenu->lastTouch = false;
	devMenu->dragging = false;
	devMenu->clearRamArmed = false;
	devMenu->dropZone = DevDropZoneNone;
	devMenu->dropTarget = DevDropZoneNone;

	dev_relayout(devMenu);

	setMouseEnabled(true);
}

void dev_update(struct DevMenu *devMenu, struct CoreInput *input)
{
	struct Core *core = devMenu->runner->core;
	struct IORegisters *io = &core->machine->ioRegisters;
	bool touch;
	float touchX, touchY;

	core_handleInput(core, input);
	overlay_updateLayout(core, input);

	// the window was resized or the safe area moved, reflow the page
	if(io->shown.width != devMenu->lastShownWidth || io->shown.height != devMenu->lastShownHeight ||
	   io->safe.left != devMenu->lastSafeLeft || io->safe.top != devMenu->lastSafeTop ||
	   io->safe.right != devMenu->lastSafeRight || io->safe.bottom != devMenu->lastSafeBottom)
	{
		devMenu->pressedItem = -1;
		dev_relayout(devMenu);
	}

	touch = io->status.touch;
	touchX = io->touchX;
	touchY = io->touchY;

	if(touch && !devMenu->lastTouch)
	{
		// just pressed
		devMenu->dragStartTouchY = touchY;
		devMenu->dragStartScrollY = devMenu->scrollY;
		devMenu->dragging = false;
		devMenu->pressedItem = dev_hitTest(devMenu, touchX, touchY);
		if(devMenu->pressedItem >= 0)
		{
			dev_highlightItem(devMenu, devMenu->pressedItem, true);
		}
	}
	else if(touch)
	{
		// held: past the threshold this is a scroll, not a press
		if(!devMenu->dragging && fabsf(touchY - devMenu->dragStartTouchY) > DEV_DRAG_THRESHOLD)
		{
			devMenu->dragging = true;
			if(devMenu->pressedItem >= 0)
			{
				dev_highlightItem(devMenu, devMenu->pressedItem, false);
				devMenu->pressedItem = -1;
			}
			if(devMenu->clearRamArmed)
			{
				devMenu->clearRamArmed = false;
				dev_relayout(devMenu);
			}
		}
		if(devMenu->dragging)
		{
			dev_setScroll(devMenu, devMenu->dragStartScrollY - (int)(touchY - devMenu->dragStartTouchY));
		}
	}
	else if(devMenu->lastTouch)
	{
		// just released
		int pressed = devMenu->pressedItem;

		devMenu->pressedItem = -1;
		devMenu->lastTouch = false;
		if(pressed >= 0)
		{
			dev_highlightItem(devMenu, pressed, false);
			if(!devMenu->dragging && dev_hitTest(devMenu, touchX, touchY) == pressed)
			{
				devMenu->dragging = false;
				if(!dev_activate(devMenu, pressed))
				{
					return;
				}
			}
		}
		devMenu->dragging = false;
	}
	devMenu->lastTouch = touch;

	overlay_draw(core, false);
}

// The page is split in two, not into a small target and a big fallback: the
// editors zone is everything from the EDITOR PROGRAMS heading to the bottom.
static enum DevDropZone dev_dropZoneAt(struct DevMenu *devMenu, float screenY)
{
	if(screenY < 0)
	{
		// no position from the platform, fall back to replacing the main program
		return DevDropZoneMainProgram;
	}
	if(((int)screenY + devMenu->scrollY) / 8 >= devMenu->editorsRow)
	{
		return DevDropZoneEditors;
	}
	return DevDropZoneMainProgram;
}

void dev_handleDropPosition(struct DevMenu *devMenu, float screenY)
{
	enum DevDropZone zone = dev_dropZoneAt(devMenu, screenY);

	devMenu->dropTarget = zone;
	if(zone != devMenu->dropZone)
	{
		devMenu->dropZone = zone;
		dev_relayout(devMenu);
	}
}

void dev_handleDropEnd(struct DevMenu *devMenu)
{
	if(devMenu->dropZone != DevDropZoneNone)
	{
		devMenu->dropZone = DevDropZoneNone;
		dev_relayout(devMenu);
	}
}

bool dev_handleDropFile(struct DevMenu *devMenu, const char *filename, float screenY)
{
	// Prefer the zone the page already highlighted from SDL_EVENT_DROP_POSITION: that
	// is what the user aimed at, and it is the only source that is reliable on every
	// backend. SDL_EVENT_DROP_FILE carries the last reported position too, but only as
	// a copy that some backends never fill in -- use it just as a fallback.
	enum DevDropZone zone = devMenu->dropTarget;

	if(zone == DevDropZoneNone)
	{
		// no hover position was ever reported, all we have is the file event's copy
		zone = dev_dropZoneAt(devMenu, screenY);
	}
	devMenu->dropZone = DevDropZoneNone;
	devMenu->dropTarget = DevDropZoneNone;
	if(zone != DevDropZoneEditors)
	{
		// let main.c load it as the main program instead
		dev_relayout(devMenu);
		return false;
	}

	if(settings_addTool(devMenu->settings, filename))
	{
		settings_save(devMenu->settings);
		dev_relayout(devMenu);
	}
	else
	{
		dev_relayout(devMenu);
		overlay_message(devMenu->runner->core, "TOO MANY PROGRAMS");
	}
	return true;
}

static void dev_clearPersistentRam(void)
{
	char ramFilename[FILENAME_MAX];

	getRamFilename(ramFilename);
	remove(ramFilename);
}

#endif
