//
//  UIViewController+LowResCoder.m
//  Pixels
//
//  Created by Timo Kloss on 1/6/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import "UIViewController+LowResCoder.h"

@implementation UIViewController (LowResCoder)

- (void)showAlertWithTitle:(NSString *)title message:(NSString *)message block:(void (^)(void))block
{
    if (self.view.superview)
    {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
        [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {
            if (block)
            {
                block();
            }
        }]];
        [self presentViewController:alert animated:YES completion:nil];
    }
}

- (void)showConfirmAlertWithTitle:(NSString *)title message:(NSString *)message block:(void (^)(void))block
{
    UIAlertController *alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {
        block();
    }]];
    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
}

- (void)presentInNavigationViewController:(UIViewController *)vc;
{
    UINavigationController *nav = [[UINavigationController alloc] initWithRootViewController:vc];
    nav.modalPresentationStyle = vc.modalPresentationStyle;
    nav.modalTransitionStyle = vc.modalTransitionStyle;
    [self presentViewController:nav animated:YES completion:nil];
}

@end
