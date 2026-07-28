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

#import "SearchToolbar.h"

@interface SearchToolbar () <UITextFieldDelegate, UITraitEnvironment>

@property (weak, nonatomic) IBOutlet UITextField *findTextField;
@property (weak, nonatomic) IBOutlet UITextField *replaceTextField;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *findConstraint;
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *replaceConstraint;

@property (weak) UITextField *activeTextField;

@end

@implementation SearchToolbar

- (void)awakeFromNib
{
	[super awakeFromNib];
	self.findTextField.delegate = self;
	self.replaceTextField.delegate = self;

	[self updateColors];
}

- (void)updateColors
{
	// Use system dynamic colors for automatic light/dark mode adaptation
	self.backgroundColor = [UIColor systemBackgroundColor];
	self.tintColor = [UIColor systemBlueColor];
	self.findTextField.backgroundColor = [UIColor secondarySystemBackgroundColor];
	self.replaceTextField.backgroundColor = [UIColor secondarySystemBackgroundColor];
	self.findTextField.textColor = [UIColor labelColor];
	self.replaceTextField.textColor = [UIColor labelColor];
	self.findTextField.tintColor = [UIColor systemBlueColor];
	self.replaceTextField.tintColor = [UIColor systemBlueColor];
}

- (void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection
{
	[super traitCollectionDidChange:previousTraitCollection];
	[self updateColors];
	if (self.traitCollection.horizontalSizeClass == UIUserInterfaceSizeClassRegular)
	{
		_findTextField.clearButtonMode = UITextFieldViewModeAlways;
		_replaceTextField.clearButtonMode = UITextFieldViewModeAlways;
	}
	else
	{
		_findTextField.clearButtonMode = UITextFieldViewModeNever;
		_replaceTextField.clearButtonMode = UITextFieldViewModeNever;
	}
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	[self updateDynamicConstraints];
}

- (BOOL)dynamicLayout
{
	return (self.bounds.size.width < 414.0); // < iPhone 6+
}

- (void)updateDynamicConstraints
{
	if (![self dynamicLayout])
	{
		self.findConstraint.priority = 240;
		self.replaceConstraint.priority = 241;
	}
	else
	{
		self.findConstraint.priority = (self.activeTextField == _findTextField) ? 999 : 240;
		self.replaceConstraint.priority = (self.activeTextField == _replaceTextField) ? 999 : 241;
	}
}

- (void)activate
{
	[self.findTextField becomeFirstResponder];
}

- (void)updateFontSize:(CGFloat)fontSize
{
	// Scale the search toolbar font size relative to the editor font
	// Use a slightly smaller size for the search fields (multiply by 0.9)
	CGFloat searchFontSize = fontSize * 0.9;

	// Set minimum font size of 12pt for readability
	if (searchFontSize < 12.0) {
		searchFontSize = 12.0;
	}

	UIFont *font = [UIFont systemFontOfSize:searchFontSize];
	self.findTextField.font = font;
	self.replaceTextField.font = font;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	self.activeTextField = textField;
	if ([self dynamicLayout])
	{
		[self updateDynamicConstraints];
		[UIView animateWithDuration:0.3 animations:^{
			 [self layoutIfNeeded];
		 }];
	}
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	// textField.text = [textField.text uppercaseString];
	self.activeTextField = nil;
	if ([self dynamicLayout])
	{
		[self updateDynamicConstraints];
		[UIView animateWithDuration:0.3 animations:^{
			 [self layoutIfNeeded];
		 }];
	}
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	[self endEditing:YES];
	return NO;
}

- (IBAction)onFindPrevTapped:(id)sender
{
	if (self.findTextField.text.length > 0)
	{
		[self endEditing:YES];
		[self.searchDelegate searchToolbar:self didSearch:self.findTextField.text backwards:YES];
	}
}

- (IBAction)onFindNextTapped:(id)sender
{
	if (self.findTextField.text.length > 0)
	{
		[self endEditing:YES];
		[self.searchDelegate searchToolbar:self didSearch:self.findTextField.text backwards:NO];
	}
}

- (IBAction)onReplaceTapped:(id)sender
{
	if (self.findTextField.text.length > 0 && self.replaceTextField.text.length > 0)
	{
		[self endEditing:YES];
		[self.searchDelegate searchToolbar:self didReplace:self.findTextField.text with:self.replaceTextField.text];
	}
}

@end
