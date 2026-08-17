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

#include "core_syntax.h"
#include "charsets.h"
#include "string_utils.h"
#include "token.h"
#include "error.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SYNTAX_INITIAL_CAPACITY 1024

static int scanIdentifierLength(const char *sourceCode, int start)
{
	int i = start;
	while(sourceCode[i] && strchr(CharSetAlphaNum, uppercaseChar(sourceCode[i])))
	{
		i++;
	}
	if(sourceCode[i] == '$')
	{
		i++;
	}
	return i - start;
}

static int scanNumberLength(const char *sourceCode, int start)
{
	int i = start;
	if(sourceCode[i] == '$')
	{
		i++;
		while(sourceCode[i] && strchr(CharSetHex, uppercaseChar(sourceCode[i])))
		{
			i++;
		}
	}
	else if(sourceCode[i] == '%')
	{
		i++;
		while(sourceCode[i] == '0' || sourceCode[i] == '1')
		{
			i++;
		}
	}
	else
	{
		int afterDot = 0;
		while(sourceCode[i])
		{
			if(strchr(CharSetDigits, sourceCode[i]))
			{
				i++;
			}
			else if(sourceCode[i] == '.' && afterDot == 0)
			{
				afterDot = 1;
				i++;
			}
			else
			{
				break;
			}
		}
	}
	return i - start;
}

static int scanStringLength(const char *sourceCode, int start)
{
	int i = start + 1; // opening quote
	while(sourceCode[i] && sourceCode[i] != '"' && sourceCode[i] != '\n' && sourceCode[i] != '\r')
	{
		i++;
	}
	if(sourceCode[i] == '"')
	{
		i++;
	}
	return i - start;
}

static int scanCommentLength(const char *sourceCode, int start)
{
	int i = start;
	while(sourceCode[i] && sourceCode[i] != '\n' && sourceCode[i] != '\r')
	{
		i++;
	}
	return i - start;
}

static bool addSpan(struct Syntax *syntax, int start, int length, enum SyntaxKind kind)
{
	if(length <= 0)
	{
		return true;
	}
	if(syntax->numSpans >= syntax->capacity)
	{
		int capacity = syntax->capacity ? syntax->capacity * 2 : SYNTAX_INITIAL_CAPACITY;
		struct SyntaxSpan *spans = realloc(syntax->spans, capacity * sizeof(struct SyntaxSpan));
		if(!spans)
		{
			return false;
		}
		syntax->spans = spans;
		syntax->capacity = capacity;
	}
	struct SyntaxSpan *span = &syntax->spans[syntax->numSpans++];
	span->start = start;
	span->length = length;
	span->kind = kind;
	return true;
}

void syntax_init(struct Syntax *syntax)
{
	memset(syntax, 0, sizeof(struct Syntax));

	syntax->tokenizer = calloc(1, sizeof(struct Tokenizer));
	if(!syntax->tokenizer)
		exit(EXIT_FAILURE);
}

void syntax_deinit(struct Syntax *syntax)
{
	free(syntax->tokenizer);
	syntax->tokenizer = NULL;

	free(syntax->spans);
	syntax->spans = NULL;
	syntax->numSpans = 0;
	syntax->capacity = 0;
}

// After GOTO/GOSUB/RESTORE the following identifiers name labels, and ON x GOTO takes a comma separated list of them. These track how far into such a list we are; anything else ends it.
enum LabelListState
{
	LabelListNone,
	LabelListExpectLabel,
	LabelListExpectComma,
};

struct CoreError syntax_update(struct Syntax *syntax, const char *sourceCode)
{
	syntax->numSpans = 0;

	struct Tokenizer *tokenizer = syntax->tokenizer;
	struct CoreError error = tok_tokenizeProgram(tokenizer, sourceCode);

	enum LabelListState labelList = LabelListNone;

	for(int i = 0; i < tokenizer->numTokens; i++)
	{
		struct Token *token = &tokenizer->tokens[i];
		enum TokenType type = token->type;
		int start = token->sourcePosition;
		enum TokenType previousType = (i > 0) ? tokenizer->tokens[i - 1].type : TokenUndefined;

		switch(type)
		{
		case TokenString:
			addSpan(syntax, start, scanStringLength(sourceCode, start), SyntaxString);
			labelList = LabelListNone;
			break;

		case TokenFloat:
			addSpan(syntax, start, scanNumberLength(sourceCode, start), SyntaxNumber);
			labelList = LabelListNone;
			break;

		case TokenApostrophe:
			addSpan(syntax, start, scanCommentLength(sourceCode, start), SyntaxComment);
			labelList = LabelListNone;
			break;

		case TokenLabel:
			// The tokenizer consumed the ':' too, but colour the identifier only.
			addSpan(syntax, start, scanIdentifierLength(sourceCode, start), SyntaxLabel);
			labelList = LabelListNone;
			break;

		case TokenComma:
			labelList = (labelList == LabelListExpectComma) ? LabelListExpectLabel : LabelListNone;
			break;

		case TokenEol:
			// '\n', '\r' and also ':' either way the statement is over.
			labelList = LabelListNone;
			break;

		case TokenIdentifier:
		case TokenStringIdentifier:
		{
			int length = scanIdentifierLength(sourceCode, start);
			if(labelList == LabelListExpectLabel && type == TokenIdentifier
			   && tok_getJumpLabel(tokenizer, token->symbolIndex))
			{
				addSpan(syntax, start, length, SyntaxLabel);
				labelList = LabelListExpectComma;
			}
			else
			{
				if(previousType == TokenSUB
				   || (previousType == TokenCALL && tok_getSub(tokenizer, token->symbolIndex)))
				{
					addSpan(syntax, start, length, SyntaxSub);
				}
				labelList = LabelListNone;
			}
			break;
		}

		default:
		{
			// Everything else is a keyword or an operator.
			const char *keyword = TokenStrings[type];
			if(keyword && strchr(CharSetAlphaNum, keyword[0]))
			{
				addSpan(syntax, start, (int)strlen(keyword), SyntaxKeyword);
			}
			labelList = (type == TokenGOTO || type == TokenGOSUB || type == TokenRESTORE)
				? LabelListExpectLabel
				: LabelListNone;
			break;
		}
		}
	}

	// Releases the token strings and zeroes the tokenizer for the next call.
	// Must run on the error path too, or every failed keystroke leaks them.
	tok_freeTokens(tokenizer);

	return error;
}
