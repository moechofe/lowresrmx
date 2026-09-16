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

#ifndef harness_h
#define harness_h

#include "core.h"
#include "golden.h"
#include "scenario.h"
#include "video_chip.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HARNESS_MAX_CASES 256
#define HARNESS_PATH_SIZE 512
#define HARNESS_STEM_SIZE 96
#define HARNESS_MAX_MESSAGES 8
#define HARNESS_MESSAGE_SIZE 192

enum HarnessVerdict
{
	HarnessVerdictPending,
	HarnessVerdictPass,
	HarnessVerdictFail,
	HarnessVerdictTimeout,
	HarnessVerdictSkip,
	/** Annotation the harness could not parse; counted as a failure, never ignored. */
	HarnessVerdictError
};

struct HarnessCase
{
	char path[HARNESS_PATH_SIZE];
	// path relative to the suite root that was added, used in the report
	char name[HARNESS_PATH_SIZE];
	// file name without directory or extension, used for the sandbox files
	char stem[HARNESS_STEM_SIZE];
	enum HarnessVerdict verdict;
	// mismatches found while running; messages alone never decide a verdict
	int failures;
	char messages[HARNESS_MAX_MESSAGES][HARNESS_MESSAGE_SIZE];
	int numMessages;
	int numAssertions;
	int numFrames;
};

struct HarnessSuite
{
	struct CoreDelegate delegate;
	char outDir[HARNESS_PATH_SIZE];
	bool updateGoldens;

	struct HarnessCase cases[HARNESS_MAX_CASES];
	int numCases;

	// running case, -1 when none
	int caseIndex;
	int run;
	int frame;
	bool done;
	bool runStarted;
	char *source;
	struct Scenario scenario;

	bool hasCompileError;
	struct CoreError compileError;
	bool hasRuntimeError;
	struct CoreError runtimeError;

	// enum HapticMode values seen since the last '@check haptic; the IO register is
	// zeroed by delegate_controlsDidChange, so recording here is the only way to observe them
	uint32_t hapticSeen;
	int numControlsChanges;

	// SHOWN/SAFE the scenario currently asks for, reapplied every frame so both drivers agree
	int width, height, left, top, right, bottom;

	uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
	int16_t samples[GOLDEN_AUDIO_SAMPLES];

	int numPassed, numFailed, numTimeout, numSkipped, numErrors;
	char statusLine[HARNESS_MESSAGE_SIZE];
};

void harness_suiteInit(struct HarnessSuite *suite, const char *outDir);
void harness_suiteDeinit(struct HarnessSuite *suite);

/** Adds a case file, or every .rmx/.nx below a directory, sorted by path. */
bool harness_suiteAddPath(struct HarnessSuite *suite, const char *path, const char *filter);
int harness_suiteCount(const struct HarnessSuite *suite);

/** Returns false when the verdict is already decided; the driver must then skip straight to endCase. */
bool harness_beginCase(struct HarnessSuite *suite, struct Core *core, int index);
void harness_beforeUpdate(struct HarnessSuite *suite, struct CoreInput *input);
void harness_afterUpdate(struct HarnessSuite *suite, struct Core *core, struct CoreInput *input);
bool harness_caseDone(const struct HarnessSuite *suite);
void harness_endCase(struct HarnessSuite *suite, struct Core *core);

void harness_didFail(struct HarnessSuite *suite, struct CoreError error);
void harness_controlsDidChange(struct HarnessSuite *suite, struct ControlsInfo info);

// Per case accessors, so drivers that cannot read the struct layout (Swift, bindings) do not
// have to reach into the fixed size arrays above.
const char *harness_caseName(const struct HarnessSuite *suite, int index);
const char *harness_caseStem(const struct HarnessSuite *suite, int index);
enum HarnessVerdict harness_caseVerdict(const struct HarnessSuite *suite, int index);
int harness_caseMessageCount(const struct HarnessSuite *suite, int index);
const char *harness_caseMessage(const struct HarnessSuite *suite, int index, int message);
const char *harness_verdictName(enum HarnessVerdict verdict);

void harness_report(const struct HarnessSuite *suite, FILE *out);
int harness_exitCode(const struct HarnessSuite *suite);
const char *harness_statusLine(const struct HarnessSuite *suite);
void harness_setUpdateGoldens(struct HarnessSuite *suite, bool enabled);

#endif /* harness_h */
