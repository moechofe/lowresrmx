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

#if SCREENSHOTS

#include "core.h"
#include "golden.h"
#include "screenshot.h"
#include "system_paths.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool screenshot_save(uint32_t *pixels, int pitch, int scale)
{
	char filename[FILENAME_MAX];

	desktop_path(filename, FILENAME_MAX);
	size_t len = strlen(filename);

	time_t rawtime;
	time(&rawtime);
	struct tm *timeinfo = localtime(&rawtime);
	strftime(&filename[len], FILENAME_MAX - len - 1, "LowRes NX %Y-%m-%d %H_%M_%S.png", timeinfo);
	return writeImage(filename, SCREEN_WIDTH, SCREEN_HEIGHT, pixels, pitch, scale);
}

#endif
