//
//  UIColor+Utils.m
//  Pixels
//
//  Created by Timo Kloss on 14/4/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import "UIColor+Utils.h"

@implementation UIColor (Utils)

+ (UIColor *)colorWithHex:(uint32_t)color alpha:(CGFloat)alpha
{
    CGFloat red   = ((color & 0xFF0000) >> 16) / 255.0;
    CGFloat green = ((color & 0x00FF00) >> 8) / 255.0;
    CGFloat blue  = (color & 0x0000FF) / 255.0;
    return [UIColor colorWithRed:red green:green blue:blue alpha:alpha];
}

@end
