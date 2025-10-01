//
// Copyright 2018-2020 Timo Kloss
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

#include "cmd_subs.h"
#include "core.h"

enum ErrorCode cmd_CALL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // CALL
    struct Token *tokenCALL = interpreter->pc;
    ++interpreter->pc;

    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedSubprogramName;
    struct Token *tokenSubIdentifier = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct SubItem *item = tok_getSub(&interpreter->tokenizer, tokenSubIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedSubprogram;
        tokenCALL->jumpToken = item->token;
    }

    // optional arguments
    int numArguments = 0;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        do
        {
            // bracket or comma
            ++interpreter->pc;

            // argument
            struct Token *tokens = interpreter->pc;
            if ((interpreter->pc->type == TokenIdentifier || interpreter->pc->type == TokenStringIdentifier)
                && tokens[1].type == TokenBracketOpen
                && tokens[2].type == TokenBracketClose)
            {
                // pass array by reference
                if (interpreter->pass == PassRun)
                {
                    struct ArrayVariable *variable = var_getArrayVariable(interpreter, interpreter->pc->symbolIndex, interpreter->subLevel);
                    if (!variable) return ErrorArrayNotDimensionized;

                    enum ErrorCode errorCode = ErrorNone;
                    var_createArrayVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, variable);
                    if (errorCode != ErrorNone) return errorCode;
                }
                interpreter->pc += 3;
            }
            else
            {
                // expression
                struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
                if (value.type == ValueTypeError) return value.v.errorCode;

                if (interpreter->pass == PassRun)
                {
                    enum ErrorCode errorCode = ErrorNone;
                    if (interpreter->lastVariableValue)
                    {
                        // pass by reference (simple variable or array element)
                        enum ErrorCode errorCode = ErrorNone;
                        var_createSimpleVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, value.type, interpreter->lastVariableValue);
                        if (errorCode != ErrorNone) return errorCode;
                    }
                    else
                    {
                        // pass by value
                        struct SimpleVariable *variable = var_createSimpleVariable(interpreter, &errorCode, numArguments + 1, interpreter->subLevel + 1, value.type, NULL);
                        if (!variable) return errorCode;

                        variable->v = value.v;
                    }
                }
            }
            ++numArguments;
        }
        while (interpreter->pc->type == TokenComma);

        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;
    }

    if (interpreter->pass == PassRun)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeCALL, interpreter->pc);
        if (errorCode != ErrorNone) return errorCode;

        interpreter->pc = tokenCALL->jumpToken; // after sub name
        interpreter->subLevel++;

        // parameters
        if (interpreter->pc->type == TokenBracketOpen)
        {
            int parameterIndex = 0;
            do
            {
                if (parameterIndex >= numArguments) return ErrorArgumentCountMismatch;

                // bracket or comma
                ++interpreter->pc;

                // parameter
                struct Token *tokenIdentifier = interpreter->pc;
                if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
                enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);

                struct Token *nextToken = interpreter->pc + 1;
                if (nextToken->type == TokenBracketOpen)
                {
                    // array
                    struct ArrayVariable *variable = var_getArrayVariable(interpreter, parameterIndex + 1, interpreter->subLevel);
                    if (!variable || variable->type != varType) return ErrorTypeMismatch;

                    variable->symbolIndex = tokenIdentifier->symbolIndex;

                    interpreter->pc += 2;

                    if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
                    ++interpreter->pc;
                }
                else
                {
                    // simple variable
                    struct SimpleVariable *variable = var_getSimpleVariable(interpreter, parameterIndex + 1, interpreter->subLevel);
                    if (!variable || variable->type != varType) return ErrorTypeMismatch;

                    variable->symbolIndex = tokenIdentifier->symbolIndex;

                    ++interpreter->pc;
                }

                ++parameterIndex;
            }
            while (interpreter->pc->type == TokenComma);

            if (parameterIndex < numArguments) return ErrorArgumentCountMismatch;

            if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
            ++interpreter->pc;
        }
        else if (numArguments > 0)
        {
            return ErrorArgumentCountMismatch;
        }

        return ErrorNone;
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SUB
    struct Token *tokenSUB = interpreter->pc;
    ++interpreter->pc;

    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedSubprogramName;
    ++interpreter->pc;

    // parameters
    if (interpreter->pc->type == TokenBracketOpen)
    {
        do
        {
            // bracket or comma
            ++interpreter->pc;

            // parameter
            struct Token *tokenIdentifier = interpreter->pc;
            if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
            ++interpreter->pc;

            if (interpreter->pc->type == TokenBracketOpen)
            {
                ++interpreter->pc;
                if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
                ++interpreter->pc;
            }
        }
        while (interpreter->pc->type == TokenComma);

        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;
    }

    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->numLabelStackItems > 0)
        {
            return ErrorSubCannotBeNested;
        }
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeSUB, tokenSUB);
        if (errorCode != ErrorNone) return errorCode;

        interpreter->subLevel++;

        // Eol
        if (interpreter->pc->type != TokenEol) return ErrorSyntax;
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenSUB->jumpToken; // after END SUB
    }

    return ErrorNone;
}

enum ErrorCode cmd_END_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // END SUB
    ++interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item)
        {
            return ErrorEndSubWithoutSub;
        }
        else if (item->type == LabelTypeSUB)
        {
            item->token->jumpToken = interpreter->pc;
        }
        else
        {
            enum ErrorCode errorCode = itp_labelStackError(item);
            return errorCode != ErrorNone ? errorCode : ErrorEndSubWithoutSub;
        }

        // Eol
        if (interpreter->pc->type != TokenEol) return ErrorSyntax;
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemCALL = lab_popLabelStackItem(interpreter);
        if (!itemCALL) return ErrorEndSubWithoutSub;

        // clean local variables
        var_freeSimpleVariables(interpreter, interpreter->subLevel);
        var_freeArrayVariables(interpreter, interpreter->subLevel);

        if (itemCALL->type == LabelTypeONCALL)
        {
            // exit from interrupt
            interpreter->exitEvaluation = true;
        }
        else if (itemCALL->type == LabelTypeCALL)
        {
            // jump back
            interpreter->pc = itemCALL->token; // after CALL
        }
        else
        {
            return ErrorEndSubWithoutSub;
        }
    }
    interpreter->subLevel--;

    return ErrorNone;
}

/*
enum ErrorCode cmd_SHARED(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassPrepare && interpreter->subLevel == 0) return ErrorSharedOutsideOfASubprogram;

    do
    {
        // SHARED or comma
        ++interpreter->pc;

        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorExpectedVariableIdentifier;
        ++interpreter->pc;

        enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
        int symbolIndex = tokenIdentifier->symbolIndex;

        if (interpreter->pc->type == TokenBracketOpen)
        {
            // array
            ++interpreter->pc;

            if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
            ++interpreter->pc;

            if (interpreter->pass == PassRun)
            {
                struct ArrayVariable *globalVariable = var_getArrayVariable(interpreter, symbolIndex, 0);
                if (!globalVariable) return ErrorArrayNotDimensionized;

                enum ErrorCode errorCode = ErrorNone;
                var_createArrayVariable(interpreter, &errorCode, symbolIndex, interpreter->subLevel, globalVariable);
                if (errorCode != ErrorNone) return errorCode;
            }
        }
        else
        {
            // simple variable
            if (interpreter->pass == PassRun)
            {
                struct SimpleVariable *globalVariable = var_getSimpleVariable(interpreter, symbolIndex, 0);
                if (!globalVariable) return ErrorVariableNotInitialized;

                enum ErrorCode errorCode = ErrorNone;
                var_createSimpleVariable(interpreter, &errorCode, symbolIndex, interpreter->subLevel, varType, &globalVariable->v);
                if (errorCode != ErrorNone) return errorCode;
            }
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}
*/

enum ErrorCode cmd_GLOBAL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassPrepare && interpreter->subLevel > 0) return ErrorGlobalInsideOfASubprogram;

    do
    {
        // GLOBAL or comma
        ++interpreter->pc;

        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier) return ErrorSyntax;
        ++interpreter->pc;

        int symbolIndex = tokenIdentifier->symbolIndex;

        if (interpreter->pass == PassRun)
        {
            struct SimpleVariable *variable = var_getSimpleVariable(interpreter, symbolIndex, 0);
            if (variable)
            {
                variable->subLevel = SUB_LEVEL_GLOBAL;
            }
            else
            {
                enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
                enum ErrorCode errorCode = ErrorNone;
                variable = var_createSimpleVariable(interpreter, &errorCode, symbolIndex, SUB_LEVEL_GLOBAL, varType, NULL);
                if (!variable) return errorCode;
            }
        }
    }
    while (interpreter->pc->type == TokenComma);

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_EXIT_SUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // EXIT
    ++interpreter->pc;

    // SUB
    if (interpreter->pc->type != TokenSUB) return ErrorSyntax;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->subLevel == 0) return ErrorExitSubOutsideOfASubprogram;
        return itp_endOfCommand(interpreter);
    }
    else if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemCALL = lab_popLabelStackItem(interpreter);
        if (!itemCALL) return ErrorExitSubOutsideOfASubprogram;

        // clean local variables
        var_freeSimpleVariables(interpreter, interpreter->subLevel);
        var_freeArrayVariables(interpreter, interpreter->subLevel);

        if (itemCALL->type == LabelTypeONCALL)
        {
            // exit from interrupt
            interpreter->exitEvaluation = true;
        }
        else if (itemCALL->type == LabelTypeCALL)
        {
            // jump back
            interpreter->pc = itemCALL->token; // after CALL
        }
        else
        {
            return ErrorExitSubOutsideOfASubprogram;
        }
        interpreter->subLevel--;
    }
    return ErrorNone;
}
