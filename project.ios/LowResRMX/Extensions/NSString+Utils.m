//
//  NSString+Utils.m
//  Pixels
//
//  Created by Timo Kloss on 18/1/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

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
