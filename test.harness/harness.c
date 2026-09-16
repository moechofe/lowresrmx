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
#include "data_manager.h"
#include "pcg_basic.h"
#include "string_utils.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

// same state machine_reset uses, but with a constant stream selector instead of a heap address
#define HARNESS_RNG_STATE 4715711917271117164
#define HARNESS_RNG_STREAM 1

// ================ Files ================

static char *readWholeFile(const char *path)
{
	FILE *file = fopen(path, "rb");
	if(!file)
		return NULL;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if(size < 0)
	{
		fclose(file);
		return NULL;
	}

	char *text = calloc(1, (size_t)size + 1);
	if(text && size > 0)
	{
		if(fread(text, 1, (size_t)size, file) != (size_t)size)
		{
			free(text);
			text = NULL;
		}
	}
	fclose(file);
	return text;
}

static bool hasSuffix(const char *text, const char *suffix)
{
	size_t textLength = strlen(text);
	size_t suffixLength = strlen(suffix);
	return textLength >= suffixLength && strcmp(text + textLength - suffixLength, suffix) == 0;
}

static void joinPath(char *out, size_t outSize, const char *directory, const char *name)
{
	size_t length = strlen(directory);
	if(length > 0 && (directory[length - 1] == '/' || directory[length - 1] == '\\'))
	{
		snprintf(out, outSize, "%s%s", directory, name);
	}
	else
	{
		snprintf(out, outSize, "%s/%s", directory, name);
	}
}

static void sandboxPath(const struct HarnessSuite *suite, char *out, size_t outSize, const char *suffix)
{
	char name[HARNESS_STEM_SIZE + 32];
	snprintf(name, sizeof(name), "%s%s", suite->cases[suite->caseIndex].stem, suffix);
	joinPath(out, outSize, suite->outDir, name);
}

// ================ Case list ================

static void makeStem(const char *path, char *out, size_t outSize)
{
	const char *base = path;
	for(const char *p = path; *p; ++p)
	{
		if(*p == '/' || *p == '\\')
		{
			base = p + 1;
		}
	}
	snprintf(out, outSize, "%s", base);
	char *dot = strrchr(out, '.');
	if(dot)
	{
		*dot = 0;
	}
}

static bool addCase(struct HarnessSuite *suite, const char *path, const char *name, const char *filter)
{
	if(filter && !strstr(path, filter))
		return true;
	if(suite->numCases >= HARNESS_MAX_CASES)
		return false;

	struct HarnessCase *item = &suite->cases[suite->numCases++];
	memset(item, 0, sizeof(struct HarnessCase));
	snprintf(item->path, sizeof(item->path), "%s", path);
	snprintf(item->name, sizeof(item->name), "%s", name);
	makeStem(path, item->stem, sizeof(item->stem));
	return true;
}

static bool isCaseFile(const char *name)
{
	return hasSuffix(name, ".rmx") || hasSuffix(name, ".nx");
}

static bool walkDirectory(struct HarnessSuite *suite, const char *directory, const char *relative, const char *filter);

static bool visitEntry(struct HarnessSuite *suite, const char *directory, const char *relative, const char *name,
	bool isDirectory, const char *filter)
{
	if(name[0] == '.')
		return true;

	char path[HARNESS_PATH_SIZE];
	char childRelative[HARNESS_PATH_SIZE];
	joinPath(path, sizeof(path), directory, name);
	if(relative[0])
	{
		joinPath(childRelative, sizeof(childRelative), relative, name);
	}
	else
	{
		snprintf(childRelative, sizeof(childRelative), "%s", name);
	}

	if(isDirectory)
		return walkDirectory(suite, path, childRelative, filter);
	if(isCaseFile(name))
		return addCase(suite, path, childRelative, filter);
	return true;
}

#ifdef _WIN32

static bool walkDirectory(struct HarnessSuite *suite, const char *directory, const char *relative, const char *filter)
{
	char pattern[HARNESS_PATH_SIZE];
	joinPath(pattern, sizeof(pattern), directory, "*");

	WIN32_FIND_DATAA found;
	HANDLE handle = FindFirstFileA(pattern, &found);
	if(handle == INVALID_HANDLE_VALUE)
		return false;

	bool okay = true;
	do
	{
		bool isDirectory = (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		okay = visitEntry(suite, directory, relative, found.cFileName, isDirectory, filter) && okay;
	}
	while(FindNextFileA(handle, &found));
	FindClose(handle);
	return okay;
}

static bool isDirectoryPath(const char *path)
{
	DWORD attributes = GetFileAttributesA(path);
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

#else

static bool walkDirectory(struct HarnessSuite *suite, const char *directory, const char *relative, const char *filter)
{
	DIR *handle = opendir(directory);
	if(!handle)
		return false;

	bool okay = true;
	struct dirent *entry;
	while((entry = readdir(handle)) != NULL)
	{
		char path[HARNESS_PATH_SIZE];
		joinPath(path, sizeof(path), directory, entry->d_name);
		struct stat info;
		if(stat(path, &info) != 0)
			continue;
		okay = visitEntry(suite, directory, relative, entry->d_name, S_ISDIR(info.st_mode), filter) && okay;
	}
	closedir(handle);
	return okay;
}

static bool isDirectoryPath(const char *path)
{
	struct stat info;
	return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

#endif

static int compareCases(const void *left, const void *right)
{
	const struct HarnessCase *a = left;
	const struct HarnessCase *b = right;
	return strcmp(a->name, b->name);
}

bool harness_suiteAddPath(struct HarnessSuite *suite, const char *path, const char *filter)
{
	int first = suite->numCases;
	bool okay;

	if(isDirectoryPath(path))
	{
		okay = walkDirectory(suite, path, "", filter);
	}
	else
	{
		const char *base = path;
		for(const char *p = path; *p; ++p)
		{
			if(*p == '/' || *p == '\\')
			{
				base = p + 1;
			}
		}
		okay = addCase(suite, path, base, filter);
	}

	if(suite->numCases > first)
	{
		qsort(&suite->cases[first], (size_t)(suite->numCases - first), sizeof(struct HarnessCase), compareCases);
	}
	return okay;
}

int harness_suiteCount(const struct HarnessSuite *suite)
{
	return suite->numCases;
}

// ================ Delegate ================

void harness_didFail(struct HarnessSuite *suite, struct CoreError error)
{
	if(!suite->hasRuntimeError)
	{
		suite->hasRuntimeError = true;
		suite->runtimeError = error;
	}
}

void harness_controlsDidChange(struct HarnessSuite *suite, struct ControlsInfo info)
{
	++suite->numControlsChanges;
	if(info.hapticMode >= None && info.hapticMode <= Selection)
	{
		suite->hapticSeen |= 1u << (unsigned)info.hapticMode;
	}
}

static void onInterpreterDidFail(void *context, struct CoreError error)
{
	harness_didFail(context, error);
}

static void onControlsDidChange(void *context, struct ControlsInfo info)
{
	harness_controlsDidChange(context, info);
}

static bool onDiskDriveWillAccess(void *context, struct DataManager *diskDataManager)
{
	struct HarnessSuite *suite = context;
	if(suite->caseIndex < 0)
		return true;

	char path[HARNESS_PATH_SIZE];
	sandboxPath(suite, path, sizeof(path), ".disk.rmx");
	char *text = readWholeFile(path);
	if(text)
	{
		data_import(diskDataManager, text, true);
		free(text);
	}
	return true;
}

static void onDiskDriveDidSave(void *context, struct DataManager *diskDataManager)
{
	struct HarnessSuite *suite = context;
	if(suite->caseIndex < 0)
		return;

	char *output = data_export(diskDataManager);
	if(output)
	{
		char path[HARNESS_PATH_SIZE];
		sandboxPath(suite, path, sizeof(path), ".disk.rmx");
		FILE *file = fopen(path, "wb");
		if(file)
		{
			fwrite(output, 1, strlen(output), file);
			fclose(file);
		}
		free(output);
	}
}

static void onDiskDriveIsFull(void *context, struct DataManager *diskDataManager)
{
	(void)context;
	(void)diskDataManager;
}

static void onPersistentRamWillAccess(void *context, uint8_t *destination, int size)
{
	struct HarnessSuite *suite = context;
	if(suite->caseIndex < 0)
		return;

	char path[HARNESS_PATH_SIZE];
	sandboxPath(suite, path, sizeof(path), ".dat");
	FILE *file = fopen(path, "rb");
	if(file)
	{
		fread(destination, 1, (size_t)size, file);
		fclose(file);
	}
}

static void onPersistentRamDidChange(void *context, uint8_t *data, int size)
{
	struct HarnessSuite *suite = context;
	if(suite->caseIndex < 0)
		return;

	char path[HARNESS_PATH_SIZE];
	sandboxPath(suite, path, sizeof(path), ".dat");
	FILE *file = fopen(path, "wb");
	if(file)
	{
		fwrite(data, 1, (size_t)size, file);
		fclose(file);
	}
}

// ================ Suite ================

void harness_suiteInit(struct HarnessSuite *suite, const char *outDir)
{
	memset(suite, 0, sizeof(struct HarnessSuite));
	snprintf(suite->outDir, sizeof(suite->outDir), "%s", outDir ? outDir : ".");
	suite->caseIndex = -1;

	suite->delegate.context = suite;
	suite->delegate.interpreterDidFail = onInterpreterDidFail;
	suite->delegate.diskDriveWillAccess = onDiskDriveWillAccess;
	suite->delegate.diskDriveDidSave = onDiskDriveDidSave;
	suite->delegate.diskDriveIsFull = onDiskDriveIsFull;
	suite->delegate.controlsDidChange = onControlsDidChange;
	suite->delegate.persistentRamWillAccess = onPersistentRamWillAccess;
	suite->delegate.persistentRamDidChange = onPersistentRamDidChange;
}

void harness_suiteDeinit(struct HarnessSuite *suite)
{
	free(suite->source);
	suite->source = NULL;
	suite->caseIndex = -1;
}

void harness_setUpdateGoldens(struct HarnessSuite *suite, bool enabled)
{
	suite->updateGoldens = enabled;
}

// ================ Messages ================

static void addMessage(struct HarnessSuite *suite, const char *format, ...)
{
	if(suite->caseIndex < 0)
		return;
	struct HarnessCase *item = &suite->cases[suite->caseIndex];
	if(item->numMessages >= HARNESS_MAX_MESSAGES)
		return;

	va_list args;
	va_start(args, format);
	vsnprintf(item->messages[item->numMessages], HARNESS_MESSAGE_SIZE, format, args);
	va_end(args);
	++item->numMessages;
}

/** Reports an engine error as "<text> at line <n>: <source line>". */
static void addErrorMessage(struct HarnessSuite *suite, const char *prefix, struct CoreError error)
{
	int line = (error.sourcePosition >= 0 && suite->source) ? lineNumber(suite->source, error.sourcePosition) : -1;
	const char *text = (error.sourcePosition >= 0 && suite->source) ? lineString(suite->source, error.sourcePosition)
									: NULL;
	if(line > 0 && text)
	{
		addMessage(suite, "%s%s at line %d: %s", prefix, err_getString(error.code), line, text);
	}
	else if(line > 0)
	{
		addMessage(suite, "%s%s at line %d", prefix, err_getString(error.code), line);
	}
	else
	{
		addMessage(suite, "%s%s", prefix, err_getString(error.code));
	}
	free((void *)text);
}

// ================ Running ================

static void applyShownSafe(struct HarnessSuite *suite, struct CoreInput *input)
{
	input->width = suite->width;
	input->height = suite->height;
	input->left = suite->left;
	input->top = suite->top;
	input->right = suite->right;
	input->bottom = suite->bottom;
}

static bool startRun(struct HarnessSuite *suite, struct Core *core)
{
	core_deinit(core);
	core_init(core);
	core_setDelegate(core, &suite->delegate);

	suite->frame = 0;
	suite->hasRuntimeError = false;
	suite->hasCompileError = false;
	suite->hapticSeen = 0;
	suite->numControlsChanges = 0;

	suite->compileError = core_compileProgram(core, suite->source, true);
	if(suite->compileError.code != ErrorNone)
	{
		suite->hasCompileError = true;
		suite->done = true;
		return false;
	}

	// machine_reset seeds defaultRng from a heap address, which is not reproducible across runs
	pcg32_srandom_r(&core->interpreter->defaultRng, HARNESS_RNG_STATE, HARNESS_RNG_STREAM);

	struct CoreInput priming;
	memset(&priming, 0, sizeof(priming));
	applyShownSafe(suite, &priming);
	core_handleInput(core, &priming);

	core_willRunProgram(core, 0);
	suite->runStarted = true;
	return true;
}

bool harness_beginCase(struct HarnessSuite *suite, struct Core *core, int index)
{
	suite->caseIndex = index;
	suite->run = 0;
	suite->done = false;
	suite->runStarted = false;
	suite->frame = 0;
	free(suite->source);
	suite->source = NULL;

	struct HarnessCase *item = &suite->cases[index];
	item->verdict = HarnessVerdictPending;
	item->failures = 0;
	item->numMessages = 0;
	item->numAssertions = 0;
	item->numFrames = 0;

	scenario_init(&suite->scenario);

	suite->source = readWholeFile(item->path);
	if(!suite->source)
	{
		suite->done = true;
		addMessage(suite, "could not read %s", item->path);
		item->verdict = HarnessVerdictError;
		return false;
	}

	if(!scenario_parse(&suite->scenario, suite->source))
	{
		suite->done = true;
		addMessage(suite, "line %d: %s", suite->scenario.parseErrorLine, suite->scenario.parseError);
		item->verdict = HarnessVerdictError;
		return false;
	}

	if(suite->scenario.skip)
	{
		suite->done = true;
		return false;
	}

	suite->width = suite->scenario.width;
	suite->height = suite->scenario.height;
	suite->left = suite->scenario.left;
	suite->top = suite->scenario.top;
	suite->right = suite->scenario.right;
	suite->bottom = suite->scenario.bottom;

	return startRun(suite, core);
}

void harness_beforeUpdate(struct HarnessSuite *suite, struct CoreInput *input)
{
	int frame = suite->frame + 1;
	for(int i = 0; i < suite->scenario.numActions; i++)
	{
		const struct ScenarioAction *action = &suite->scenario.actions[i];
		if(action->frame != frame)
			continue;

		switch(action->type)
		{
		case ScenarioActionTouch:
			input->touch = true;
			input->touchX = action->x;
			input->touchY = action->y;
			break;

		case ScenarioActionMove:
			input->touchX = action->x;
			input->touchY = action->y;
			break;

		case ScenarioActionRelease:
			input->touch = false;
			break;

		case ScenarioActionKey:
			input->key = action->key;
			break;

		case ScenarioActionResize:
			suite->width = action->left;
			suite->height = action->top;
			break;

		case ScenarioActionSafe:
			suite->left = action->left;
			suite->top = action->top;
			suite->right = action->right;
			suite->bottom = action->bottom;
			break;

		case ScenarioActionKeyboard:
			input->keyboardChange = 1;
			input->keyboardHeight = action->height;
			break;

		case ScenarioActionKeyboardOff:
			input->keyboardChange = -1;
			input->keyboardHeight = 0;
			break;

		case ScenarioActionPause:
			input->pause = true;
			break;
		}
	}

	applyShownSafe(suite, input);
}

static void evaluateChecks(struct HarnessSuite *suite, struct Core *core)
{
	struct HarnessCase *item = &suite->cases[suite->caseIndex];

	for(int i = 0; i < suite->scenario.numChecks; i++)
	{
		struct ScenarioCheck *check = &suite->scenario.checks[i];
		if(check->frame != suite->frame)
			continue;
		check->evaluated = true;

		switch(check->type)
		{
		case ScenarioCheckAssertions:
		{
			int actual = core->interpreter->numAssertions;
			if(actual != check->value)
			{
				addMessage(suite, "frame %d assertions %d != %d", suite->frame, actual, check->value);
				++item->failures;
			}
			break;
		}

		case ScenarioCheckState:
		{
			int actual = (int)core->interpreter->state;
			if(actual != check->value)
			{
				addMessage(suite, "frame %d state %s != %s", suite->frame,
					scenario_stateName((enum State)actual), scenario_stateName((enum State)check->value));
				++item->failures;
			}
			break;
		}

		case ScenarioCheckPortrait:
		{
			int actual = core->interpreter->lockPortrait ? 1 : 0;
			if(actual != check->value)
			{
				addMessage(suite, "frame %d portrait %d != %d", suite->frame, actual, check->value);
				++item->failures;
			}
			break;
		}

		case ScenarioCheckKeyboard:
		{
			int actual = core_isKeyboardEnabled(core) ? 1 : 0;
			if(actual != check->value)
			{
				addMessage(suite, "frame %d keyboard %d != %d", suite->frame, actual, check->value);
				++item->failures;
			}
			break;
		}

		case ScenarioCheckHaptic:
		{
			bool seen = (suite->hapticSeen & (1u << (unsigned)check->value)) != 0;
			if(!seen)
			{
				addMessage(suite, "frame %d haptic %s never fired", suite->frame,
					scenario_hapticName((enum HapticMode)check->value));
				++item->failures;
			}
			suite->hapticSeen = 0;
			break;
		}

		case ScenarioCheckPixels:
		case ScenarioCheckAudio:
		{
			uint64_t actual = (check->type == ScenarioCheckPixels) ? golden_hashFrame(core, suite->pixels)
									      : golden_hashAudio(core, suite->samples);
			check->actual = actual;
			check->hasActual = true;
			bool mismatch = (actual != check->hash);
			if(mismatch && !suite->updateGoldens)
			{
				addMessage(suite, "frame %d %s 0x%016llx != 0x%016llx", suite->frame,
					scenario_checkName(check->type), (unsigned long long)actual,
					(unsigned long long)check->hash);
				++item->failures;
			}
			if(check->type == ScenarioCheckPixels && (mismatch || suite->updateGoldens))
			{
				char suffix[32];
				char path[HARNESS_PATH_SIZE];
				snprintf(suffix, sizeof(suffix), ".frame%d.actual.png", suite->frame);
				sandboxPath(suite, path, sizeof(path), suffix);
				if(golden_writePng(path, suite->pixels))
				{
					addMessage(suite, "wrote %s", path);
				}
			}
			break;
		}
		}
	}

	item->numAssertions = core->interpreter->numAssertions;
	item->numFrames = suite->frame;
}

void harness_afterUpdate(struct HarnessSuite *suite, struct Core *core, struct CoreInput *input)
{
	++suite->frame;

	bool isLastRun = (suite->run + 1 >= suite->scenario.runs);
	if(isLastRun)
	{
		evaluateChecks(suite, core);
	}
	else
	{
		suite->cases[suite->caseIndex].numFrames = suite->frame;
	}

	// the engine clears input->key itself (core_handleInput), these two it does not
	input->keyboardChange = 0;
	input->pause = false;

	bool runEnded = suite->hasRuntimeError || core->interpreter->state == StateEnd ||
		suite->frame >= suite->scenario.frames;
	if(!runEnded)
		return;

	bool unexpectedError = suite->hasRuntimeError &&
		!(suite->scenario.hasExpectedError && suite->runtimeError.code == suite->scenario.expectedError);
	if(isLastRun || unexpectedError)
	{
		suite->done = true;
		return;
	}

	// hand the persistent RAM of this run to the next one
	core_willSuspendProgram(core);
	++suite->run;
	startRun(suite, core);
}

bool harness_caseDone(const struct HarnessSuite *suite)
{
	return suite->done;
}

// ================ Verdict ================

static void rewriteGoldens(struct HarnessSuite *suite)
{
	struct HarnessCase *item = &suite->cases[suite->caseIndex];
	char *text = readWholeFile(item->path);
	if(!text)
		return;

	FILE *file = fopen(item->path, "wb");
	if(!file)
	{
		free(text);
		return;
	}

	const char *cursor = text;
	int line = 1;
	while(*cursor)
	{
		const char *eol = cursor;
		while(*eol && *eol != '\n')
		{
			++eol;
		}

		const struct ScenarioCheck *update = NULL;
		for(int i = 0; i < suite->scenario.numChecks; i++)
		{
			const struct ScenarioCheck *check = &suite->scenario.checks[i];
			if(check->hasActual && check->line == line)
			{
				update = check;
			}
		}

		if(update)
		{
			// replace the trailing hash token only, keeping the rest of the directive verbatim
			const char *lineEnd = eol;
			if(lineEnd > cursor && lineEnd[-1] == '\r')
			{
				--lineEnd;
			}
			const char *tail = lineEnd;
			while(tail > cursor && tail[-1] != ' ' && tail[-1] != '\t')
			{
				--tail;
			}
			fwrite(cursor, 1, (size_t)(tail - cursor), file);
			fprintf(file, "0x%016llx", (unsigned long long)update->actual);
			fwrite(lineEnd, 1, (size_t)(eol - lineEnd), file);
		}
		else
		{
			fwrite(cursor, 1, (size_t)(eol - cursor), file);
		}

		if(!*eol)
			break;
		fputc('\n', file);
		cursor = eol + 1;
		++line;
	}

	fclose(file);
	free(text);
}

static bool hasMismatch(const struct HarnessSuite *suite)
{
	return suite->cases[suite->caseIndex].failures > 0;
}

static void decideVerdict(struct HarnessSuite *suite, struct Core *core)
{
	struct HarnessCase *item = &suite->cases[suite->caseIndex];
	const struct Scenario *scenario = &suite->scenario;

	if(item->verdict != HarnessVerdictPending)
		return;

	if(scenario->skip)
	{
		item->verdict = HarnessVerdictSkip;
		addMessage(suite, "%s", scenario->skipReason[0] ? scenario->skipReason : "skipped");
		return;
	}

	if(suite->hasCompileError)
	{
		if(scenario->hasExpectedCompileError && suite->compileError.code == scenario->expectedCompileError)
		{
			item->verdict = HarnessVerdictPass;
			addErrorMessage(suite, "expected ", suite->compileError);
		}
		else
		{
			item->verdict = HarnessVerdictFail;
			addErrorMessage(suite, "", suite->compileError);
		}
		return;
	}

	if(scenario->hasExpectedCompileError)
	{
		item->verdict = HarnessVerdictFail;
		addMessage(suite, "expected compile error %s, but the program compiled",
			err_getString(scenario->expectedCompileError));
		return;
	}

	if(hasMismatch(suite))
	{
		item->verdict = HarnessVerdictFail;
		return;
	}

	if(suite->hasRuntimeError)
	{
		if(scenario->hasExpectedError && suite->runtimeError.code == scenario->expectedError)
		{
			item->verdict = HarnessVerdictPass;
			addErrorMessage(suite, "expected ", suite->runtimeError);
		}
		else
		{
			item->verdict = HarnessVerdictFail;
			addErrorMessage(suite, "", suite->runtimeError);
		}
		return;
	}

	if(scenario->hasExpectedError)
	{
		item->verdict = HarnessVerdictFail;
		addMessage(suite, "expected %s, but the program ended cleanly", err_getString(scenario->expectedError));
		return;
	}

	if(core->interpreter->state != StateEnd)
	{
		if(scenario->allowTimeout)
		{
			item->verdict = HarnessVerdictPass;
		}
		else
		{
			item->verdict = HarnessVerdictTimeout;
			addMessage(suite, "still %s after %d frames", scenario_stateName(core->interpreter->state),
				suite->frame);
		}
		return;
	}

	for(int i = 0; i < scenario->numChecks; i++)
	{
		if(!scenario->checks[i].evaluated)
		{
			addMessage(suite, "'@check at line %d never ran, frame %d was not reached",
				scenario->checks[i].line, scenario->checks[i].frame);
			++item->failures;
		}
	}
	if(item->failures > 0)
	{
		item->verdict = HarnessVerdictFail;
		return;
	}

	if(item->numAssertions == 0 && scenario->numChecks == 0)
	{
		item->verdict = HarnessVerdictFail;
		addMessage(suite, "no assertions");
		return;
	}

	item->verdict = HarnessVerdictPass;
}

const char *harness_verdictName(enum HarnessVerdict verdict)
{
	switch(verdict)
	{
	case HarnessVerdictPass:
		return "PASS";

	case HarnessVerdictFail:
		return "FAIL";

	case HarnessVerdictTimeout:
		return "TIMEOUT";

	case HarnessVerdictSkip:
		return "SKIP";

	case HarnessVerdictError:
		return "ERROR";

	case HarnessVerdictPending:
		return "PENDING";
	}
	return "?";
}

void harness_endCase(struct HarnessSuite *suite, struct Core *core)
{
	if(suite->caseIndex < 0)
		return;

	if(suite->runStarted)
	{
		core_willSuspendProgram(core);
	}

	decideVerdict(suite, core);

	struct HarnessCase *item = &suite->cases[suite->caseIndex];
	switch(item->verdict)
	{
	case HarnessVerdictPass:
		++suite->numPassed;
		break;

	case HarnessVerdictFail:
		++suite->numFailed;
		break;

	case HarnessVerdictTimeout:
		++suite->numTimeout;
		break;

	case HarnessVerdictSkip:
		++suite->numSkipped;
		break;

	default:
		++suite->numErrors;
		break;
	}

	bool hasGolden = false;
	for(int i = 0; i < suite->scenario.numChecks; i++)
	{
		hasGolden = hasGolden || suite->scenario.checks[i].hasActual;
	}
	if(suite->updateGoldens && hasGolden)
	{
		rewriteGoldens(suite);
	}

	snprintf(suite->statusLine, sizeof(suite->statusLine), "%s %s", harness_verdictName(item->verdict), item->stem);

	free(suite->source);
	suite->source = NULL;
	suite->caseIndex = -1;
}

// ================ Report ================

void harness_report(const struct HarnessSuite *suite, FILE *out)
{
	for(int i = 0; i < suite->numCases; i++)
	{
		const struct HarnessCase *item = &suite->cases[i];
		const char *verdict = harness_verdictName(item->verdict);

		if(item->verdict == HarnessVerdictPass && item->numMessages == 0)
		{
			fprintf(out, "%-8s %s (%d assertions, %d frames)\n", verdict, item->name, item->numAssertions,
				item->numFrames);
			continue;
		}

		if(item->numMessages == 0)
		{
			fprintf(out, "%-8s %s\n", verdict, item->name);
			continue;
		}

		if(item->verdict == HarnessVerdictPass)
		{
			fprintf(out, "%-8s %s (%s)\n", verdict, item->name, item->messages[0]);
		}
		else
		{
			fprintf(out, "%-8s %s: %s\n", verdict, item->name, item->messages[0]);
		}

		int indent = 9 + (int)strlen(item->name) + 2;
		for(int m = 1; m < item->numMessages; m++)
		{
			fprintf(out, "%*s%s\n", indent, "", item->messages[m]);
		}
	}

	fprintf(out, "\n%d passed, %d failed, %d timeout, %d skipped", suite->numPassed, suite->numFailed,
		suite->numTimeout, suite->numSkipped);
	if(suite->numErrors > 0)
	{
		fprintf(out, ", %d errors", suite->numErrors);
	}
	fprintf(out, "\n");
}

int harness_exitCode(const struct HarnessSuite *suite)
{
	return (suite->numFailed + suite->numTimeout + suite->numErrors) > 0 ? 1 : 0;
}

const char *harness_statusLine(const struct HarnessSuite *suite)
{
	return suite->statusLine;
}

static const struct HarnessCase *caseAt(const struct HarnessSuite *suite, int index)
{
	return (index >= 0 && index < suite->numCases) ? &suite->cases[index] : NULL;
}

const char *harness_caseName(const struct HarnessSuite *suite, int index)
{
	const struct HarnessCase *item = caseAt(suite, index);
	return item ? item->name : "";
}

const char *harness_caseStem(const struct HarnessSuite *suite, int index)
{
	const struct HarnessCase *item = caseAt(suite, index);
	return item ? item->stem : "";
}

enum HarnessVerdict harness_caseVerdict(const struct HarnessSuite *suite, int index)
{
	const struct HarnessCase *item = caseAt(suite, index);
	return item ? item->verdict : HarnessVerdictPending;
}

int harness_caseMessageCount(const struct HarnessSuite *suite, int index)
{
	const struct HarnessCase *item = caseAt(suite, index);
	return item ? item->numMessages : 0;
}

const char *harness_caseMessage(const struct HarnessSuite *suite, int index, int message)
{
	const struct HarnessCase *item = caseAt(suite, index);
	if(!item || message < 0 || message >= item->numMessages)
		return "";
	return item->messages[message];
}
