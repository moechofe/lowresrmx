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

#ifndef scenario_h
#define scenario_h

#include "core.h"
#include <stdbool.h>
#include <stdint.h>

#define SCENARIO_MAX_ACTIONS 64
#define SCENARIO_MAX_CHECKS 64
#define SCENARIO_TEXT_SIZE 192
#define SCENARIO_LINE_SIZE 256

#define SCENARIO_DEFAULT_FRAMES 600
#define SCENARIO_DEFAULT_WIDTH 216
#define SCENARIO_DEFAULT_HEIGHT 384

enum ScenarioActionType
{
	ScenarioActionTouch,
	ScenarioActionMove,
	ScenarioActionRelease,
	ScenarioActionKey,
	ScenarioActionResize,
	ScenarioActionSafe,
	ScenarioActionKeyboard,
	ScenarioActionKeyboardOff,
	ScenarioActionPause
};

enum ScenarioCheckType
{
	ScenarioCheckAssertions,
	ScenarioCheckState,
	ScenarioCheckPixels,
	ScenarioCheckAudio,
	ScenarioCheckPortrait,
	ScenarioCheckKeyboard,
	ScenarioCheckHaptic
};

struct ScenarioAction
{
	int frame;
	enum ScenarioActionType type;
	float x, y;
	int left, top, right, bottom;
	int height;
	char key;
};

struct ScenarioCheck
{
	int frame;
	enum ScenarioCheckType type;
	int value;
	uint64_t hash;
	// line of the '@check directive in the case file, 1 based; --update-goldens rewrites it
	int line;

	// filled by the harness while the case runs
	bool evaluated;
	bool hasActual;
	uint64_t actual;
};

struct Scenario
{
	int frames;
	int thumbnailFrame;
	int runs;
	int width, height;
	int left, top, right, bottom;
	bool allowTimeout;

	bool hasExpectedError;
	enum ErrorCode expectedError;
	bool hasExpectedCompileError;
	enum ErrorCode expectedCompileError;

	bool skip;
	char skipReason[SCENARIO_TEXT_SIZE];

	struct ScenarioAction actions[SCENARIO_MAX_ACTIONS];
	int numActions;
	struct ScenarioCheck checks[SCENARIO_MAX_CHECKS];
	int numChecks;

	bool hasParseError;
	int parseErrorLine;
	char parseError[SCENARIO_TEXT_SIZE];
};

void scenario_init(struct Scenario *scenario);

/** Reads the leading '@ annotation block of a case. Returns false and fills parseError on failure. */
bool scenario_parse(struct Scenario *scenario, const char *source);

const char *scenario_stateName(enum State state);
const char *scenario_hapticName(enum HapticMode mode);
const char *scenario_checkName(enum ScenarioCheckType type);

/** Resolves a numeric code or a space/case insensitive ErrorStrings name. Returns false if unknown. */
bool scenario_parseErrorCode(const char *text, enum ErrorCode *outCode);

#endif /* scenario_h */
