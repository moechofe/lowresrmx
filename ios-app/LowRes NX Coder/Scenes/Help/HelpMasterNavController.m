//
//  HelpMasterNavController.m
//  Pixels
//
//  Created by Timo Kloss on 7/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import "HelpMasterNavController.h"
#import "HelpTableViewController.h"

@implementation HelpMasterNavController

- (void)collapseSecondaryViewController:(UIViewController *)secondaryViewController forSplitViewController:(UISplitViewController *)splitViewController
{
    [super collapseSecondaryViewController:secondaryViewController forSplitViewController:splitViewController];
    HelpTableViewController *helpVC = self.viewControllers.firstObject;
    [helpVC updateBarButtonCollapsed:YES];
}

- (UIViewController *)separateSecondaryViewControllerForSplitViewController:(UISplitViewController *)splitViewController
{
    UIViewController *vc = [super separateSecondaryViewControllerForSplitViewController:splitViewController];
    HelpTableViewController *helpVC = self.viewControllers.firstObject;
    [helpVC updateBarButtonCollapsed:NO];
    return vc;
}

@end
