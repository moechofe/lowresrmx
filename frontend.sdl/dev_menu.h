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

#ifndef dev_menu_h
#define dev_menu_h

#include "config.h"

#if DEV_MENU

#include "core.h"
#include "libraries/text_lib.h"
#include "runner.h"
#include "settings.h"
#include <stdbool.h>
#include <stdio.h>

// While a file is being dragged over the window the page highlights the half it
// would land in, so the two behaviours are visible before the mouse is released.
enum DevDropZone
{
	DevDropZoneNone,
	DevDropZoneMainProgram,
	DevDropZoneEditors
};

enum DevItemType
{
	DevItemTool,
	DevItemToolRemove,
	DevItemClearRam
};

// One clickable region. dev_draw records these as it draws them, so hit testing
// never has to reproduce the layout arithmetic -- that duplication is what made
// the old fixed 20x16 menu impossible to resize.
struct DevItem
{
	enum DevItemType type;
	int index; // tool index, -1 when the type has a single instance
	int fromX, toX; // cell columns
	int row, height; // cell rows, in content space (before scrolling)
	int palette; // palette to restore after a press highlight
};

struct DevMenu
{
	struct Runner *runner;
	struct Settings *settings;
	struct TextLib textLib;
	struct CoreError lastError;

	// the page is drawn once into plane A (64 rows) and scrolled with scrollAY
	int scrollY; // pixels
	int contentHeight; // cells, measured by the last dev_draw

	// the page splits in two drop zones at editorsRow: everything from the EDITOR
	// PROGRAMS heading down adds an editor program, everything above it loads the
	// dropped file as the main program. titleRow/editorsRow are the rows that get
	// highlighted to show which is which.
	int titleRow, editorsRow;
	// dropZone is what is highlighted right now, and is cleared when the drag ends.
	// dropTarget is the last zone a drag was seen over and deliberately outlives the
	// drag: SDL_EVENT_DROP_FILE only carries a *copy* of the last reported position,
	// and on some backends that copy is already zeroed by the time the file arrives.
	// The drop must follow what was highlighted, so it reads dropTarget, not the event.
	enum DevDropZone dropZone, dropTarget;

	struct DevItem items[MAX_TOOLS * 2 + 1];
	int numItems;

	int pressedItem; // index into items, -1 for none
	bool lastTouch;
	bool dragging;
	float dragStartTouchY;
	int dragStartScrollY;
	bool clearRamArmed;

	// SHOWN and SAFE as of the last dev_draw, to notice a resize
	int lastShownWidth, lastShownHeight;
	int lastSafeLeft, lastSafeTop, lastSafeRight, lastSafeBottom;
};

void dev_init(struct DevMenu *devMenu, struct Runner *runner, struct Settings *settings);
void dev_show(struct DevMenu *devMenu, bool reload);
void dev_update(struct DevMenu *devMenu, struct CoreInput *input);
void dev_relayout(struct DevMenu *devMenu);
void dev_scroll(struct DevMenu *devMenu, int deltaPixels);
void dev_handleDropPosition(struct DevMenu *devMenu, float screenY);
void dev_handleDropEnd(struct DevMenu *devMenu);
bool dev_handleDropFile(struct DevMenu *devMenu, const char *filename, float screenY);

#endif

#endif /* dev_menu_h */
