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
#import "BlockerView.h"

static BlockerView *_currentInstance;

@interface BlockerView ()
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicatorView;
@end

@implementation BlockerView

+ (instancetype)view
{
	BlockerView *view = [[UINib nibWithNibName:NSStringFromClass([self class]) bundle:nil] instantiateWithOwner:nil options:nil].firstObject;
	return view;
}

+ (void)show
{
	if (!_currentInstance)
	{
		_currentInstance = [BlockerView view];
	}
	else if (_currentInstance.superview)
	{
		[_currentInstance removeFromSuperview];
	}
	UIWindow *container = nil;
	if (@available(iOS 13.0, *)) {
		for (UIScene *scene in [UIApplication sharedApplication].connectedScenes) {
			if ([scene isKindOfClass:[UIWindowScene class]] && scene.activationState == UISceneActivationStateForegroundActive) {
				UIWindowScene *windowScene = (UIWindowScene *)scene;
				for (UIWindow *window in windowScene.windows) {
					if (window.isKeyWindow) {
						container = window;
						break;
					}
				}
			}
			if (container) break;
		}
	}  _currentInstance.frame = container.bounds;
	[container addSubview:_currentInstance];
	[_currentInstance.activityIndicatorView startAnimating];
	[UIView animateWithDuration:0.3 animations:^{
		 _currentInstance.alpha = 1.0;
	 }];
}

+ (void)dismiss
{
	if (_currentInstance)
	{
		BlockerView *view = _currentInstance;
		_currentInstance = nil;
		[UIView animateWithDuration:0.3 animations:^{
			 view.alpha = 0.0;
		 } completion:^(BOOL finished) {
			 [view removeFromSuperview];
		 }];
	}
}

- (void)awakeFromNib
{
	[super awakeFromNib];
	self.alpha = 0;
}

@end
