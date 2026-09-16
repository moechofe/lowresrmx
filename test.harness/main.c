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

#include "harness.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define makeDirectory(path) _mkdir(path)
#else
#include <sys/stat.h>
#define makeDirectory(path) mkdir(path, 0777)
#endif

// backend.core/core.c reads this when SDL_SCALING is on, which frontend.sdl/config.h turns on
// for every build that compiles the engine with frontend.sdl on its include path. The headless
// harness has no renderer, so the scale is the identity.
#if SDL_SCALING
float rendererScale = 1;
#endif

static struct HarnessSuite suite;
static struct Core core;

static void usage(void)
{
	printf("usage: LowResRMXTest [--filter <substr>] [--update-goldens] [--out <dir>] [-v] <path>...\n");
	printf("       <path> is a .rmx/.nx case or a directory of cases\n");
}

int main(int argc, const char **argv)
{
	const char *filter = NULL;
	const char *outDir = "test.harness/build";
	bool updateGoldens = false;
	bool verbose = false;
	const char *paths[64];
	int numPaths = 0;

	for(int i = 1; i < argc; i++)
	{
		const char *argument = argv[i];
		if(strcmp(argument, "--filter") == 0 && i + 1 < argc)
		{
			filter = argv[++i];
		}
		else if(strcmp(argument, "--out") == 0 && i + 1 < argc)
		{
			outDir = argv[++i];
		}
		else if(strcmp(argument, "--update-goldens") == 0)
		{
			updateGoldens = true;
		}
		else if(strcmp(argument, "-v") == 0)
		{
			verbose = true;
		}
		else if(strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0)
		{
			usage();
			return 0;
		}
		else if(argument[0] == '-')
		{
			printf("unknown option %s\n", argument);
			usage();
			return 2;
		}
		else if(numPaths < (int)(sizeof(paths) / sizeof(*paths)))
		{
			paths[numPaths++] = argument;
		}
	}

	if(numPaths == 0)
	{
		usage();
		return 2;
	}

	makeDirectory(outDir);

	harness_suiteInit(&suite, outDir);
	harness_setUpdateGoldens(&suite, updateGoldens);
	for(int i = 0; i < numPaths; i++)
	{
		if(!harness_suiteAddPath(&suite, paths[i], filter))
		{
			printf("could not read %s\n", paths[i]);
			return 2;
		}
	}

	if(harness_suiteCount(&suite) == 0)
	{
		printf("no cases matched\n");
		return 2;
	}

	core_init(&core);

	for(int i = 0; i < harness_suiteCount(&suite); i++)
	{
		struct CoreInput input;
		memset(&input, 0, sizeof(input));

		if(harness_beginCase(&suite, &core, i))
		{
			while(!harness_caseDone(&suite))
			{
				harness_beforeUpdate(&suite, &input);
				core_update(&core, &input);
				harness_afterUpdate(&suite, &core, &input);
			}
		}
		harness_endCase(&suite, &core);

		if(verbose)
		{
			printf("%s\n", harness_statusLine(&suite));
			fflush(stdout);
		}
	}

	harness_report(&suite, stdout);

	int exitCode = harness_exitCode(&suite);
	harness_suiteDeinit(&suite);
	core_deinit(&core);
	return exitCode;
}
