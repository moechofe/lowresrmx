//
//  EditorTextView.h
//  Pixels
//
//  Created by Timo Kloss on 25/2/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol EditorTextViewDelegate;

NS_ASSUME_NONNULL_BEGIN
@interface EditorTextView : UITextView

@property (readonly, nullable) UIToolbar *keyboardToolbar;
@property (weak, nullable) id<EditorTextViewDelegate> editorDelegate;

@end

@protocol EditorTextViewDelegate <NSObject>

- (void)editorTextView:(EditorTextView *)editorTextView didSelectHelpWithRange:(NSRange)range;

@end
NS_ASSUME_NONNULL_END
