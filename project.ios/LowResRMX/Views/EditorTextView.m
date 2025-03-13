//
//  EditorTextView.m
//  Pixels
//
//  Created by Timo Kloss on 25/2/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import "EditorTextView.h"

@interface EditorTextView ()
@property UIToolbar *keyboardToolbar;
@end

@implementation EditorTextView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    self.textContainerInset = UIEdgeInsetsMake(8, 8, 8, 8);
    
    if ([UITextInputAssistantItem class] && [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
    {
        [self initShortcutsBar];
    }
    else
    {
        [self initKeyboardToolbar];
    }

    UIMenuController *menu = [UIMenuController sharedMenuController];
    menu.menuItems = @[
                       [[UIMenuItem alloc] initWithTitle:@"Help" action:@selector(help:)],
                       [[UIMenuItem alloc] initWithTitle:@"Indent <" action:@selector(indentLeft:)],
                       [[UIMenuItem alloc] initWithTitle:@"Indent >" action:@selector(indentRight:)]
                       ];
}

- (void)initKeyboardToolbar
{
    self.keyboardToolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 32)];
    
    NSArray *keys = @[@"=", @"<", @">", @"+", @"-", @"*", @"/", @"(", @")", @","];
    
    NSMutableArray *buttons = [NSMutableArray array];
    for (NSString *key in keys)
    {
        UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithTitle:key style:UIBarButtonItemStylePlain target:self action:@selector(onSpecialKeyTapped:)];
        [buttons addObject:button];
        
        UIBarButtonItem *space = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil];
        [buttons addObject:space];
    }
    
    
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(onKeyboardDoneTapped:)];
    [buttons addObject:doneButton];
    
    self.keyboardToolbar.tintColor = self.tintColor;
    
    self.keyboardToolbar.items = buttons;
    self.inputAccessoryView = self.keyboardToolbar;
}

- (void)initShortcutsBar
{
    NSArray *keys = @[@"_", @"=", @"<", @">", @"+", @"-", @"*", @"/", @"(", @")"];
    
    NSMutableArray *buttons = [NSMutableArray array];
    for (NSString *key in keys)
    {
        UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithTitle:key style:UIBarButtonItemStylePlain target:self action:@selector(onSpecialKeyTapped:)];
        [buttons addObject:button];
    }
    
    UIBarButtonItemGroup *group = [[UIBarButtonItemGroup alloc] initWithBarButtonItems:buttons representativeItem:nil];
    self.inputAssistantItem.trailingBarButtonGroups = @[group];
    self.inputAssistantItem.allowsHidingShortcuts = NO;
}

- (void)onSpecialKeyTapped:(UIBarButtonItem *)button
{
    [self insertCheckedText:button.title];
}

- (void)onKeyboardDoneTapped:(UIBarButtonItem *)button
{
    [self resignFirstResponder];
}

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender
{
    if (action == @selector(help:))
    {
        return self.selectedRange.length > 0 && self.selectedRange.length <= 20;
    }
    else if (   action == @selector(indentRight:)
             || action == @selector(indentLeft:) )
    {
        return self.isEditable;
    }
    else if (   action == @selector(copy:)
             || action == @selector(paste:)
             || action == @selector(cut:)
             || action == @selector(delete:)
             || action == @selector(select:)
             || action == @selector(selectAll:) )
    {
        return [super canPerformAction:action withSender:sender];
    }
    return NO;
}

- (void)help:(id)sender
{
    [self.editorDelegate editorTextView:self didSelectHelpWithRange:self.selectedRange];
}

- (void)indentRight:(id)sender
{
    [self indentToRight:YES];
}

- (void)indentLeft:(id)sender
{
    [self indentToRight:NO];
}

- (void)indentToRight:(BOOL)right
{
    NSRange originalRange = [self.text lineRangeForRange:self.selectedRange];
    NSRange finalRange = originalRange;
    NSMutableString *subtext = [[self.text substringWithRange:originalRange] mutableCopy];
    NSInteger pos = 0;
    
    NSCharacterSet *spacesSet = [NSCharacterSet whitespaceCharacterSet];
    NSCharacterSet *newlineSet = [NSCharacterSet newlineCharacterSet];
    
    while (pos < subtext.length)
    {
        if (right)
        {
            [subtext insertString:@"  " atIndex:pos];
            finalRange.length += 2;
        }
        else
        {
            NSInteger num = 0;
            for (NSInteger ci = pos; ci < pos + 2 && ci < subtext.length; ci++)
            {
                unichar character = [subtext characterAtIndex:ci];
                if ([spacesSet characterIsMember:character])
                {
                    num++;
                }
                else if ([newlineSet characterIsMember:character])
                {
                    break;
                }
            }
            if (num > 0)
            {
                [subtext replaceCharactersInRange:NSMakeRange(pos, num) withString:@""];
                finalRange.length -= num;
            }
        }
        
        NSRange lineRange = [subtext lineRangeForRange:NSMakeRange(pos, 0)];
        pos += lineRange.length;
    }
    self.text = [self.text stringByReplacingCharactersInRange:originalRange withString:subtext];
    [self.delegate textViewDidChange:self];

    // selection and menu
    if (finalRange.location + finalRange.length < self.text.length)
    {
        finalRange.length--;
    }
    self.selectedRange = finalRange;
    [self scrollRangeToVisible:self.selectedRange];
/*    CGRect rect = [self firstRectForRange:self.selectedTextRange];
    UIMenuController *menu = [UIMenuController sharedMenuController];
    [menu setTargetRect:rect inView:self];
    [menu setMenuVisible:YES animated:NO];*/
}

- (void)insertCheckedText:(NSString *)text
{
    if (!self.isEditable) return;
    if (!self.delegate || [self.delegate textView:self shouldChangeTextInRange:self.selectedRange replacementText:text])
    {
        [self insertText:text];
    }
}

@end
