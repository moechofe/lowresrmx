//
//  UIViewController+LowResCoder.h
//  Pixels
//
//  Created by Timo Kloss on 1/6/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import <UIKit/UIKit.h>

#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

@interface UIViewController (LowResCoder)

- (void)showAlertWithTitle:(NSString *)title message:(NSString *)message block:(void (^)(void))block;
- (void)showConfirmAlertWithTitle:(NSString *)title message:(NSString *)message block:(void (^)(void))block;

- (void)presentInNavigationViewController:(UIViewController *)vc;

@end
