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

#include "scenario.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *StateNames[] = {"NoProgram", "Evaluate", "Input", "Paused", "WaitForDisk", "End"};

static const char *HapticModeNames[] = {"None",  "Error", "Warning", "Success", "Heavy",
					"Light", "Medium", "Rigid",   "Soft",     "Selection"};

_Static_assert(sizeof(HapticModeNames) / sizeof(*HapticModeNames) == Selection + 1,
	"HapticModeNames is out of sync with enum HapticMode");
_Static_assert(sizeof(StateNames) / sizeof(*StateNames) == StateEnd + 1,
	"StateNames is out of sync with enum State");

const char *scenario_stateName(enum State state)
{
	if(state < StateNoProgram || state > StateEnd)
		return "?";
	return StateNames[state];
}

const char *scenario_hapticName(enum HapticMode mode)
{
	if(mode < None || mode > Selection)
		return "?";
	return HapticModeNames[mode];
}

const char *scenario_checkName(enum ScenarioCheckType type)
{
	switch(type)
	{
	case ScenarioCheckAssertions:
		return "assertions";

	case ScenarioCheckState:
		return "state";

	case ScenarioCheckPixels:
		return "pixels";

	case ScenarioCheckAudio:
		return "audio";

	case ScenarioCheckPortrait:
		return "portrait";

	case ScenarioCheckKeyboard:
		return "keyboard";

	case ScenarioCheckHaptic:
		return "haptic";
	}
	return "?";
}

// ================ Text helpers ================

static const char *skipSpace(const char *text)
{
	while(*text == ' ' || *text == '\t')
	{
		++text;
	}
	return text;
}

/** Copies the next whitespace delimited word, lowercased when requested. Returns the rest. */
static const char *readWord(const char *text, char *out, size_t outSize, bool lower)
{
	text = skipSpace(text);
	size_t i = 0;
	while(*text && *text != ' ' && *text != '\t')
	{
		if(i + 1 < outSize)
		{
			out[i++] = lower ? (char)tolower((unsigned char)*text) : *text;
		}
		++text;
	}
	out[i] = 0;
	return text;
}

/** Strips spaces and lowercases, so "Illegal Memory Access" and "illegalmemoryaccess" compare equal. */
static void normalize(const char *source, char *out, size_t outSize)
{
	size_t i = 0;
	for(; *source; ++source)
	{
		if(*source == ' ' || *source == '\t' || *source == '_' || *source == '-')
			continue;
		if(i + 1 < outSize)
		{
			out[i++] = (char)tolower((unsigned char)*source);
		}
	}
	out[i] = 0;
}

/** Reads exactly count numbers separated by commas and/or spaces, rejecting trailing junk. */
static bool readNumbers(const char *text, float *out, int count)
{
	for(int i = 0; i < count; ++i)
	{
		text = skipSpace(text);
		if(i > 0 && *text == ',')
		{
			text = skipSpace(text + 1);
		}
		char *end = NULL;
		out[i] = strtof(text, &end);
		if(end == text)
			return false;
		text = end;
	}
	return *skipSpace(text) == 0;
}

static bool readHash(const char *text, uint64_t *out)
{
	text = skipSpace(text);
	if(text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
	{
		text += 2;
	}
	if(!isxdigit((unsigned char)*text))
		return false;
	char *end = NULL;
	*out = strtoull(text, &end, 16);
	return *skipSpace(end) == 0;
}

bool scenario_parseErrorCode(const char *text, enum ErrorCode *outCode)
{
	text = skipSpace(text);
	if(!*text)
		return false;

	bool allDigits = true;
	for(const char *p = text; *p && *p != ' ' && *p != '\t'; ++p)
	{
		if(!isdigit((unsigned char)*p))
		{
			allDigits = false;
			break;
		}
	}
	if(allDigits)
	{
		int code = atoi(text);
		if(code <= ErrorNone || code >= ErrorMax)
			return false;
		*outCode = (enum ErrorCode)code;
		return true;
	}

	char wanted[SCENARIO_TEXT_SIZE];
	normalize(text, wanted, sizeof(wanted));
	for(int code = ErrorNone + 1; code < ErrorMax; ++code)
	{
		char candidate[SCENARIO_TEXT_SIZE];
		normalize(err_getString((enum ErrorCode)code), candidate, sizeof(candidate));
		if(strcmp(candidate, wanted) == 0)
		{
			*outCode = (enum ErrorCode)code;
			return true;
		}
	}
	return false;
}

// ================ Parsing ================

void scenario_init(struct Scenario *scenario)
{
	memset(scenario, 0, sizeof(struct Scenario));
	scenario->frames = SCENARIO_DEFAULT_FRAMES;
	scenario->runs = 1;
	scenario->width = SCENARIO_DEFAULT_WIDTH;
	scenario->height = SCENARIO_DEFAULT_HEIGHT;
}

static bool fail(struct Scenario *scenario, int line, const char *format, ...)
{
	if(!scenario->hasParseError)
	{
		scenario->hasParseError = true;
		scenario->parseErrorLine = line;
		va_list args;
		va_start(args, format);
		vsnprintf(scenario->parseError, sizeof(scenario->parseError), format, args);
		va_end(args);
	}
	return false;
}

static bool addAction(struct Scenario *scenario, int line, struct ScenarioAction action)
{
	if(scenario->numActions >= SCENARIO_MAX_ACTIONS)
		return fail(scenario, line, "too many '@at actions (max %d)", SCENARIO_MAX_ACTIONS);
	scenario->actions[scenario->numActions++] = action;
	return true;
}

static bool addCheck(struct Scenario *scenario, int line, struct ScenarioCheck check)
{
	if(scenario->numChecks >= SCENARIO_MAX_CHECKS)
		return fail(scenario, line, "too many '@check entries (max %d)", SCENARIO_MAX_CHECKS);
	check.line = line;
	scenario->checks[scenario->numChecks++] = check;
	return true;
}

static bool parseKey(struct Scenario *scenario, int line, const char *text, char *outKey)
{
	text = skipSpace(text);
	if(text[0] == '"' && text[1] && text[2] == '"' && *skipSpace(text + 3) == 0)
	{
		*outKey = text[1];
		return true;
	}

	char name[SCENARIO_TEXT_SIZE];
	normalize(text, name, sizeof(name));
	if(strcmp(name, "return") == 0)
		*outKey = CoreInputKeyReturn;
	else if(strcmp(name, "backspace") == 0)
		*outKey = CoreInputKeyBackspace;
	else if(strcmp(name, "left") == 0)
		*outKey = CoreInputKeyLeft;
	else if(strcmp(name, "right") == 0)
		*outKey = CoreInputKeyRight;
	else if(strcmp(name, "up") == 0)
		*outKey = CoreInputKeyUp;
	else if(strcmp(name, "down") == 0)
		*outKey = CoreInputKeyDown;
	else if(strcmp(name, "delete") == 0)
		*outKey = CoreInputKeyDelete;
	else
		return fail(scenario, line, "key wants a quoted character or RETURN BACKSPACE LEFT RIGHT UP DOWN DELETE, got %s",
			text);
	return true;
}

static bool parseAt(struct Scenario *scenario, int line, const char *text)
{
	char word[SCENARIO_TEXT_SIZE];

	text = skipSpace(text);
	char *end = NULL;
	long frameNumber = strtol(text, &end, 10);
	if(end == text || frameNumber < 1)
		return fail(scenario, line, "'@at wants a 1 based frame number");
	text = readWord(end, word, sizeof(word), true);

	struct ScenarioAction action = {0};
	action.frame = (int)frameNumber;

	if(strcmp(word, "touch") == 0 || strcmp(word, "move") == 0)
	{
		float xy[2];
		if(!readNumbers(text, xy, 2))
			return fail(scenario, line, "%s wants <x>,<y>", word);
		action.type = (word[0] == 't') ? ScenarioActionTouch : ScenarioActionMove;
		action.x = xy[0];
		action.y = xy[1];
	}
	else if(strcmp(word, "release") == 0)
	{
		if(*skipSpace(text) != 0)
			return fail(scenario, line, "release takes no argument");
		action.type = ScenarioActionRelease;
	}
	else if(strcmp(word, "key") == 0)
	{
		if(!parseKey(scenario, line, text, &action.key))
			return false;
		action.type = ScenarioActionKey;
	}
	else if(strcmp(word, "resize") == 0)
	{
		float wh[2];
		if(!readNumbers(text, wh, 2))
			return fail(scenario, line, "resize wants <w>,<h>");
		action.type = ScenarioActionResize;
		action.left = (int)wh[0];
		action.top = (int)wh[1];
	}
	else if(strcmp(word, "safe") == 0)
	{
		float ltrb[4];
		if(!readNumbers(text, ltrb, 4))
			return fail(scenario, line, "safe wants <l>,<t>,<r>,<b>");
		action.type = ScenarioActionSafe;
		action.left = (int)ltrb[0];
		action.top = (int)ltrb[1];
		action.right = (int)ltrb[2];
		action.bottom = (int)ltrb[3];
	}
	else if(strcmp(word, "keyboard") == 0)
	{
		char argument[SCENARIO_TEXT_SIZE];
		normalize(text, argument, sizeof(argument));
		if(strcmp(argument, "off") == 0)
		{
			action.type = ScenarioActionKeyboardOff;
		}
		else
		{
			float height[1];
			if(!readNumbers(text, height, 1))
				return fail(scenario, line, "keyboard wants <height> or off");
			action.type = ScenarioActionKeyboard;
			action.height = (int)height[0];
		}
	}
	else if(strcmp(word, "pause") == 0)
	{
		if(*skipSpace(text) != 0)
			return fail(scenario, line, "pause takes no argument");
		action.type = ScenarioActionPause;
	}
	else
	{
		return fail(scenario, line, "unknown '@at action %s", word);
	}

	return addAction(scenario, line, action);
}

static bool parseCheck(struct Scenario *scenario, int line, const char *text)
{
	char word[SCENARIO_TEXT_SIZE];

	text = skipSpace(text);
	char *end = NULL;
	long frameNumber = strtol(text, &end, 10);
	if(end == text || frameNumber < 1)
		return fail(scenario, line, "'@check wants a 1 based frame number");
	text = readWord(end, word, sizeof(word), true);

	struct ScenarioCheck check = {0};
	check.frame = (int)frameNumber;

	if(strcmp(word, "assertions") == 0 || strcmp(word, "portrait") == 0 || strcmp(word, "keyboard") == 0)
	{
		float value[1];
		if(!readNumbers(text, value, 1))
			return fail(scenario, line, "%s wants a number", word);
		check.type = (word[0] == 'a') ? ScenarioCheckAssertions
					      : ((word[0] == 'p') ? ScenarioCheckPortrait : ScenarioCheckKeyboard);
		check.value = (int)value[0];
	}
	else if(strcmp(word, "state") == 0)
	{
		char wanted[SCENARIO_TEXT_SIZE];
		normalize(text, wanted, sizeof(wanted));
		check.type = ScenarioCheckState;
		check.value = -1;
		for(int state = StateNoProgram; state <= StateEnd; ++state)
		{
			char candidate[SCENARIO_TEXT_SIZE];
			normalize(StateNames[state], candidate, sizeof(candidate));
			if(strcmp(candidate, wanted) == 0)
			{
				check.value = state;
				break;
			}
		}
		if(check.value < 0)
			return fail(scenario, line, "unknown state %s", skipSpace(text));
	}
	else if(strcmp(word, "haptic") == 0)
	{
		char wanted[SCENARIO_TEXT_SIZE];
		normalize(text, wanted, sizeof(wanted));
		check.type = ScenarioCheckHaptic;
		check.value = -1;
		for(int mode = None; mode <= Selection; ++mode)
		{
			char candidate[SCENARIO_TEXT_SIZE];
			normalize(HapticModeNames[mode], candidate, sizeof(candidate));
			if(strcmp(candidate, wanted) == 0)
			{
				check.value = mode;
				break;
			}
		}
		if(check.value < 0)
			return fail(scenario, line, "unknown haptic mode %s", skipSpace(text));
	}
	else if(strcmp(word, "pixels") == 0 || strcmp(word, "audio") == 0)
	{
		check.type = (word[0] == 'p') ? ScenarioCheckPixels : ScenarioCheckAudio;
		if(!readHash(text, &check.hash))
			return fail(scenario, line, "%s wants a 64 bit hex hash", word);
	}
	else
	{
		return fail(scenario, line, "unknown '@check subject %s", word);
	}

	return addCheck(scenario, line, check);
}

static bool parseDirective(struct Scenario *scenario, int line, const char *text)
{
	char word[SCENARIO_TEXT_SIZE];
	char name[SCENARIO_TEXT_SIZE];
	text = readWord(text, word, sizeof(word), false);
	// hyphens and case are irrelevant: allow-timeout, allowTimeout and ALLOWTIMEOUT are the same directive
	normalize(word, name, sizeof(name));

	if(strcmp(name, "frames") == 0)
	{
		float value[1];
		if(!readNumbers(text, value, 1) || value[0] < 1)
			return fail(scenario, line, "'@frames wants a positive frame count");
		scenario->frames = (int)value[0];
	}
	else if(strcmp(name, "runs") == 0)
	{
		float value[1];
		if(!readNumbers(text, value, 1) || value[0] < 1)
			return fail(scenario, line, "'@runs wants a positive run count");
		scenario->runs = (int)value[0];
	}
	else if(strcmp(name, "shown") == 0)
	{
		float wh[2];
		if(!readNumbers(text, wh, 2))
			return fail(scenario, line, "'@shown wants <w>,<h>");
		scenario->width = (int)wh[0];
		scenario->height = (int)wh[1];
	}
	else if(strcmp(name, "safe") == 0)
	{
		float ltrb[4];
		if(!readNumbers(text, ltrb, 4))
			return fail(scenario, line, "'@safe wants <l>,<t>,<r>,<b>");
		scenario->left = (int)ltrb[0];
		scenario->top = (int)ltrb[1];
		scenario->right = (int)ltrb[2];
		scenario->bottom = (int)ltrb[3];
	}
	else if(strcmp(name, "allowtimeout") == 0)
	{
		if(*skipSpace(text) != 0)
			return fail(scenario, line, "'@allow-timeout takes no argument");
		scenario->allowTimeout = true;
	}
	else if(strcmp(name, "expecterror") == 0)
	{
		if(!scenario_parseErrorCode(text, &scenario->expectedError))
			return fail(scenario, line, "unknown error code %s", skipSpace(text));
		scenario->hasExpectedError = true;
	}
	else if(strcmp(name, "expectcompileerror") == 0)
	{
		if(!scenario_parseErrorCode(text, &scenario->expectedCompileError))
			return fail(scenario, line, "unknown error code %s", skipSpace(text));
		scenario->hasExpectedCompileError = true;
	}
	else if(strcmp(name, "skip") == 0)
	{
		scenario->skip = true;
		snprintf(scenario->skipReason, sizeof(scenario->skipReason), "%s", skipSpace(text));
	}
	else if(strcmp(name, "at") == 0)
	{
		return parseAt(scenario, line, text);
	}
	else if(strcmp(name, "check") == 0)
	{
		return parseCheck(scenario, line, text);
	}
	else
	{
		return fail(scenario, line, "unknown directive '@%s", name);
	}
	return true;
}

bool scenario_parse(struct Scenario *scenario, const char *source)
{
	scenario_init(scenario);

	const char *cursor = source;
	int line = 1;
	while(*cursor)
	{
		const char *eol = cursor;
		while(*eol && *eol != '\n')
		{
			++eol;
		}

		// a ROM section header in column 0 ends the annotation block
		if(*cursor == '#')
			break;

		char text[SCENARIO_LINE_SIZE];
		size_t length = (size_t)(eol - cursor);
		if(length >= sizeof(text))
		{
			length = sizeof(text) - 1;
		}
		memcpy(text, cursor, length);
		text[length] = 0;
		while(length > 0 && (text[length - 1] == '\r' || text[length - 1] == ' ' || text[length - 1] == '\t'))
		{
			text[--length] = 0;
		}

		const char *body = skipSpace(text);
		if(body[0] == '\'' && body[1] == '@')
		{
			if(!parseDirective(scenario, line, body + 2))
				return false;
		}

		if(!*eol)
			break;
		cursor = eol + 1;
		++line;
	}

	return !scenario->hasParseError;
}
