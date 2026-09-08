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

#include "labels.h"
#include "interpreter.h"
#include <stdbool.h>
#include <string.h>

enum ErrorCode lab_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token)
{
	if(interpreter->numLabelStackItems >= MAX_LABEL_STACK_ITEMS)
		return ErrorStackOverflow;
	struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems];
	item->type = type;
	item->token = token;
	interpreter->numLabelStackItems++;
	return ErrorNone;
}

struct LabelStackItem *lab_popLabelStackItem(struct Interpreter *interpreter)
{
	if(interpreter->numLabelStackItems > 0)
	{
		interpreter->numLabelStackItems--;
		return &interpreter->labelStackItems[interpreter->numLabelStackItems];
	}
	return NULL;
}

struct LabelStackItem *lab_peekLabelStackItem(struct Interpreter *interpreter)
{
	if(interpreter->numLabelStackItems > 0)
	{
		return &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
	}
	return NULL;
}

struct LabelStackItem *lab_searchLabelStackItem(struct Interpreter *interpreter, enum LabelType types[], int numTypes)
{
	int i = interpreter->numLabelStackItems - 1;
	while(i >= 0)
	{
		struct LabelStackItem *item = &interpreter->labelStackItems[i];
		for(int j = 0; j < numTypes; j++)
		{
			if(item->type == types[j])
			{
				return item;
			}
		}
		--i;
	}
	return NULL;
}

static bool lab_isNameCharacter(char character)
{
	return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
	       (character >= '0' && character <= '9') || character == '_';
}

void lab_getStackItemName(struct Interpreter *interpreter, const struct LabelStackItem *item, char *buffer,
                          size_t bufferSize)
{
	if(bufferSize == 0)
		return;
	buffer[0] = '\0';

	// the program was freed
	if(!item || !item->token || !interpreter->sourceCode || item->token->sourcePosition <= 0)
		return;

	// scan backwards
	const char *sourceCode = interpreter->sourceCode;
	const char *end = &sourceCode[item->token->sourcePosition];
	const char *start = end;
	while(start > sourceCode && lab_isNameCharacter(start[-1]))
		--start;

	size_t length = (size_t)(end - start);
	if(length > bufferSize - 1)
		length = bufferSize - 1;
	memcpy(buffer, start, length);
	buffer[length] = '\0';
}
