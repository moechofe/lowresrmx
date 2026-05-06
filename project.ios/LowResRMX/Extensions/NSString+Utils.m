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

#import "NSString+Utils.h"

@implementation NSString (Utils)

- (NSString *)substringWithLineAtIndex:(NSUInteger)index
{
	NSUInteger lineStart = 0;
	NSUInteger lineEnd = self.length;
	if (index > lineEnd)
	{
		return nil;
	}

	NSRange lineStartRange = [self rangeOfString:@"\n" options:NSBackwardsSearch range:NSMakeRange(0, index)];
	NSRange lineEndRange = [self rangeOfString:@"\n" options:0 range:NSMakeRange(index, self.length - index)];

	if (lineStartRange.location != NSNotFound)
	{
		lineStart = lineStartRange.location + 1;
	}
	if (lineEndRange.location != NSNotFound)
	{
		lineEnd = lineEndRange.location;
	}

	return [self substringWithRange:NSMakeRange(lineStart, lineEnd - lineStart)];
}

- (NSUInteger)countLines
{
	NSUInteger numberOfLines, index, stringLength = [self length];
	for (index = 0, numberOfLines = 0; index < stringLength; numberOfLines++)
	{
		index = NSMaxRange([self lineRangeForRange:NSMakeRange(index, 0)]);
	}
	return numberOfLines;
}

- (NSUInteger)countChar:(unichar)character
{
	NSUInteger number = 0;
	for (NSUInteger pos = 0; pos < self.length; pos++)
	{
		if ([self characterAtIndex:pos] == character)
		{
			number++;
		}
	}
	return number;
}

- (NSString *)stringWithMaxWords:(int)maxWords
{
	NSArray *parts = [self componentsSeparatedByString:@" "];
	if (parts.count > maxWords)
	{
		NSMutableArray *mutableParts = parts.mutableCopy;
		[mutableParts removeObjectsInRange:NSMakeRange(maxWords, parts.count - maxWords)];
		NSString *shortString = [mutableParts componentsJoinedByString:@" "];
		return [NSString stringWithFormat:@"%@…", shortString];
	}
	return self;
}

@end
