//
//  SearchToolbar.h
//  Pixels
//
//  Created by Timo Kloss on 5/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol SearchToolbarDelegate;

@interface SearchToolbar : UIView

@property (weak) id<SearchToolbarDelegate> searchDelegate;
- (void)activate;
- (void)updateFontSize:(CGFloat)fontSize;

@end

@protocol SearchToolbarDelegate <NSObject>

- (void)searchToolbar:(SearchToolbar *)searchToolbar didSearch:(NSString *)findText backwards:(BOOL)backwards;
- (void)searchToolbar:(SearchToolbar *)searchToolbar didReplace:(NSString *)findText with:(NSString *)replaceText;

@end
