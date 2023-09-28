//
//  HelpSplitViewController.m
//  Pixels
//
//  Created by Timo Kloss on 1/10/15.
//  Copyright © 2015 Inutilis Software. All rights reserved.
//

#import "HelpSplitViewController.h"
#import "HelpTextViewController.h"

@interface HelpSplitViewController () <UISplitViewControllerDelegate>

@end

@implementation HelpSplitViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.delegate = self;
    self.preferredDisplayMode = UISplitViewControllerDisplayModeAllVisible;
    self.preferredPrimaryColumnWidthFraction = 0.3;

    _detailNavigationController = self.viewControllers.lastObject;
}

- (void)showChapter:(NSString *)chapter
{
    UINavigationController *nc = self.viewControllers.lastObject;
    if (self.viewControllers.count == 1 && nc.viewControllers.count == 1)
    {
        [self showDetailViewController:self.detailNavigationController sender:self];
    }
    
    HelpTextViewController *textViewController = (HelpTextViewController *)self.detailNavigationController.topViewController;
    textViewController.chapter = chapter;
}

@end
