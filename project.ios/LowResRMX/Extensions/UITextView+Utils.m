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
