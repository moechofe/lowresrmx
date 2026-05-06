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

#import "AppStyle.h"
#import "UIColor+Utils.h"
#import "BackgroundView.h"
#import "TextLabel.h"
#import "GORLabel.h"

@implementation AppStyle

+ (void)setAppearance
{
	// App tint color
	//UIWindow *window = (UIWindow *)[UIApplication sharedApplication].windows.firstObject;
	//window.tintColor = [AppStyle darkTintColor];

	// Bars
	//[UINavigationBar appearance].barTintColor = [AppStyle mediumTintColor];
	//[UINavigationBar appearance].tintColor = [AppStyle brightTintColor];
//            [UINavigationBar appearance].translucent = NO;
//            [UINavigationBar appearance].barStyle = UIBarStyleDefault;
	//[UINavigationBar appearance].titleTextAttributes = @{NSForegroundColorAttributeName: [AppStyle darkGrayColor]};
//            [[UINavigationBar appearance] setBackgroundImage:[[UIImage alloc] init] forBarMetrics:UIBarMetricsDefault];
//            [UINavigationBar appearance].shadowImage = [UIImage imageNamed:@"nav_shadow"];
//    [UIToolbar appearanceWhenContainedInInstancesOfClasses:@[[UINavigationController class]]].barTintColor = [AppStyle mediumTintColor];
//    [UIToolbar appearanceWhenContainedInInstancesOfClasses:@[[UINavigationController class]]].tintColor = [AppStyle brightTintColor];
//    [UIToolbar appearanceWhenContainedInInstancesOfClasses:@[[UINavigationController class]]].translucent = NO;
	//[UITabBar appearance].barTintColor = [AppStyle mediumDarkGrayColor];
	//[UITabBar appearance].tintColor = [AppStyle brightTintColor];
//            [UITabBar appearance].translucent = NO;
//            [UITabBar appearance].barStyle = UIBarStyleDefault;
//            [UITabBar appearance].backgroundImage = [[UIImage alloc] init];
//            [UITabBar appearance].shadowImage = [UIImage imageNamed:@"tab_shadow"];
//    [[UITabBarItem appearance] setTitleTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:[AppStyle brightColor], NSForegroundColorAttributeName, nil] forState:UIControlStateNormal];
//    [[UITabBarItem appearance] setTitleTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:[AppStyle tintColor], NSForegroundColorAttributeName, nil] forState:UIControlStateSelected];


	// Backgrounds
	//[BackgroundView appearance].backgroundColor = [AppStyle darkGrayColor];
	//[UIWebView appearance].backgroundColor = [UIColor whiteColor];
//    [UITableView appearance].backgroundColor = [AppStyle darkGrayColor];
//    [UITableViewCell appearance].backgroundColor = [AppStyle darkGrayColor];
//    [UICollectionView appearance].backgroundColor = [AppStyle darkGrayColor];
//    [UITextView appearanceWhenContainedInInstancesOfClasses:@[[UITableViewCell class]]].backgroundColor = [AppStyle darkGrayColor];

	// Texts
//    [TextLabel appearance].textColor = [AppStyle whiteColor];
//    [GORLabel appearance].textColor = [AppStyle whiteColor];
//    [UITextField appearance].textColor = [AppStyle darkGrayColor];
//    [UITextView appearance].textColor = [AppStyle whiteColor];
}
//
//+ (UIColor *)mediumTintColor
//{
//    return [UIColor colorWithHex:0x00AAAA alpha:1.0f];
//}
//
//+ (UIColor *)darkTintColor
//{
//    return [UIColor colorWithHex:0x005555 alpha:1.0f];
//}
//
//+ (UIColor *)brightTintColor
//{
//    return [UIColor colorWithHex:0x00FFFF alpha:1.0f];
//}
//
//+ (UIColor *)darkGrayColor
//{
//    return [UIColor colorWithHex:0x222222 alpha:1.0f];
//}
//
//+ (UIColor *)mediumDarkGrayColor
//{
//    return [UIColor colorWithHex:0x333333 alpha:1.0f];
//}
//
//+ (UIColor *)mediumGrayColor
//{
//    return [UIColor colorWithHex:0x555555 alpha:1.0f];
//}
//
//+ (UIColor *)whiteColor
//{
//    return [UIColor colorWithHex:0xDDDDDD alpha:1.0f];
//}

@end
