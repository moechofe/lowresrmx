//
//  SearchToolbar.m
//  Pixels
//
//  Created by Timo Kloss on 5/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import "SearchToolbar.h"
#import "AppStyle.h"

@interface SearchToolbar() <UITextFieldDelegate, UITraitEnvironment>

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
//    self.backgroundColor = [AppStyle mediumTintColor];
//    self.tintColor = [AppStyle brightTintColor];
//    self.findTextField.tintColor = [AppStyle mediumTintColor];
//    self.replaceTextField.tintColor = [AppStyle mediumTintColor];
    self.findTextField.delegate = self;
    self.replaceTextField.delegate = self;
}

- (void)traitCollectionDidChange:(UITraitCollection *)previousTraitCollection
{
    [super traitCollectionDidChange:previousTraitCollection];
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
    textField.text = [textField.text uppercaseString];
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
