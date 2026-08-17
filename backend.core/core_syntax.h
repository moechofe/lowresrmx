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

#ifndef core_syntax_h
#define core_syntax_h

#include "error.h"
#include "tokenizer.h"
#include <stdbool.h>
#include <stdio.h>

enum SyntaxKind
{
	SyntaxKeyword,
	SyntaxNumber,
	SyntaxString,
	SyntaxComment,
	SyntaxLabel,
	SyntaxSub,
};

struct SyntaxSpan
{
	int start;
	int length;
	enum SyntaxKind kind;
	bool isDeclaration;
};

struct Syntax
{
	struct Tokenizer *tokenizer;
	struct SyntaxSpan *spans;
	int numSpans;
	int capacity; // allocated spans, kept between calls
};

void syntax_init(struct Syntax *syntax);
void syntax_deinit(struct Syntax *syntax);

// Tokenizes sourceCode and fills syntax->spans with the ranges to colour, in ascending start order and never overlapping. Only the program part is covered: the tokenizer stops at the first '#', so ROM entries produce no spans.
// On a tokenizer error the spans found *before* the error are kept and the error is returned, so a half-typed program still colours the part that parses. Callers should render the spans regardless of the return value.
struct CoreError syntax_update(struct Syntax *syntax, const char *sourceCode);

#endif /* core_syntax_h */
