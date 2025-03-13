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

#include "cmd_maths.h"
#include "core.h"
#include "pcg_basic.h"
#include "machine.h"

#define _USE_MATH_DEFINES
#ifndef __USE_MISC
  #define __USE_MISC
#endif

#include <math.h>
#include <assert.h>
#include <stdlib.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

struct TypedValue fnc_math0(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPI:
                value.v.floatValue = 3.14159265358979323846264338327950288;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math1(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenABS:
                value.v.floatValue = fabsf(xValue.v.floatValue);
                break;

            case TokenACOS:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = acosf(xValue.v.floatValue);
                break;

            case TokenASIN:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = asinf(xValue.v.floatValue);
                break;

            case TokenATAN:
                value.v.floatValue = atanf(xValue.v.floatValue);
                break;

            case TokenCOS:
                value.v.floatValue = cosf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenEXP:
                value.v.floatValue = expf(xValue.v.floatValue);
                break;

            case TokenHCOS:
                value.v.floatValue = coshf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenHSIN:
                value.v.floatValue = sinhf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenHTAN:
                value.v.floatValue = tanhf(xValue.v.floatValue);
                break;

            case TokenINT:
                value.v.floatValue = floorf(xValue.v.floatValue);
                break;

            case TokenLOG:
                if (xValue.v.floatValue <= 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = logf(xValue.v.floatValue);
                break;

            case TokenSGN:
                value.v.floatValue = (xValue.v.floatValue > 0) ? 1 : (xValue.v.floatValue < 0) ? BAS_TRUE : BAS_FALSE;
                break;

            case TokenSIN:
                value.v.floatValue = sinf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenSQR:
                if (xValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = sqrtf(xValue.v.floatValue);
                break;

            case TokenTAN:
                value.v.floatValue = tanf(xValue.v.floatValue * M_PI * 2);
                break;

            case TokenCEIL:
								value.v.floatValue = ceilf(xValue.v.floatValue);
								break;

						case TokenFLOOR:
								value.v.floatValue = floorf(xValue.v.floatValue);
								break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math2(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        float x = xValue.v.floatValue;
        float y = yValue.v.floatValue;

        switch (type)
        {
            case TokenMAX:
                value.v.floatValue = (x > y) ? x : y;
                break;

            case TokenMIN:
                value.v.floatValue = (x < y) ? x : y;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math3(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // z expression
    struct TypedValue zValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (zValue.type == ValueTypeError) return zValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        float x = xValue.v.floatValue;
        float y = yValue.v.floatValue;
        float z = zValue.v.floatValue;

        switch (type)
        {
            case TokenCLAMP:
                value.v.floatValue = (x < y) ? y : (x > z) ? z : x;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

float easeOutBounce(float x) {

  if (x < 1 / 2.75) return (7.5625 * x * x);
  if (x < 2 / 2.75) {
    x = x - (1.5 / 2.75);
    return (7.5625 * x * x + 0.75);
  }
  else if (x < 2.5 / 2.75) {
    x = x - (2.25 / 2.75);
    return (7.5625 * x * x + 0.9375);
  }
  x = x - (2.625 / 2.75);
  return (7.5625 * x * x + 0.984375);
}

float easeInBounce(float x) {
  return 1 - easeOutBounce(1 - x);
}

float easeInOutBounce(float x) {
  return x < 0.5
    ? (1 - easeOutBounce(1 - 2 * x)) / 2
    : (1 + easeOutBounce(2 * x - 1)) / 2;
}

struct TypedValue fnc_EASE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;

    // easing value
    struct TypedValue easing = itp_evaluateNumericExpression(core, 0, 9);
    if (easing.type == ValueTypeError) return easing;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // inout value
    struct TypedValue inout = itp_evaluateNumericExpression(core, -1, 1);
    if (inout.type == ValueTypeError) return inout;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    int e=(int)easing.v.floatValue;
    int io=(int)inout.v.floatValue;
    float x=xValue.v.floatValue;
    float v;

    x = x>1.0f ? 1.0f : x<0.0f ? 0.0f : x;

    const float c1 = 1.70158;
    const float c2 = c1 * 1.525;
    const float c3 = c1 + 1;
    const float c4 = (2 * M_PI) / 3;
    const float c5 = (2 * M_PI) / 4.5;

    // linear
    if (e==0) v = xValue.v.floatValue;

    // sine
    else if(e==1 && io<0) v = 1 - cosf((x * M_PI) / 2);
    else if(e==1 && io==0) v = -(cosf(M_PI * x) - 1) / 2;
    else if(e==1 && io>0) v = sinf((x * M_PI) / 2);

    // quad
    else if(e==2 && io<0) v =  x * x;
    else if(e==2 && io==0) v = x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
    else if(e==2 && io>0) v = 1 - (1 - x) * (1 - x);

    // cubic
    else if(e==3 && io<0) v = x * x * x;
    else if(e==3 && io==0) v = x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
    else if(e==3 && io>0) v = 1 - pow(1 - x, 3);

    // quart
    else if(e==4 && io<0) v = x * x * x * x;
    else if(e==4 && io==0) v = x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
    else if(e==4 && io>0) v = 1 - pow(1 - x, 4);

    // quint
    else if(e==5 && io<0) v =  x * x * x * x * x;
    else if(e==5 && io==0) v = x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
    else if(e==5 && io>0) v = 1 - pow(1 - x, 5);

    // circ
    else if(e==6 && io<0) v = 1 - sqrt(1 - pow(x, 2));
    else if(e==6 && io==0) v = x < 0.5 ? (1 - sqrt(1 - pow(2 * x, 2))) / 2 : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
    else if(e==6 && io>0) v = sqrt(1 - pow(x - 1, 2));

    // back
    else if(e==7 && io<0) v = c3 * x * x * x - c1 * x * x;
    else if(e==7 && io==0) v = x < 0.5 ? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2 : (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    else if(e==7 && io>0) v = 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);

    // elastic
    else if(e==8 && io<0) v = x == 0 ? 0 : x == 1 ? 1 : -pow(2, 10 * x - 10) * sinf((x * 10 - 10.75) * c4);
    else if(e==8 && io==0) v = x == 0 ? 0 : x == 1 ? 1 : x < 0.5 ? -(pow(2, 20 * x - 10) * sinf((20 * x - 11.125) * c5)) / 2 : (pow(2, -20 * x + 10) * sinf((20 * x - 11.125) * c5)) / 2 + 1;
    else if(e==8 && io>0) v = x == 0 ? 0 : x == 1 ? 1 : pow(2, -10 * x) * sinf((x * 10 - 0.75) * c4) + 1;

    // bounce
    else if(e==9 && io<0) v = easeInBounce(x);
    else if(e==9 && io==0) v = easeInOutBounce(x);
    else if(e==9 && io>0) v = easeOutBounce(x);

    struct TypedValue value;
    value.type = ValueTypeFloat;
    value.v.floatValue = v;

    return value;
}

enum ErrorCode cmd_RANDOMIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // RANDOMIZE
    ++interpreter->pc;

    struct TypedValue yValue;
    yValue.type = ValueTypeNull;

    pcg32_random_t *rng = &interpreter->defaultRng;

    // RANDOMIZE seed
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    // XXX: if (xValue.type != ValueTypeNull)

    // RANDOMIZE seed [,]
    if(interpreter->pc->type == TokenComma && !core->interpreter->compat) {
      ++interpreter->pc;

      // RANDOMIZE seed [, addr]
      yValue = itp_evaluateExpression(core, TypeClassNumeric);
      if (yValue.type == ValueTypeError) return yValue.v.errorCode;

      if (interpreter->pass == PassRun)
      {
          int addr = yValue.v.floatValue;
          rng = (pcg32_random_t *)(((uint8_t*)core->machine)+addr);
      }
    }

    if (interpreter->pass == PassRun)
    {
        if(interpreter->compat) interpreter->seed = xValue.v.floatValue;
        else {
          pcg32_srandom_r(rng, (uint32_t)xValue.v.floatValue, (intptr_t)rng);
        }
    }

    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_RND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // RND
    ++interpreter->pc;

    struct TypedValue xValue;
    xValue.type=ValueTypeNull;
    pcg32_random_t *rng = &interpreter->defaultRng;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // RND(
        ++interpreter->pc;

        // RND(bound
        xValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (xValue.type == ValueTypeError) return xValue;

        // RND(bound [,]
        if(interpreter->pc->type == TokenComma && !core->interpreter->compat) {
          ++interpreter->pc;

          // RND(bound [, addr]
          struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
          if (yValue.type == ValueTypeError) return yValue;

          if (interpreter->pass == PassRun) {
              int addr = yValue.v.floatValue;
              rng = (pcg32_random_t *)((uint8_t*)(core->machine)+addr);
          }
        }

        // RND(bound)
        // RND(bound [, addr])
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    }

    struct TypedValue value;
    value.type = ValueTypeFloat;

    if (interpreter->pass == PassRun)
    {
        if(core->interpreter->compat) {
            int seed = (1140671485 * interpreter->seed + 12820163) & 0xFFFFFF;
            interpreter->seed = seed;
            float rnd = seed / (float)0x1000000;

            if (xValue.type!=ValueTypeNull && xValue.v.floatValue >= 0)
            {
                // integer 0...x
                value.v.floatValue = floorf(rnd * (xValue.v.floatValue + 1));
            }
            else
            {
                // float 0..<1
                value.v.floatValue = rnd;
            }
        } else {

            if (xValue.type==ValueTypeNull) {
                value.v.floatValue = (float)ldexp(pcg32_random_r(rng), -32);
            } else {
                value.v.floatValue = (float)pcg32_boundedrand_r(rng, (uint32_t)xValue.v.floatValue + 1);
            }
        }
    }
    return value;
}

enum ErrorCode cmd_ADD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // ADD
    ++interpreter->pc;

    enum ErrorCode errorCode = ErrorNone;

    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // n vale
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;

    bool hasRange = false;
    float base = 0;
    float top = 0;

    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;

        // base value
        struct TypedValue baseValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (baseValue.type == ValueTypeError) return baseValue.v.errorCode;
        base = baseValue.v.floatValue;

        // TO
        if (interpreter->pc->type != TokenTO) return ErrorSyntax;
        ++interpreter->pc;

        // top value
        struct TypedValue topValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (topValue.type == ValueTypeError) return topValue.v.errorCode;
        top = topValue.v.floatValue;

        hasRange = true;
    }

    if (interpreter->pass == PassRun)
    {
        varValue->floatValue += nValue.v.floatValue;
        if (hasRange)
        {
            if (varValue->floatValue < base) varValue->floatValue = top;
            if (varValue->floatValue > top) varValue->floatValue = base;
        }
    }

    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INC_DEC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // INC/DEC
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;

    enum ErrorCode errorCode = ErrorNone;

    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;

    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenINC:
                ++varValue->floatValue;
                break;

            case TokenDEC:
                --varValue->floatValue;
                break;

            default:
                assert(0);
                break;
        }
    }

    return itp_endOfCommand(interpreter);
}
