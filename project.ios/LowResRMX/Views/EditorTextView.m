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

#import "EditorTextView.h"
#import "core_syntax.h"

static dispatch_queue_t syntaxQueue(void) {
	static dispatch_queue_t queue;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		queue = dispatch_queue_create("it.ro.ret.ios.LowResRMX.syntax", DISPATCH_QUEUE_SERIAL);
	});
	return queue;
}

static NSArray<UIColor *> *syntaxColors(void) {
	static NSArray<UIColor *> *colors;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		UIColor *fallback = [UIColor labelColor];
		colors = @[
			[UIColor colorNamed:@"syntax_keywoard"] ?: fallback, // SyntaxKeyword
			[UIColor colorNamed:@"syntax_number"] ?: fallback,   // SyntaxNumber
			[UIColor colorNamed:@"syntax_string"] ?: fallback,   // SyntaxString
			[UIColor colorNamed:@"syntax_comment"] ?: fallback,  // SyntaxComment
			[UIColor colorNamed:@"syntax_label"] ?: fallback,    // SyntaxLabel
			[UIColor colorNamed:@"syntax_subs"] ?: fallback      // SyntaxSub
		];
	});
	return colors;
}

typedef struct {
	const char *bytes;
	NSUInteger byteCursor;
	NSUInteger utf16Cursor;
	BOOL isAscii; // then the mapping is the identity and the walk is skipped
} Utf8ToUtf16Cursor;

static NSUInteger utf16OffsetForByteOffset(Utf8ToUtf16Cursor *cursor, NSUInteger byteOffset) {
	if (cursor->isAscii) {
		return byteOffset;
	}
	while (cursor->byteCursor < byteOffset) {
		unsigned char lead = (unsigned char)cursor->bytes[cursor->byteCursor];
		NSUInteger byteLength = (lead < 0x80) ? 1 : (lead < 0xE0) ? 2 : (lead < 0xF0) ? 3 : 4;
		cursor->byteCursor += byteLength;
		// Only astral characters need a surrogate pair.
		cursor->utf16Cursor += (byteLength == 4) ? 2 : 1;
	}
	return cursor->utf16Cursor;
}

@interface EditorTextView ()
+ (NSArray *)syntaxUpdatesForText:(NSString *)text inRange:(NSRange)range;
@end

@implementation EditorTextView

- (void)awakeFromNib {
	[super awakeFromNib];

	self.textContainerInset = UIEdgeInsetsMake(8, 8, 8, 8);
	self.autocapitalizationType = UITextAutocapitalizationTypeNone;
	self.autocorrectionType = UITextAutocorrectionTypeNo;
	self.spellCheckingType = UITextSpellCheckingTypeNo;

	[self initKeyboardToolbar];

	// Resolve the syntax colours here so the one-time UIColor lookup happens on
	// the main thread; every later read comes from the cache on syntaxQueue().
	syntaxColors();

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
	dispatch_async(syntaxQueue(), ^{
		NSArray *updates = [EditorTextView syntaxUpdatesForText:text inRange:expandedRange];
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

// Must run on syntaxQueue(): the struct Syntax below is shared and reused.
+ (NSArray *)syntaxUpdatesForText:(NSString *)text inRange:(NSRange)range {
	static struct Syntax syntax;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		syntax_init(&syntax);
	});

	const char *sourceCode = [text UTF8String];
	if (!sourceCode) {
		return @[];
	}

	syntax_update(&syntax, sourceCode);

	NSArray<UIColor *> *colors = syntaxColors();
	NSMutableArray *updates = [NSMutableArray arrayWithCapacity:syntax.numSpans];

	size_t byteLength = strlen(sourceCode);
	Utf8ToUtf16Cursor cursor = { sourceCode, 0, 0, (byteLength == text.length) };

	for (int i = 0; i < syntax.numSpans; i++) {
		struct SyntaxSpan span = syntax.spans[i];
		// Both offsets must be converted even when the span is discarded below, so the cursor stays in step with the span order.
		NSUInteger start = utf16OffsetForByteOffset(&cursor, span.start);
		NSUInteger end = utf16OffsetForByteOffset(&cursor, span.start + span.length);
		if (end <= start || end > text.length) {
			continue;
		}
		NSRange spanRange = NSMakeRange(start, end - start);
		if (NSIntersectionRange(spanRange, range).length == 0) {
			continue;
		}
		[updates addObject:@{ @"range":[NSValue valueWithRange:spanRange],
				      @"color" : colors[(NSUInteger)span.kind] }];
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
