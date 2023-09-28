//
//  HelpSplitViewController.h
//  Pixels
//
//  Created by Timo Kloss on 1/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import <UIKit/UIKit.h>

@class HelpContent, HelpTextViewController;

@interface HelpSplitViewController : UISplitViewController

@property (readonly) UINavigationController *detailNavigationController;

- (void)showChapter:(NSString *)chapter;

@end
