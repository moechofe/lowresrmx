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

#ifndef data_h
#define data_h

#include "rcstring.h"
#include <stdio.h>
#include <stdint.h>

struct Interpreter;
struct Token;

void dat_nextData(struct Interpreter *interpreter);
void dat_restoreData(struct Interpreter *interpreter, struct Token *jumpToken);

struct Token* dat_reachData(struct Interpreter *interpreter, struct Token *jumpToken);

struct Token* dat_readData(struct Token *jumpToken, int skip);
float dat_readFloat(struct Token *jumpToken, int skip, float def);
uint8_t dat_readU8(struct Token *jumpToken, int skip, uint8_t def);

struct RCString *dat_readString(struct Token *jumpToken, int skip);

#endif /* data_h */
