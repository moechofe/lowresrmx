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

#import <Foundation/Foundation.h>

#import "core_syntax.h"

NS_ASSUME_NONNULL_BEGIN

typedef struct
{
	NSRange range;
	enum SyntaxKind kind;
	bool isDeclaration;
} SourceSpan;

@interface SourceSpans : NSObject

@property (readonly) NSUInteger count;

- (SourceSpan)spanAtIndex:(NSUInteger) index NS_SWIFT_NAME(span(at:));

@end

@interface SyntaxWrapper : NSObject

@property (class, readonly) SyntaxWrapper *shared;

// Analyses off the caller's thread and delivers the result on the main queue.
- (void)spansForText:(NSString *)text completion:(void (^)(SourceSpans *spans)) completion
	NS_SWIFT_NAME(spans(for:completion:));

// Analyses and blocks until the result is ready. Safe to call from the main thread
- (SourceSpans *)synchronousSpansForText:(NSString *) text NS_SWIFT_NAME(synchronousSpans(for:));

@end

NS_ASSUME_NONNULL_END
