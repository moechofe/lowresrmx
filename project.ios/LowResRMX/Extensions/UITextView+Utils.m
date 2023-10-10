//
//  UITextView+Utils.m
//  Pixels
//
//  Created by Timo Kloss on 7/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import "UITextView+Utils.h"

@implementation UITextView (Utils)

- (void)scrollSelectedRangeToVisible
{
    CGRect rect = [self firstRectForRange:self.selectedTextRange];
    
    CGRect bounds = self.bounds;
    UIEdgeInsets contentInset = self.contentInset;
    CGRect visibleRect = [self visibleRectConsideringInsets];
    
    // Do not scroll if rect is on screen
    if (!CGRectContainsRect(visibleRect, rect))
    {
        CGPoint contentOffset = self.contentOffset;
        // Calculates new contentOffset
        if (rect.origin.y < visibleRect.origin.y)
        {
            // rect precedes bounds, scroll up
            contentOffset.y = rect.origin.y - contentInset.top;
        }
        else
        {
            // rect follows bounds, scroll down
            contentOffset.y = rect.origin.y + contentInset.bottom + rect.size.height - bounds.size.height;
        }
        [self setContentOffset:contentOffset animated:YES];
    }
}

- (CGRect)visibleRectConsideringInsets
{
    UIEdgeInsets contentInset = self.contentInset;
    CGRect visibleRect = self.bounds;
    visibleRect.origin.x += contentInset.left;
    visibleRect.origin.y += contentInset.top;
    visibleRect.size.width -= (contentInset.left + contentInset.right);
    visibleRect.size.height -= (contentInset.top + contentInset.bottom);
    return visibleRect;
}

@end
