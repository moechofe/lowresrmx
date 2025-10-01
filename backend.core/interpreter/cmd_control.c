//
// Copyright 2017-2020 Timo Kloss
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

#include "cmd_control.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_END(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// END
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		itp_endProgram(core);
		return ErrorNone;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_IF(struct Core *core, bool isAfterBlockElse)
{
	struct Interpreter *interpreter = core->interpreter;

	// IF
	struct Token *tokenIF = interpreter->pc;
	++interpreter->pc;

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	// THEN
	if (interpreter->pc->type != TokenTHEN)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		if (interpreter->pc->type == TokenEol)
		{
			// IF block
			if (interpreter->isSingleLineIf)
				return ErrorExpectedCommand;
			enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, isAfterBlockElse ? LabelTypeELSEIF : LabelTypeIF, tokenIF);
			if (errorCode != ErrorNone)
				return errorCode;

			// Eol
			++interpreter->pc;
		}
		else
		{
			// single line IF
			interpreter->isSingleLineIf = true;
			struct Token *token = interpreter->pc;
			while (token->type != TokenEol && token->type != TokenELSE)
			{
				token++;
			}
			tokenIF->jumpToken = token + 1; // after ELSE or Eol
		}
	}
	else if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenIF->jumpToken; // after ELSE or END IF, or Eol for single line
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_ELSE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ELSE
	struct Token *tokenELSE = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		if (interpreter->isSingleLineIf)
		{
			if (interpreter->pc->type == TokenEol)
				return ErrorExpectedCommand;
			struct Token *token = interpreter->pc;
			while (token->type != TokenEol)
			{
				token++;
			}
			tokenELSE->jumpToken = token + 1; // after Eol
		}
		else
		{
			struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
			if (!item)
				return ErrorElseWithoutIf;
			if (item->type == LabelTypeIF)
			{
				item->token->jumpToken = interpreter->pc;
			}
			else if (item->type == LabelTypeELSEIF)
			{
				item->token->jumpToken = interpreter->pc;

				item = lab_popLabelStackItem(interpreter);
				assert(item->type == LabelTypeELSE);
				item->token->jumpToken = tokenELSE;
			}
			else
			{
				return ErrorElseWithoutIf;
			}

			enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeELSE, tokenELSE);
			if (errorCode != ErrorNone)
				return errorCode;

			if (interpreter->pc->type == TokenIF)
			{
				return cmd_IF(core, true);
			}
			else
			{
				// Eol
				if (interpreter->pc->type != TokenEol)
					return ErrorSyntax;
				++interpreter->pc;
			}
		}
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenELSE->jumpToken; // after END IF, or Eol for single line
	}
	return ErrorNone;
}

enum ErrorCode cmd_END_IF(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// END IF
	++interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item)
		{
			return ErrorEndIfWithoutIf;
		}
		else if (item->type == LabelTypeIF || item->type == LabelTypeELSE)
		{
			item->token->jumpToken = interpreter->pc;
		}
		else if (item->type == LabelTypeELSEIF)
		{
			item->token->jumpToken = interpreter->pc;

			item = lab_popLabelStackItem(interpreter);
			assert(item->type == LabelTypeELSE);
			item->token->jumpToken = interpreter->pc;
		}
		else
		{
			return ErrorEndIfWithoutIf;
		}
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_FOR(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// FOR
	struct Token *tokenFOR = interpreter->pc;
	++interpreter->pc;

	// Variable
	struct Token *tokenFORVar = interpreter->pc;
	enum ErrorCode errorCode = ErrorNone;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeFloat)
		return ErrorTypeMismatch;

	// Eq
	if (interpreter->pc->type != TokenEq)
		return ErrorSyntax;
	++interpreter->pc;

	// start value
	struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (startValue.type == ValueTypeError)
		return startValue.v.errorCode;

	// TO
	if (interpreter->pc->type != TokenTO)
		return ErrorSyntax;
	++interpreter->pc;

	// limit value
	struct Token *tokenFORLimit = interpreter->pc;
	struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (limitValue.type == ValueTypeError)
		return limitValue.v.errorCode;

	// STEP
	struct TypedValue stepValue;
	if (interpreter->pc->type == TokenSTEP)
	{
		++interpreter->pc;

		// step value
		stepValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (stepValue.type == ValueTypeError)
			return stepValue.v.errorCode;
	}
	else
	{
		stepValue.type = ValueTypeFloat;
		stepValue.v.floatValue = 1.0f;
	}

	if (interpreter->pass == PassPrepare)
	{
		lab_pushLabelStackItem(interpreter, LabelTypeFORLimit, tokenFORLimit);
		lab_pushLabelStackItem(interpreter, LabelTypeFORVar, tokenFORVar);
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeFOR, tokenFOR);
		if (errorCode != ErrorNone)
			return errorCode;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		varValue->floatValue = startValue.v.floatValue;

		// limit check
		if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
		{
			interpreter->pc = tokenFOR->jumpToken; // after NEXT's Eol
		}
		else
		{
			// Eol
			if (interpreter->pc->type != TokenEol)
				return ErrorSyntax;
			++interpreter->pc;
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_NEXT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;
	struct LabelStackItem *itemFORLimit = NULL;
	struct LabelStackItem *itemFORVar = NULL;
	struct LabelStackItem *itemFOR = NULL;

	if (interpreter->pass == PassPrepare)
	{
		itemFOR = lab_popLabelStackItem(interpreter);
		if (!itemFOR || itemFOR->type != LabelTypeFOR)
			return ErrorNextWithoutFor;

		itemFORVar = lab_popLabelStackItem(interpreter);
		assert(itemFORVar && itemFORVar->type == LabelTypeFORVar);

		itemFORLimit = lab_popLabelStackItem(interpreter);
		assert(itemFORLimit && itemFORLimit->type == LabelTypeFORLimit);
	}

	// NEXT
	struct Token *tokenNEXT = interpreter->pc;
	++interpreter->pc;

	// Variable
	enum ErrorCode errorCode = ErrorNone;
	struct Token *tokenVar = interpreter->pc;
	enum ValueType valueType = ValueTypeNull;
	union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
	if (!varValue)
		return errorCode;
	if (valueType != ValueTypeFloat)
		return ErrorTypeMismatch;

	if (interpreter->pass == PassPrepare)
	{
		if (tokenVar->symbolIndex != itemFORVar->token->symbolIndex)
			return ErrorNextWithoutFor;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		itemFOR->token->jumpToken = interpreter->pc;
		tokenNEXT->jumpToken = itemFORLimit->token;
	}
	else if (interpreter->pass == PassRun)
	{
		struct Token *storedPc = interpreter->pc;
		interpreter->pc = tokenNEXT->jumpToken;

		// limit value
		struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
		if (limitValue.type == ValueTypeError)
			return limitValue.v.errorCode;

		// STEP
		struct TypedValue stepValue;
		if (interpreter->pc->type == TokenSTEP)
		{
			++interpreter->pc;

			// step value
			stepValue = itp_evaluateExpression(core, TypeClassNumeric);
			if (stepValue.type == ValueTypeError)
				return stepValue.v.errorCode;
		}
		else
		{
			stepValue.type = ValueTypeFloat;
			stepValue.v.floatValue = 1.0f;
		}

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;

		varValue->floatValue += stepValue.v.floatValue;

		// limit check
		if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
		{
			interpreter->pc = storedPc; // after NEXT's Eol
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_GOTO(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// GOTO
	struct Token *tokenGOTO = interpreter->pc;
	++interpreter->pc;

	// Identifier
	if (interpreter->pc->type != TokenIdentifier)
		return ErrorExpectedLabel;
	struct Token *tokenIdentifier = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
		if (!item)
			return ErrorUndefinedLabel;
		tokenGOTO->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenGOTO->jumpToken; // after label
	}
	return ErrorNone;
}

enum ErrorCode cmd_GOSUB(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// GOSUB
	struct Token *tokenGOSUB = interpreter->pc;
	++interpreter->pc;

	// Identifier
	if (interpreter->pc->type != TokenIdentifier)
		return ErrorExpectedLabel;
	struct Token *tokenIdentifier = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
		if (!item)
			return ErrorUndefinedLabel;
		tokenGOSUB->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeGOSUB, interpreter->pc);
		if (errorCode != ErrorNone)
			return errorCode;

		interpreter->pc = tokenGOSUB->jumpToken; // after label
	}
	return ErrorNone;
}

enum ErrorCode cmd_RETURN(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// RETURN
	struct Token *tokenRETURN = interpreter->pc;
	++interpreter->pc;

	// Identifier
	struct Token *tokenIdentifier = NULL;
	if (interpreter->pc->type == TokenIdentifier)
	{
		tokenIdentifier = interpreter->pc;
		++interpreter->pc;
	}

	if (interpreter->pass == PassPrepare)
	{
		if (tokenIdentifier)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenRETURN->jumpToken = item->token;
		}
	}
	else if (interpreter->pass == PassRun)
	{
		struct LabelStackItem *itemGOSUB = lab_popLabelStackItem(interpreter);
		if (!itemGOSUB)
			return ErrorReturnWithoutGosub;

		if (itemGOSUB->type == LabelTypeGOSUB)
		{
			if (tokenRETURN->jumpToken)
			{
				// jump to label
				interpreter->pc = tokenRETURN->jumpToken; // after label
				// clear stack
				interpreter->numLabelStackItems = 0;
			}
			else
			{
				// jump back
				interpreter->pc = itemGOSUB->token; // after GOSUB
			}
		}
		else
		{
			return ErrorReturnWithoutGosub;
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_WAIT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
		return ErrorNotAllowedInInterrupt;

	// WAIT
	++interpreter->pc;

	int wait = 0;
	if (interpreter->pc->type == TokenVBL)
	{
		// VBL
		++interpreter->pc;
	}
	else if (interpreter->pc->type == TokenTAP)
	{
		++interpreter->pc;
		if (interpreter->pass == PassRun)
		{
			interpreter->exitEvaluation = true;
			interpreter->waitTap = true;
		}
	}
	else
	{
		// value
		struct TypedValue value = itp_evaluateNumericExpression(core, 1, VM_MAX);
		if (value.type == ValueTypeError)
			return value.v.errorCode;
		wait = value.v.floatValue - 1;
	}

	if (interpreter->pass == PassRun)
	{
		interpreter->exitEvaluation = true;
		interpreter->waitCount = wait;
		if (interpreter->pauseAtWait) interpreter->state = StatePaused;
		interpreter->pauseAtWait = false;
	}
	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ON(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// ON
	++interpreter->pc;

	// RASTER/VBL/PARTICLE/EMITTER
	if (interpreter->pc->type == TokenRASTER || interpreter->pc->type == TokenVBL || interpreter->pc->type == TokenPARTICLE || interpreter->pc->type == TokenEMITTER)
	{
		enum TokenType type = interpreter->pc->type;
		++interpreter->pc;

		if (interpreter->pc->type == TokenOFF)
		{
			// OFF
			++interpreter->pc;

			if (interpreter->pass == PassRun)
			{
				if (type == TokenRASTER)
				{
					interpreter->currentOnRasterToken = NULL;
				}
				else if (type == TokenVBL)
				{
					interpreter->currentOnVBLToken = NULL;
				}
				else if (type == TokenPARTICLE)
				{
					interpreter->currentOnParticleToken = NULL;
				}
				else if (type == TokenEMITTER)
				{
					interpreter->currentOnEmitterToken = NULL;
				}
			}
		}
		else if (interpreter->pc->type == TokenCALL)
		{
			// CALL
			// if (interpreter->pc->type != TokenCALL) return ErrorSyntax;
			struct Token *tokenCALL = interpreter->pc;
			++interpreter->pc;

			// Identifier
			if (interpreter->pc->type != TokenIdentifier)
				return ErrorExpectedSubprogramName;
			struct Token *tokenIdentifier = interpreter->pc;
			++interpreter->pc;

			if (interpreter->pass == PassPrepare)
			{
				struct SubItem *item = tok_getSub(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
				if (!item)
					return ErrorUndefinedSubprogram;
				tokenCALL->jumpToken = item->token;
			}
			else if (interpreter->pass == PassRun)
			{
				if (type == TokenRASTER)
				{
					interpreter->currentOnRasterToken = tokenCALL->jumpToken;
				}
				else if (type == TokenVBL)
				{
					interpreter->currentOnVBLToken = tokenCALL->jumpToken;
				}
				else if (type == TokenPARTICLE)
				{
					interpreter->currentOnParticleToken = tokenCALL->jumpToken;
				}
				else if (type == TokenEMITTER)
				{
					interpreter->currentOnEmitterToken = tokenCALL->jumpToken;
				}
			}
		}
	}
	else
	{
		// n value
		struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 255);
		if (nValue.type == ValueTypeError)
			return nValue.v.errorCode;
		int n = nValue.v.floatValue;

		struct Token *tokenGOTO = NULL;
		struct Token *tokenGOSUB = NULL;
		struct Token *tokenRESTORE = NULL;

		if (interpreter->pc->type == TokenGOTO)
		{
			tokenGOTO = interpreter->pc;
			++interpreter->pc;
		}
		else if (interpreter->pc->type == TokenGOSUB)
		{
			tokenGOSUB = interpreter->pc;
			++interpreter->pc;
		}
		else if (interpreter->pc->type == TokenRESTORE)
		{
			tokenRESTORE = interpreter->pc;
			++interpreter->pc;
		}
		else
		{
			return ErrorSyntax;
		}

		if (interpreter->pass == PassRun)
			while (n > 0)
			{
				++interpreter->pc;
				if (interpreter->pc->type != TokenComma)
					return ErrorExpectedLabel;
				++interpreter->pc;
				--n;
			}

		// Identifier
		if (interpreter->pc->type != TokenIdentifier)
			return ErrorExpectedLabel;
		struct Token *tokenIdentifier = interpreter->pc;
		++interpreter->pc;

		while (interpreter->pc->type == TokenComma)
		{
			++interpreter->pc;
			if (interpreter->pc->type != TokenIdentifier)
				return ErrorSyntax;
			++interpreter->pc;
		}

		if (tokenGOTO)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenGOTO->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				interpreter->pc = tokenGOTO->jumpToken; // after label
			}
		}
		else if (tokenGOSUB)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenGOSUB->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeGOSUB, interpreter->pc);
				if (errorCode != ErrorNone)
					return errorCode;

				interpreter->pc = tokenGOSUB->jumpToken; // after label
			}
		}
		else if (tokenRESTORE)
		{
			struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
			if (!item)
				return ErrorUndefinedLabel;
			tokenRESTORE->jumpToken = item->token;
			if (interpreter->pass == PassRun)
			{
				dat_restoreData(interpreter, tokenRESTORE->jumpToken);
			}
		}
		else
			return ErrorNone;
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DO(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// DO
	struct Token *tokenDO = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeDO, tokenDO);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_LOOP(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// LOOP
	struct Token *tokenLOOP = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeDO)
			return ErrorLoopWithoutDo;

		tokenLOOP->jumpToken = item->token + 1;
		item->token->jumpToken = tokenLOOP + 1;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenLOOP->jumpToken; // after DO
	}

	return ErrorNone;
}

enum ErrorCode cmd_REPEAT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// REPEAT
	struct Token *tokenREPEAT = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeREPEAT, tokenREPEAT);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	return ErrorNone;
}

enum ErrorCode cmd_UNTIL(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// UNTIL
	struct Token *tokenUNTIL = interpreter->pc;
	++interpreter->pc;

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeREPEAT)
			return ErrorUntilWithoutRepeat;

		tokenUNTIL->jumpToken = item->token + 1;
		item->token->jumpToken = interpreter->pc;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenUNTIL->jumpToken; // after REPEAT
		}
		else
		{
			// Eol
			if (interpreter->pc->type != TokenEol)
				return ErrorSyntax;
			++interpreter->pc;
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_WHILE(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// WHILE
	struct Token *tokenWHILE = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeWHILE, tokenWHILE);
		if (errorCode != ErrorNone)
			return errorCode;
	}

	// Expression
	struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
	if (value.type == ValueTypeError)
		return value.v.errorCode;

	// Eol
	if (interpreter->pc->type != TokenEol)
		return ErrorSyntax;
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		if (value.v.floatValue == 0)
		{
			interpreter->pc = tokenWHILE->jumpToken; // after WEND
		}
	}

	return ErrorNone;
}

enum ErrorCode cmd_WEND(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// WEND
	struct Token *tokenWEND = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
		if (!item || item->type != LabelTypeWHILE)
			return ErrorWendWithoutWhile;

		tokenWEND->jumpToken = item->token;
		item->token->jumpToken = tokenWEND + 1;

		// Eol
		if (interpreter->pc->type != TokenEol)
			return ErrorSyntax;
		++interpreter->pc;
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenWEND->jumpToken; // on WHILE
	}

	return ErrorNone;
}

enum ErrorCode cmd_EXIT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// EXIT
	struct Token *tokenEXIT = interpreter->pc;
	++interpreter->pc;

	if (interpreter->pass == PassPrepare)
	{
		enum LabelType types[] = {LabelTypeFOR, LabelTypeDO, LabelTypeWHILE, LabelTypeREPEAT};
		struct LabelStackItem *item = lab_searchLabelStackItem(interpreter, types, 4);
		if (!item)
			return ErrorExitNotInsideLoop;

		tokenEXIT->jumpToken = item->token;

		return itp_endOfCommand(interpreter);
	}
	else if (interpreter->pass == PassRun)
	{
		interpreter->pc = tokenEXIT->jumpToken->jumpToken;
	}

	return ErrorNone;
}

enum ErrorCode cmd_SYSTEM(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// SYSTEM
	++interpreter->pc;

	// type value
	struct TypedValue tValue = itp_evaluateNumericExpression(core, 0, 8);
	if (tValue.type == ValueTypeError)
		return tValue.v.errorCode;

	// comma
	if (interpreter->pc->type != TokenComma)
		return ErrorSyntax;
	++interpreter->pc;

	// setting value
	struct TypedValue sValue = itp_evaluateExpression(core, TypeClassNumeric);
	if (sValue.type == ValueTypeError)
		return sValue.v.errorCode;

	if (interpreter->pass == PassRun)
	{
		switch ((int)tValue.v.floatValue)
		{
		case 0:
			core->machineInternals->isEnergySaving = (sValue.v.floatValue != 0.0f);
			break;

		case 1:
			core->machineInternals->planeColor0IsOpaque[0] = (sValue.v.floatValue != 0.0f);
			break;

		case 2:
			core->machineInternals->planeColor0IsOpaque[1] = (sValue.v.floatValue != 0.0f);
			break;

		case 3:
			core->machineInternals->planeColor0IsOpaque[2] = (sValue.v.floatValue != 0.0f);
			break;

		case 4:
			core->machineInternals->planeColor0IsOpaque[3] = (sValue.v.floatValue != 0.0f);
			break;

		case 5:
			core->machine->videoRegisters.attr.planeADoubled = (sValue.v.floatValue != 0.0f);
			break;

		case 6:
			core->machine->videoRegisters.attr.planeBDoubled = (sValue.v.floatValue != 0.0f);
			break;

		case 7:
			core->machine->videoRegisters.attr.planeCDoubled = (sValue.v.floatValue != 0.0f);
			break;

		case 8:
			core->machine->videoRegisters.attr.planeDDoubled = (sValue.v.floatValue != 0.0f);
			break;
		}
	}

	return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_COMPAT(struct Core *core)
{
	struct Interpreter *interpreter = core->interpreter;

	// COMPAT
	++interpreter->pc;

	if (interpreter->pass == PassRun)
	{
		interpreter->compat = true;
	}

	return itp_endOfCommand(interpreter);
}
