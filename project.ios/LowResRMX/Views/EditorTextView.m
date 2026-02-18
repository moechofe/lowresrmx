//
//  EditorTextView.m
//  Pixels
//
//  Created by Timo Kloss on 25/2/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import "EditorTextView.h"

@interface EditorTextView ()
@end

@implementation EditorTextView

- (void)awakeFromNib
{
    [super awakeFromNib];

    self.textContainerInset = UIEdgeInsetsMake(8, 8, 8, 8);
    self.autocapitalizationType = UITextAutocapitalizationTypeNone;
    self.autocorrectionType = UITextAutocorrectionTypeNo;
    self.spellCheckingType = UITextSpellCheckingTypeNo;

    [self initKeyboardToolbar];

    if (@available(iOS 9.0, *)) {
        self.inputAssistantItem.leadingBarButtonGroups = @[];
        self.inputAssistantItem.trailingBarButtonGroups = @[];
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
    UIView *accessoryView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.bounds.size.width, 44)];
    if (@available(iOS 13.0, *)) {
        accessoryView.backgroundColor = [UIColor secondarySystemBackgroundColor];
    } else {
        accessoryView.backgroundColor = [UIColor colorWithWhite:0.95 alpha:1.0];
    }
    accessoryView.autoresizingMask = UIViewAutoresizingFlexibleWidth;

    UIScrollView *scrollView = [[UIScrollView alloc] init];
    scrollView.translatesAutoresizingMaskIntoConstraints = NO;
    scrollView.showsHorizontalScrollIndicator = NO;
    [accessoryView addSubview:scrollView];

    UIStackView *stackView = [[UIStackView alloc] init];
    stackView.translatesAutoresizingMaskIntoConstraints = NO;
    stackView.axis = UILayoutConstraintAxisHorizontal;
    stackView.spacing = 10;
    stackView.layoutMargins = UIEdgeInsetsMake(0, 10, 0, 10);
    stackView.layoutMarginsRelativeArrangement = YES;
    [scrollView addSubview:stackView];

    // Edit buttons
    UIButton *undoButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [undoButton addTarget:self action:@selector(undo:) forControlEvents:UIControlEventTouchUpInside];
    undoButton.tintColor = self.tintColor;

    UIButton *redoButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [redoButton addTarget:self action:@selector(redo:) forControlEvents:UIControlEventTouchUpInside];
    redoButton.tintColor = self.tintColor;

    UIButton *copyButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [copyButton addTarget:self action:@selector(copy:) forControlEvents:UIControlEventTouchUpInside];
    copyButton.tintColor = self.tintColor;

    UIButton *pasteButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [pasteButton addTarget:self action:@selector(paste:) forControlEvents:UIControlEventTouchUpInside];
    pasteButton.tintColor = self.tintColor;

    if (@available(iOS 13.0, *)) {
        [undoButton setImage:[UIImage systemImageNamed:@"arrow.uturn.backward"] forState:UIControlStateNormal];
        [redoButton setImage:[UIImage systemImageNamed:@"arrow.uturn.forward"] forState:UIControlStateNormal];
        [copyButton setImage:[UIImage systemImageNamed:@"doc.on.doc"] forState:UIControlStateNormal];
        [pasteButton setImage:[UIImage systemImageNamed:@"doc.on.clipboard"] forState:UIControlStateNormal];
    } else {
        [undoButton setTitle:@"Undo" forState:UIControlStateNormal];
        [redoButton setTitle:@"Redo" forState:UIControlStateNormal];
        [copyButton setTitle:@"Copy" forState:UIControlStateNormal];
        [pasteButton setTitle:@"Paste" forState:UIControlStateNormal];
    }

    [stackView addArrangedSubview:undoButton];
    [stackView addArrangedSubview:redoButton];
    [stackView addArrangedSubview:copyButton];
    [stackView addArrangedSubview:pasteButton];

    NSArray *keys = @[@"_", @"(", @",", @")", @"<", @"=", @">", @"+", @"-", @"*", @"/", @".", @":", @"'", @"\"", @"$"];

    for (NSString *key in keys)
    {
        UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
        [button setTitle:key forState:UIControlStateNormal];
        [button addTarget:self action:@selector(onSpecialKeyTapped:) forControlEvents:UIControlEventTouchUpInside];
        button.titleLabel.font = [UIFont systemFontOfSize:20 weight:UIFontWeightLight];
        button.tintColor = self.tintColor;
        [stackView addArrangedSubview:button];
    }

    UIButton *doneButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [doneButton addTarget:self action:@selector(onKeyboardDoneTapped:) forControlEvents:UIControlEventTouchUpInside];
    doneButton.tintColor = self.tintColor;
    if (@available(iOS 13.0, *)) {
        UIImageSymbolConfiguration *config = [UIImageSymbolConfiguration configurationWithPointSize:22 weight:UIImageSymbolWeightLight];
        UIImage *image = [[UIImage systemImageNamed:@"keyboard.chevron.compact.down"] imageWithConfiguration:config];
        [doneButton setImage:image forState:UIControlStateNormal];
    } else {
        [doneButton setTitle:@"Done" forState:UIControlStateNormal];
        doneButton.titleLabel.font = [UIFont boldSystemFontOfSize:doneButton.titleLabel.font.pointSize];
    }
    [stackView addArrangedSubview:doneButton];

    [NSLayoutConstraint activateConstraints:@[
        [scrollView.leadingAnchor constraintEqualToAnchor:accessoryView.leadingAnchor],
        [scrollView.topAnchor constraintEqualToAnchor:accessoryView.topAnchor],
        [scrollView.bottomAnchor constraintEqualToAnchor:accessoryView.bottomAnchor],
        [scrollView.trailingAnchor constraintEqualToAnchor:accessoryView.trailingAnchor],

        [stackView.leadingAnchor constraintEqualToAnchor:scrollView.contentLayoutGuide.leadingAnchor],
        [stackView.trailingAnchor constraintEqualToAnchor:scrollView.contentLayoutGuide.trailingAnchor],
        [stackView.topAnchor constraintEqualToAnchor:scrollView.contentLayoutGuide.topAnchor],
        [stackView.bottomAnchor constraintEqualToAnchor:scrollView.contentLayoutGuide.bottomAnchor],
        [stackView.heightAnchor constraintEqualToAnchor:scrollView.heightAnchor]
    ]];

    self.inputAccessoryView = accessoryView;
}

- (void)undo:(id)sender
{
    [self.undoManager undo];
}

- (void)redo:(id)sender
{
    [self.undoManager redo];
}

- (void)onSpecialKeyTapped:(id)sender
{
    NSString *textToInsert = nil;
    if ([sender isKindOfClass:[UIButton class]]) {
        textToInsert = [sender currentTitle];
    }

    if (textToInsert) {
        [self insertCheckedText:textToInsert];
    }
}

- (void)onKeyboardDoneTapped:(id)sender
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

// BASIC syntax highlighting (standalone, not called automatically)
- (void)applyBasicSyntaxHighlighting {
    NSArray *keywords = @[ @"PRINT", @"IF", @"THEN", @"ELSE", @"FOR", @"TO", @"NEXT", @"GOTO", @"GOSUB", @"RETURN", @"END", @"REM", @"INPUT", @"LET", @"DIM", @"READ", @"DATA", @"RESTORE", @"ON", @"STOP", @"DEF", @"POKE", @"PEEK", @"CALL", @"SUB", @"FUNCTION", @"WHILE", @"WEND", @"DO", @"LOOP", @"UNTIL", @"STEP" ];
    UIColor *keywordColor = [UIColor colorWithRed:0.2 green:0.2 blue:0.8 alpha:1.0];
    UIColor *numberColor = [UIColor colorWithRed:0.8 green:0.2 blue:0.2 alpha:1.0];
    UIColor *stringColor = [UIColor colorWithRed:0.2 green:0.6 blue:0.2 alpha:1.0];
    UIColor *commentColor = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
    UIColor *defaultColor = [UIColor labelColor];
    UIFont *font = self.font ?: [UIFont monospacedSystemFontOfSize:14 weight:UIFontWeightRegular];
    NSString *text = self.text ?: @"";
    NSMutableAttributedString *attributed = [[NSMutableAttributedString alloc] initWithString:text attributes:@{NSFontAttributeName: font, NSForegroundColorAttributeName: defaultColor}];
    // Strings
    NSRegularExpression *stringRegex = [NSRegularExpression regularExpressionWithPattern:@"\"[^\"]*\"" options:0 error:nil];
    NSArray<NSTextCheckingResult *> *stringMatches = [stringRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];
    for (NSTextCheckingResult *match in stringMatches) {
        [attributed addAttribute:NSForegroundColorAttributeName value:stringColor range:match.range];
    }
    // Comments (REM ... or ' ...)
    NSRegularExpression *remRegex = [NSRegularExpression regularExpressionWithPattern:@"REM.*|'.*" options:NSRegularExpressionCaseInsensitive error:nil];
    NSArray<NSTextCheckingResult *> *remMatches = [remRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];
    for (NSTextCheckingResult *match in remMatches) {
        [attributed addAttribute:NSForegroundColorAttributeName value:commentColor range:match.range];
    }
    // Numbers (not inside strings)
    NSRegularExpression *numberRegex = [NSRegularExpression regularExpressionWithPattern:@"\\b[0-9]+(\\.[0-9]+)?\\b" options:0 error:nil];
    NSArray<NSTextCheckingResult *> *numberMatches = [numberRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];
    for (NSTextCheckingResult *match in numberMatches) {
        BOOL inString = NO;
        for (NSTextCheckingResult *stringMatch in stringMatches) {
            if (NSIntersectionRange(match.range, stringMatch.range).length > 0) {
                inString = YES;
                break;
            }
        }
        if (!inString) {
            [attributed addAttribute:NSForegroundColorAttributeName value:numberColor range:match.range];
        }
    }
    // Keywords (not inside strings or comments)
    for (NSString *keyword in keywords) {
        NSString *pattern = [NSString stringWithFormat:@"\\b%@\\b", keyword];
        NSRegularExpression *keywordRegex = [NSRegularExpression regularExpressionWithPattern:pattern options:NSRegularExpressionCaseInsensitive error:nil];
        NSArray<NSTextCheckingResult *> *keywordMatches = [keywordRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];
        for (NSTextCheckingResult *match in keywordMatches) {
            BOOL inStringOrComment = NO;
            for (NSTextCheckingResult *stringMatch in stringMatches) {
                if (NSIntersectionRange(match.range, stringMatch.range).length > 0) {
                    inStringOrComment = YES;
                    break;
                }
            }
            for (NSTextCheckingResult *remMatch in remMatches) {
                if (NSIntersectionRange(match.range, remMatch.range).length > 0) {
                    inStringOrComment = YES;
                    break;
                }
            }
            if (!inStringOrComment) {
                [attributed addAttribute:NSForegroundColorAttributeName value:keywordColor range:match.range];
            }
        }
    }
    // Set the attributed text (preserve selection)
    NSRange selectedRange = self.selectedRange;
    self.attributedText = attributed;
    self.selectedRange = selectedRange;
}

@end
