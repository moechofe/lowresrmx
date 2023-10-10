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

#include "data.h"
#include "interpreter.h"

void dat_nextData(struct Interpreter *interpreter)
{
    interpreter->currentDataValueToken++;
    if (interpreter->currentDataValueToken->type == TokenComma)
    {
        // value follows
        interpreter->currentDataValueToken++;
    }
    else
    {
        // next DATA line
        interpreter->currentDataToken = interpreter->currentDataToken->jumpToken;
        if (interpreter->currentDataToken)
        {
            interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
        }
        else
        {
            interpreter->currentDataValueToken = NULL;
        }
    }
}

void dat_restoreData(struct Interpreter *interpreter, struct Token *jumpToken)
{
    if (jumpToken)
    {
        struct Token *dataToken = interpreter->firstData;
        while (dataToken && dataToken < jumpToken)
        {
            dataToken = dataToken->jumpToken;
        }
        interpreter->currentDataToken = dataToken;
    }
    else
    {
        interpreter->currentDataToken = interpreter->firstData;
    }

    if (interpreter->currentDataToken)
    {
        interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
    }
    else
    {
        interpreter->currentDataValueToken = NULL;
    }
}

struct Token* dat_reachData(struct Interpreter *interpreter, struct Token *jumpToken)
{
    struct Token *dataToken=NULL;
    if (jumpToken)
    {
        dataToken = interpreter->firstData;
        while (dataToken && dataToken < jumpToken)
        {
            dataToken = dataToken->jumpToken;
        }
    }
    return dataToken;
}

struct Token* dat_readData(struct Token *dataToken, int skip)
{
    while(dataToken)
    {
				switch(dataToken->type)
				{
        		case TokenDATA:
								dataToken+=1;
								break;

						case TokenREM:
						case TokenApostrophe:
								dataToken+=1;
								break;

        		case TokenComma:
								dataToken+=1;
								break;

        		case TokenEol:
								dataToken+=1;
								break;

        		case TokenFloat:
						case TokenMinus:
						case TokenString:
            		if(skip-->0) dataToken+=dataToken->type==TokenMinus?2:1;
            		else return dataToken;
								break;

						default:
								return NULL;
        }
    }
    return NULL;
}

float dat_readFloat(struct Token *jumpToken, int skip, float def)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return def;
    if(dataToken->type==TokenMinus) return -(dataToken+1)->floatValue;
    if(dataToken->type==TokenFloat) return dataToken->floatValue;
    else return def;
}

uint8_t dat_readU8(struct Token *jumpToken, int skip, uint8_t def)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return def;
    if(dataToken->type==TokenMinus) return -(uint8_t)((dataToken+1)->floatValue);
    if(dataToken->type==TokenFloat) return (uint8_t)dataToken->floatValue;
    else return def;
}

struct RCString *dat_readString(struct Token *jumpToken, int skip)
{
    struct Token* dataToken=dat_readData(jumpToken, skip);
		if(!dataToken) return NULL;
    if(dataToken->type==TokenString) return dataToken->stringValue;
    else return NULL;
}
