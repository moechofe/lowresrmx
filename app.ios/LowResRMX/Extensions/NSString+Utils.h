//
//  NSString+Utils.h
//  Pixels
//
//  Created by Timo Kloss on 18/1/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (Utils)

- (NSString *)substringWithLineAtIndex:(NSUInteger)index;
- (NSUInteger)countLines;
- (NSUInteger)countChar:(unichar)character;
- (NSString *)stringWithMaxWords:(int)maxWords;

@end
