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

#include "cmd_io.h"
#include "core.h"
#include "overlay_debugger.h"
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
		core->machine->ioRegisters.status.keyboardVisible = (type == TokenON);
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
		value.v.floatValue = core->machine->ioRegisters.keyboardHeight;
	}
	return value;
}

enum ErrorCode cmd_PAUSE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// PAUSE
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		trigger_debugger(core);
	}

	return itp_endOfCommand(interpreter);
}

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
