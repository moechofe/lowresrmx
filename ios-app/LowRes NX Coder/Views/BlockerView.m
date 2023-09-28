//
//  BlockerView.m
//  Pixels
//
//  Created by Timo Kloss on 27/11/16.
//  Copyright © 2016 Inutilis Software. All rights reserved.
//

#import "BlockerView.h"
#import "AppStyle.h"

static BlockerView *_currentInstance;

@interface BlockerView()
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
    UIView *container = [UIApplication sharedApplication].keyWindow;
    _currentInstance.frame = container.bounds;
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
//    self.activityIndicatorView.color = [AppStyle whiteColor];
//    self.backgroundColor = [[AppStyle darkGrayColor] colorWithAlphaComponent:0.25];
}

@end
