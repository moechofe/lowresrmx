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

#include "cmd_particle.h"
#include "particles_lib.h"

#include "core.h"
#include "value.h"
#include "video_chip.h"

enum ErrorCode cmd_PARTICLE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct ParticlesLib *lib = &interpreter->particlesLib;

	// PARTICLE
	++interpreter->pc;

	// PARTICLE OFF
	if(interpreter->pc->type == TokenOFF)
	{
		++interpreter->pc;

		prtclib_clear(core, lib);
		return itp_endOfCommand(interpreter);
	}

	// PARTICLE <NUM>
	// PARTICLE <APPAREANCE>
	struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);

	if(interpreter->pc->type == TokenComma)
	{
		// PARTICLE <NUM>
		if(core->interpreter->pass == PassPrepare && nValue.type != ValueTypeFloat)
			return ErrorTypeMismatch;
		else if(core->interpreter->pass == PassRun &&
			((int)nValue.v.floatValue < 0 || (int)nValue.v.floatValue > NUM_SPRITES - 1))
			return ErrorInvalidParameter;

		++interpreter->pc;

		// PARTICLE <NUM>,<COUNT>
		struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES);
		if(cValue.type == ValueTypeError)
			return cValue.v.errorCode;

		// PARTICLE <NUM>,<COUNT> AT
		if(interpreter->pc->type != TokenAT)
			return ErrorSyntax;
		++interpreter->pc;

		// PARTICLE <NUM>,<COUNT> AT <ADDR>
		struct TypedValue aValue = itp_evaluateExpression(core, TypeClassNumeric);
		if(aValue.type == ValueTypeError)
			return aValue.v.errorCode;

		// TODO: check if last particle is out of RAM

		if(interpreter->pass == PassRun)
			prtclib_setupPool(lib, (int)nValue.v.floatValue, (int)cValue.v.floatValue, (int)aValue.v.floatValue);
	}
	else
		return ErrorSyntax;

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_EMITTER(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct ParticlesLib *lib = &interpreter->particlesLib;

	// EMITTER
	++interpreter->pc;

	// EMITTER <COUNT>
	// EMITTER <SPAWNER>
	struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);

	if(interpreter->pc->type == TokenAT)
	{
		// EMITTER <COUNT>
		if(core->interpreter->pass == PassPrepare && nValue.type != ValueTypeFloat)
			return ErrorTypeMismatch;
		else if(core->interpreter->pass == PassRun &&
			((int)nValue.v.floatValue < 0 || (int)nValue.v.floatValue > EMITTER_MAX - 1))
			return ErrorInvalidParameter;

		// EMITTER <COUNT> AT
		if(interpreter->pc->type != TokenAT)
			return ErrorSyntax;
		++interpreter->pc;

		// EMITTER <COUNT> AT <ADDR>
		struct TypedValue aValue = itp_evaluateExpression(core, TypeClassNumeric);
		if(aValue.type == ValueTypeError)
			return aValue.v.errorCode;

		// TODO: check if last particle is out of RAM

		if(interpreter->pass == PassRun)
			prtclib_setupEmitter(lib, (int)nValue.v.floatValue, (int)aValue.v.floatValue);
	}
	else if(interpreter->pc->type == TokenDATA)
	{
		// EMITTER <SPAWNER>
		if(core->interpreter->pass == PassPrepare && nValue.type != ValueTypeFloat)
			return ErrorTypeMismatch;
		else if(core->interpreter->pass == PassRun &&
			((int)nValue.v.floatValue < 0 || (int)nValue.v.floatValue > SPAWNER_MAX - 1))
			return ErrorInvalidParameter;

		// EMITTER <SPAWNER> DATA
		++interpreter->pc;

		// EMITTER <SPAWNER> DATA <LABEL>
		if(interpreter->pc->type != TokenIdentifier)
			return ErrorExpectedLabel;
		struct Token *tk = interpreter->pc;
		++interpreter->pc;

		if(interpreter->pass == PassPrepare)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);
			if(!item)
				return ErrorUndefinedLabel;
		}
		else if(interpreter->pass == PassRun)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tk->symbolIndex);

			struct Token *dataToken = dat_reachData(interpreter, item->token);

			prtclib_setSpawnerLabel(lib, (int)nValue.v.floatValue, dataToken);
		}
	}
	else if(interpreter->pc->type == TokenON)
	{
		// EMITTER <SPAWNER>
		if(core->interpreter->pass == PassPrepare && nValue.type != ValueTypeFloat)
			return ErrorTypeMismatch;
		else if(core->interpreter->pass == PassRun &&
			((int)nValue.v.floatValue < 0 || (int)nValue.v.floatValue > SPAWNER_MAX - 1))
			return ErrorInvalidParameter;

		// EMITTER <SPAWNER> ON
		++interpreter->pc;

		// EMITTER <SPAWNER> ON <X>
		struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
		if(xValue.type == ValueTypeError)
			return xValue.v.errorCode;

		// EMITTER <SPAWNER> ON <X>,
		if(interpreter->pc->type != TokenComma)
			return ErrorSyntax;
		++interpreter->pc;

		// EMITTER <SPAWNER> ON <X>,<Y>
		struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
		if(yValue.type == ValueTypeError)
			return yValue.v.errorCode;

		if(interpreter->pass == PassRun)
			prtclib_spawn(lib, (int)nValue.v.floatValue, xValue.v.floatValue, yValue.v.floatValue);
	}
	else if(interpreter->pc->type == TokenOFF)
	{
		// EMITTER <SPAWNER>
		if(core->interpreter->pass == PassPrepare && nValue.type != ValueTypeFloat)
			return ErrorTypeMismatch;
		else if(core->interpreter->pass == PassRun &&
			((int)nValue.v.floatValue < 0 || (int)nValue.v.floatValue > SPAWNER_MAX - 1))
			return ErrorInvalidParameter;

		// EMITTER <SPAWNER> OFF
		++interpreter->pc;

		if(interpreter->pass == PassRun)
			prtclib_stop(lib, (int)nValue.v.floatValue);
	}
	else
		return ErrorSyntax;

	return itp_endOfCommand(interpreter);
}
