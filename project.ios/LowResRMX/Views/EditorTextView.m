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

- (void)awakeFromNib {
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

// New method with range
- (void)applyColoration:(NSInteger)mode inRange:(NSRange)range {
	switch (mode) {
	case 1:
	[self applyBasicSyntaxHighlightingAsyncInRange:range];
		break;
	default:
		break;
	}
}

// Old method for compatibility
- (void)applyColoration:(NSInteger)mode {
	[self applyColoration:mode inRange:NSMakeRange(0, self.text.length)];
}

// New async highlighting for a range with cancellation token
- (void)applyBasicSyntaxHighlightingAsyncInRange:(NSRange)range {
	static NSUInteger syntaxHighlightingToken = 0;
	syntaxHighlightingToken++;
	if (syntaxHighlightingToken > 1000000000) syntaxHighlightingToken = 1;
	NSUInteger currentToken = syntaxHighlightingToken;

	NSString *text = self.text;
	if (range.location == NSNotFound || NSMaxRange(range) > text.length)
		return;

	// Expand range to full lines, and one line above and below
	NSRange expandedRange = [text lineRangeForRange:range];
	// One line above
	if (expandedRange.location > 0) {
		NSRange prevLineRange = [text lineRangeForRange:NSMakeRange(expandedRange.location - 1, 0)];
		expandedRange.location = prevLineRange.location;
		expandedRange.length += prevLineRange.length;
	}
	// One line below
	NSUInteger afterRange = NSMaxRange(expandedRange);
	if (afterRange < text.length) {
		NSRange nextLineRange = [text lineRangeForRange:NSMakeRange(afterRange, 0)];
		expandedRange.length += nextLineRange.length;
	}

	UIFont *font = self.font ?: [UIFont monospacedSystemFontOfSize:14 weight:UIFontWeightRegular];
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		NSArray *updates = [self computeSyntaxHighlightingUpdatesForText:text range:expandedRange];
		dispatch_async(dispatch_get_main_queue(), ^{
			// Only apply if this is the latest request
			if (currentToken == syntaxHighlightingToken) {
				[self applyUpdates:updates toStorage:self.textStorage range:expandedRange font:font token:currentToken latestTokenPtr:&syntaxHighlightingToken];
			}
		});
	});
}

- (void)initKeyboardToolbar {
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
	scrollView.alwaysBounceHorizontal = NO;
	scrollView.bounces = NO;
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

	[stackView addArrangedSubview:doneButton];
	[stackView addArrangedSubview:undoButton];
	[stackView addArrangedSubview:redoButton];
	[stackView addArrangedSubview:copyButton];
	[stackView addArrangedSubview:pasteButton];

	NSArray *keys = @[
		@"_", @"(", @",", @")", @"<", @"=", @">", @"+", @"-", @"*", @"/", @"\\", @".", @":", @"'", @"\"", @"$"
	];

	for (NSString *key in keys) {
		UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
		[button setTitle:key forState:UIControlStateNormal];
		[button addTarget:self action:@selector(onSpecialKeyTapped:) forControlEvents:UIControlEventTouchUpInside];
		button.titleLabel.font = [UIFont systemFontOfSize:20 weight:UIFontWeightLight];
		button.tintColor = self.tintColor;
		[stackView addArrangedSubview:button];
	}

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

- (void)undo:(id)sender {
	[self.undoManager undo];
}

- (void)redo:(id)sender {
	[self.undoManager redo];
}

- (void)onSpecialKeyTapped:(id)sender {
	NSString *textToInsert = nil;
	if ([sender isKindOfClass:[UIButton class]]) {
		textToInsert = [sender currentTitle];
	}

	if (textToInsert) {
		[self insertCheckedText:textToInsert];
	}
}

- (void)onKeyboardDoneTapped:(id)sender {
	[self resignFirstResponder];
}

- (BOOL)canPerformAction:(SEL)action withSender:(id)sender {
	if (action == @selector(help:)) {
		return self.selectedRange.length > 0 && self.selectedRange.length <= 20;
	} else if (action == @selector(indentRight:) || action == @selector (indentLeft:)) {
		return self.isEditable;
	} else if (action == @selector(copy:) || action == @selector(paste:) || action == @selector(cut:) || action == @selector(delete:) || action == @selector(select:) || action == @selector(selectAll:)) {
		return [super canPerformAction:action withSender:sender];
	}
	return NO;
}

- (void)help:(id)sender {
	[self.editorDelegate editorTextView:self didSelectHelpWithRange:self.selectedRange];
	// NSLog(@"Help for range: %@", NSStringFromRange(self.selectedRange));
}

- (void)indentRight:(id)sender {
	[self indentToRight:YES];
}

- (void)indentLeft:(id)sender {
	[self indentToRight:NO];
}

- (void)indentToRight:(BOOL)right {
	NSRange originalRange = [self.text lineRangeForRange:self.selectedRange];
	NSRange finalRange = originalRange;
	NSMutableString *subtext = [[self.text substringWithRange:originalRange] mutableCopy];
	NSInteger pos = 0;

	NSCharacterSet *spacesSet = [NSCharacterSet whitespaceCharacterSet];
	NSCharacterSet *newlineSet = [NSCharacterSet newlineCharacterSet];

	while (pos < subtext.length) {
		if (right) {
			[subtext insertString:@"  " atIndex:pos];
			finalRange.length += 2;
		} else {
			NSInteger num = 0;
			for (NSInteger ci = pos; ci < pos + 2 && ci < subtext.length; ci++) {
				unichar character = [subtext characterAtIndex:ci];
				if ([spacesSet characterIsMember:character]) {
					num++;
				} else if ([newlineSet characterIsMember:character]) {
					break;
				}
			}
			if (num > 0) {
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
	if (finalRange.location + finalRange.length < self.text.length) {
		finalRange.length--;
	}
	self.selectedRange = finalRange;
	[self scrollRangeToVisible:self.selectedRange];
}

- (void)insertCheckedText:(NSString *)text {
	if (!self.isEditable)
		return;
	if (!self.delegate || [self.delegate textView:self shouldChangeTextInRange:self.selectedRange replacementText:text]) {
		[self insertText:text];
	}
}

- (NSArray *)computeSyntaxHighlightingUpdatesForText:(NSString *)text range:(NSRange)range {
	NSMutableArray *updates = [NSMutableArray array];

	NSArray *keywords = @[
		@"ABS", @"ADD", @"AND", @"ASC", @"AT", @"ATAN", @"ATTR", @"BG", @"BIN", @"CALL", @"CEIL", @"CELL", @"CELL\\.A", @"CELL\\.C", @"CHAR", @"CHR", @"CLAMP", @"CLS", @"CLW", @"COLOR", @"COMPAT", @"COPY", @"COS", @"CURSOR\\.X", @"CURSOR\\.Y", @"DATA", @"DEC", @"DIM", @"DMA", @"DO", @"EASE", @"ELSE", @"EMITTER", @"END", @"ENVELOPE", @"EXIT", @"EXP", @"FILE", @"FILES", @"FILL", @"FLIP", @"FLOOR", @"FONT", @"FOR", @"FSIZE", @"GLOBAL", @"GOSUB", @"GOTO", @"HAPTIC", @"HEX", @"HIT", @"IF", @"INC", @"INKEY", @"INPUT", @"INSTR", @"INT", @"KEYBOARD", @"LEFT", @"LEN", @"LET", @"LFO", @"LFO\\.A", @"LOAD", @"LOCATE", @"LOG", @"LOOP", @"MAX", @"MCELL", @"MCELL\\.A", @"MCELL\\.C", @"MESSAGE", @"MID", @"MIN", @"MOD", @"MUSIC", @"NEXT", @"NOT", @"NUMBER", @"OFF", @"ON", @"OR", @"PAL", @"PALETTE", @"PARTICLE", @"PAUSE", @"PEEK", @"PEEKL", @"PEEKW", @"PI", @"PLAY", @"POKE", @"POKEL", @"POKEW", @"PRINT", @"PRIO", @"RANDOMIZE", @"RASTER", @"READ", @"REPEAT", @"RESTORE", @"RETURN", @"RIGHT", @"RND", @"ROL", @"ROM", @"ROR", @"SAFE\\.B", @"SAFE\\.L", @"SAFE\\.R", @"SAFE\\.T", @"SAVE", @"SCROLL", @"SCROLL\\.X", @"SCROLL\\.Y", @"SGN", @"SHOWN\\.H", @"SHOWN\\.W", @"SIN", @"SIZE", @"SKIP", @"SOUND", @"SOURCE", @"SPRITE", @"SPRITE\\.A", @"SPRITE\\.C", @"SPRITE\\.X", @"SPRITE\\.Y", @"SQR", @"STEP", @"STOP", @"STR", @"SUB", @"SWAP", @"SYSTEM", @"TAN", @"TAP", @"TEXT", @"THEN", @"TIMER", @"TINT", @"TO", @"TOUCH", @"TOUCH\\.X", @"TOUCH\\.Y", @"TOUCH\\.TAP", @"TOUCH\\.DRAG", @"TOUCH\\.LONG", @"TOUCH\\.CHANGE", @"TOUCH\\.PX", @"TOUCH\\.PY", @"TRACE", @"TRACK", @"UBOUND", @"UNTIL", @"VAL", @"VBL", @"VIEW", @"VOLUME", @"WAIT", @"WAVE", @"WEND", @"WHILE", @"WINDOW", @"XOR", ];

	UIColor *keywordColor = [UIColor colorWithRed:0.5059 green:0.2392 blue:0.6118 alpha:1.0];
	UIColor *numberColor = [UIColor colorWithRed:0.6471 green:0.1137 blue:0.1765 alpha:1.0];
	UIColor *stringColor = [UIColor colorWithRed:0.6471 green:0.1137 blue:0.1765 alpha:1.0];
	UIColor *commentColor = [UIColor colorWithRed:0.4667 green:0.4627 blue:0.4824 alpha:1.0];
	UIColor *labelColor = [UIColor colorWithRed:0.7765 green:0.2745 blue:0.0000 alpha:1.0];
	UIColor *subsColor = [UIColor colorWithRed:0.1020 green:0.3725 blue:0.7059 alpha:1.0];

	// End of code, start of DATA block
	NSRegularExpression *endOfCodeRegex = [NSRegularExpression regularExpressionWithPattern:@"^#[0-9]+:" options:NSRegularExpressionAnchorsMatchLines error:nil];
	NSArray<NSTextCheckingResult *> *endOfCodeMatches = [endOfCodeRegex matchesInString:text options:0 range:range];

	// After a certain point, we are in the DATA block and should not apply syntax
	// highlighting. Find the earliest match in the range.
	if (endOfCodeMatches.count > 0) {
		NSRange dataStartRange = endOfCodeMatches[0].range;
		if (NSLocationInRange(dataStartRange.location, range)) {
			range.length = dataStartRange.location - range.location;
		} else {
			// No DATA block in range, do nothing
		}
	}

	// Strings
	NSRegularExpression *stringRegex = [NSRegularExpression regularExpressionWithPattern:@"\"[^\"]*\"" options:0 error:nil];
	NSArray<NSTextCheckingResult *> *stringMatches = [stringRegex matchesInString:text options:0 range:range];
	for (NSTextCheckingResult *match in stringMatches) {
		[updates addObject:@{ @"range" : [NSValue valueWithRange:match.range], @"color" : stringColor
		}];
	}

	// Comments (' ...)
	NSRegularExpression *remRegex = [NSRegularExpression regularExpressionWithPattern:@"(^\\s*|\\s+:\\s*)'.*" options:(NSRegularExpressionCaseInsensitive|NSRegularExpressionAnchorsMatchLines) error:nil];
	NSArray<NSTextCheckingResult *> *remMatches = [remRegex matchesInString:text options:0 range:range];
	for (NSTextCheckingResult *match in remMatches) {
		[updates addObject:@{ @"range" : [NSValue valueWithRange:match.range], @"color" : commentColor }];
	}

	// Numbers (not inside strings)
	NSRegularExpression *numberRegex = [NSRegularExpression regularExpressionWithPattern:@"\\b[0-9]+(\\.[0-9]+)?\\b" options:0 error:nil];
	NSArray<NSTextCheckingResult *> *numberMatches = [numberRegex matchesInString:text options:0 range:range];
	for (NSTextCheckingResult *match in numberMatches) {
		BOOL inString = NO;
		for (NSTextCheckingResult *stringMatch in stringMatches) {
			if (NSIntersectionRange(match.range, stringMatch.range).length > 0) {
				inString = YES;
				break;
			}
		}
		BOOL inComment = NO;
		for (NSTextCheckingResult *remMatch in remMatches) {
			if (NSIntersectionRange(match.range, remMatch.range).length > 0) {
				inComment = YES;
				break;
			}
		}
		if (!inString && !inComment) {
			[updates addObject:@{ @"range" : [NSValue valueWithRange:match.range], @"color" : numberColor }];
		}
	}

	// Keywords (not inside strings or comments)
	for (NSString *keyword in keywords) {
		NSString *pattern = [NSString stringWithFormat:@"\\b%@\\b", keyword];
		NSRegularExpression *keywordRegex = [NSRegularExpression regularExpressionWithPattern:pattern options:NSRegularExpressionCaseInsensitive error:nil];
		NSArray<NSTextCheckingResult *> *keywordMatches = [keywordRegex matchesInString:text options:0 range:range];
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
				[updates addObject:@{ @"range" : [NSValue valueWithRange:match.range],
					@"color" : keywordColor
				}];
			}
		}
	}

	// Labels: find lines like 'label:' at the start of a line
	NSRegularExpression *labelRegex = [NSRegularExpression regularExpressionWithPattern:@"^([A-Za-z_][A-Za-z0-9_]*)\\s*:" options:NSRegularExpressionAnchorsMatchLines error:nil];
	NSMutableSet<NSString *> *labelSet = [NSMutableSet set];
	// Scan all labels to ensure GOTO targets are correct, even if outside range
	NSArray<NSTextCheckingResult *> *labelMatches = [labelRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];

	for (NSTextCheckingResult *match in labelMatches) {
		if (match.numberOfRanges > 1) {
			NSRange idRange = [match rangeAtIndex:1];
			if (NSIntersectionRange(idRange, range).length > 0) {
				[updates addObject:@{ @"range" : [NSValue valueWithRange:idRange], @"color" : labelColor }];
			}
			NSString *labelName = [text substringWithRange:idRange];
			[labelSet addObject:labelName.uppercaseString];
		}
	}

	// Colorize GOTO/GOSUB targets
	NSRegularExpression *gotoRegex = [NSRegularExpression regularExpressionWithPattern: @"\\b(GOTO|GOSUB)\\s+([A-Za-z_][A-Za-z0-9_]*)\\b" options:NSRegularExpressionCaseInsensitive error:nil];
	NSArray<NSTextCheckingResult *> *gotoMatches = [gotoRegex matchesInString:text options:0 range:range];

	for (NSTextCheckingResult *match in gotoMatches) {
		BOOL inString = NO;
		for (NSTextCheckingResult *stringMatch in stringMatches) {
			if (NSIntersectionRange(match.range, stringMatch.range).length > 0) {
				inString = YES;
				break;
			}
		}
		BOOL inComment = NO;
		for (NSTextCheckingResult *remMatch in remMatches) {
			if (NSIntersectionRange(match.range, remMatch.range).length > 0) {
				inComment = YES;
				break;
			}
		}
		if (inString || inComment)
			continue;
		if (match.numberOfRanges > 2) {
			NSRange labelRange = [match rangeAtIndex:2];
			NSString *target = [text substringWithRange:labelRange];
			if ([labelSet containsObject:target.uppercaseString]) {
				[updates addObject:@{ @"range" : [NSValue valueWithRange:labelRange], @"color" : labelColor }];
			}
		}
	}

	// Colorize ON var GOTO/GOSUB label1, label2, ...
	NSRegularExpression *onGotoGosubRegex = [NSRegularExpression regularExpressionWithPattern:@"\\bON\\s+[A-Za-z_][A-Za-z0-9_\\(\\)]*\\s+(GOTO|GOSUB)\\s+([A-Za-z_][A-Za-z0-9_]*(?:\\s*,\\s*[A-Za-z_][A-Za-z0-9_]*)*)" options:NSRegularExpressionCaseInsensitive error:nil];
	NSArray<NSTextCheckingResult *> *onGotoGosubMatches = [onGotoGosubRegex matchesInString:text options:0 range:range];
	for (NSTextCheckingResult *match in onGotoGosubMatches) {
		BOOL inString = NO;
		for (NSTextCheckingResult *stringMatch in stringMatches) {
			if (NSIntersectionRange(match.range, stringMatch.range).length > 0) {
				inString = YES;
				break;
			}
		}
		BOOL inComment = NO;
		for (NSTextCheckingResult *remMatch in remMatches) {
			if (NSIntersectionRange(match.range, remMatch.range).length > 0) {
				inComment = YES;
				break;
			}
		}
		if (inString || inComment)
			continue;
		if (match.numberOfRanges > 2) {
			NSRange labelsRange = [match rangeAtIndex:2];
			NSString *labelsString = [text substringWithRange:labelsRange];
			NSArray *labels = [labelsString componentsSeparatedByString:@","];
			// Scan within labelsString, map to full text using labelsRange.location
			NSUInteger localPos = 0;
			for (NSString *label in labels) {
				NSString *trimmedLabel = [label stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
				// Find the range of trimmedLabel in labelsString starting from localPos
				NSRange searchRange = NSMakeRange(localPos, labelsString.length - localPos);
				NSRange foundRange = [labelsString rangeOfString:trimmedLabel options:0 range:searchRange];
				if (foundRange.location != NSNotFound) {
					NSRange labelRange = NSMakeRange(labelsRange.location + foundRange.location, trimmedLabel.length);
					if ([labelSet containsObject:trimmedLabel.uppercaseString]) {
						[updates addObject:@{ @"range" : [NSValue valueWithRange:labelRange], @"color" : labelColor }];
					}
					localPos = foundRange.location + foundRange.length;
				}
				// Move past comma and whitespace
				while (localPos < labelsString.length && ([[NSCharacterSet whitespaceCharacterSet] characterIsMember:[labelsString characterAtIndex:localPos]] || [labelsString characterAtIndex:localPos] == ',')) {
					localPos++;
				}
			}
		}
	}

	// Subs: find lines like 'SUB name'
	NSRegularExpression *subRegex = [NSRegularExpression regularExpressionWithPattern:@"^\\s*SUB\\s+([A-Za-z_][A-Za-z0-9_]*)" options:(NSRegularExpressionCaseInsensitive|NSRegularExpressionAnchorsMatchLines) error:nil];
	NSArray<NSTextCheckingResult *> *subMatches = [subRegex matchesInString:text options:0 range:NSMakeRange(0, text.length)];
	for (NSTextCheckingResult *match in subMatches) {
		if (match.numberOfRanges > 1) {
			NSRange idRange = [match rangeAtIndex:1];
			if (NSIntersectionRange(idRange, range).length > 0) {
				[updates addObject:@{ @"range" : [NSValue valueWithRange:idRange], @"color" : subsColor }];
			}
		}
	}

	// Cololize CALL targets that match a SUB
	NSRegularExpression *callRegex = [NSRegularExpression regularExpressionWithPattern:@"\\bCALL\\s+([A-Za-z_][A-Za-z0-9_]*)" options:NSRegularExpressionCaseInsensitive error:nil];
	NSArray<NSTextCheckingResult *> *callMatches = [callRegex matchesInString:text options:0 range:range];
	for (NSTextCheckingResult *match in callMatches) {
		if (match.numberOfRanges > 1) {
			NSRange idRange = [match rangeAtIndex:1];
			if (NSIntersectionRange(idRange, range).length > 0) {
				NSString *target = [text substringWithRange:idRange];
				// Check if there's a SUB with this name
				BOOL hasSub = NO;
				for (NSTextCheckingResult *subMatch in subMatches) {
					if (subMatch.numberOfRanges > 1) {
						NSRange subIdRange = [subMatch rangeAtIndex:1];
						NSString *subName = [text substringWithRange:subIdRange];
						if ([subName caseInsensitiveCompare:target] == NSOrderedSame) {
							hasSub = YES;
							break;
						}
					}
				}
				if (hasSub) {
					[updates addObject:@{ @"range" : [NSValue valueWithRange:idRange], @"color" : subsColor }];
				}
			}
		}
	}

	return updates;
}


// Apply updates, aborting if token is outdated
- (void)applyUpdates:(NSArray *)updates toStorage:(NSTextStorage *)storage range:(NSRange)range font:(UIFont *)font token:(NSUInteger)token latestTokenPtr:(NSUInteger *)latestTokenPtr {
	[storage beginEditing];
	UIColor *defaultColor = [UIColor blackColor];
	if (@available(iOS 13.0, *)) {
		defaultColor = [UIColor labelColor];
	}
	[storage removeAttribute:NSForegroundColorAttributeName range:range];
	[storage addAttribute:NSForegroundColorAttributeName value:defaultColor range:range];
	[storage addAttribute:NSFontAttributeName value:font range:range];

	for (NSDictionary *update in updates) {
		if (token != *latestTokenPtr) {
			// Abort if a new request has started
			[storage endEditing];
			return;
		}
		[storage addAttribute:NSForegroundColorAttributeName value:update[@"color"] range:[update[@"range"] rangeValue]];
	}
	[storage endEditing];
}

@end
