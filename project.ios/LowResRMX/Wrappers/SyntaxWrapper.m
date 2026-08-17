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

#import "SyntaxWrapper.h"

@interface SourceSpans ()
// Takes ownership of a malloc'd block; NULL is allowed when count is 0.
- (instancetype)initWithSpans:(nullable SourceSpan *)spans count:(NSUInteger)count;
@end

@implementation SourceSpans
{
	SourceSpan *_spans;
	NSUInteger _count;
}

- (instancetype)initWithSpans:(SourceSpan *)spans count:(NSUInteger)count {
	self = [super init];
	if (self) {
		// Takes ownership of the malloc'd block.
		_spans = spans;
		_count = count;
	}
	return self;
}

- (void)dealloc {
	free(_spans);
}

- (NSUInteger)count {
	return _count;
}

- (SourceSpan)spanAtIndex:(NSUInteger)index {
	NSParameterAssert(index < _count);
	return _spans[index];
}

@end

typedef struct
{
	const char *bytes;
	NSUInteger byteCursor;
	NSUInteger utf16Cursor;
	BOOL isAscii;
} Utf8ToUtf16Cursor;

static NSUInteger utf16OffsetForByteOffset(Utf8ToUtf16Cursor *cursor, NSUInteger byteOffset) {
	if (cursor->isAscii) {
		return byteOffset;
	}
	while (cursor->byteCursor < byteOffset) {
		unsigned char lead = (unsigned char)cursor->bytes[cursor->byteCursor];
		NSUInteger byteLength = (lead < 0x80) ? 1 : (lead < 0xE0) ? 2 : (lead < 0xF0) ? 3 : 4;
		cursor->byteCursor += byteLength;
		// Only astral characters need a surrogate pair.
		cursor->utf16Cursor += (byteLength == 4) ? 2 : 1;
	}
	return cursor->utf16Cursor;
}

@implementation SyntaxWrapper
{
	dispatch_queue_t _queue;
	struct Syntax _syntax;
}

+ (SyntaxWrapper *)shared {
	static SyntaxWrapper *shared;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		shared = [[SyntaxWrapper alloc] init];
	});
	return shared;
}

- (instancetype)init {
	self = [super init];
	if (self) {
		_queue = dispatch_queue_create("it.ro.ret.ios.LowResRMX.syntax", DISPATCH_QUEUE_SERIAL);
		syntax_init(&_syntax);
	}
	return self;
}

- (void)dealloc {
	syntax_deinit(&_syntax);
}

- (void)spansForText:(NSString *)text completion:(void (^)(SourceSpans *))completion {
	dispatch_async(_queue, ^{
		SourceSpans *spans = [self spansForTextOnQueue:text];
		dispatch_async(dispatch_get_main_queue(), ^{
			completion(spans);
		});
	});
}

- (SourceSpans *)synchronousSpansForText:(NSString *)text {
	__block SourceSpans *spans = nil;
	dispatch_sync(_queue, ^{
		spans = [self spansForTextOnQueue:text];
	});
	return spans;
}

// Must run on _queue.
- (SourceSpans *)spansForTextOnQueue:(NSString *)text {
	@autoreleasepool {
		const char *sourceCode = [text UTF8String];
		if (!sourceCode) {
			return [[SourceSpans alloc] initWithSpans:NULL count:0];
		}

		syntax_update(&_syntax, sourceCode);

		NSUInteger count = (NSUInteger)_syntax.numSpans;
		if (count == 0) {
			return [[SourceSpans alloc] initWithSpans:NULL count:0];
		}

		SourceSpan *mapped = malloc(count * sizeof(SourceSpan));
		if (!mapped) {
			return [[SourceSpans alloc] initWithSpans:NULL count:0];
		}

		NSUInteger textLength = text.length;
		size_t byteLength = strlen(sourceCode);
		Utf8ToUtf16Cursor cursor = { sourceCode, 0, 0, (byteLength == textLength) };

		NSUInteger numMapped = 0;
		for (NSUInteger i = 0; i < count; i++) {
			struct SyntaxSpan span = _syntax.spans[i];
			// Both ends are converted in order so the cursor stays in step, even for spans that are dropped below.
			NSUInteger start = utf16OffsetForByteOffset(&cursor, (NSUInteger)span.start);
			NSUInteger end = utf16OffsetForByteOffset(&cursor, (NSUInteger)(span.start + span.length));
			if (end <= start || end > textLength) {
				continue;
			}
			mapped[numMapped].range = NSMakeRange(start, end - start);
			mapped[numMapped].kind = span.kind;
			mapped[numMapped].isDeclaration = span.isDeclaration;
			numMapped++;
		}

		return [[SourceSpans alloc] initWithSpans:mapped count:numMapped];
	}
}

@end
