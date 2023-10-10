//
//  Copyright (c) 2013 Luke Scott
//  https://github.com/lukescott/DraggableCollectionView
//  Distributed under MIT license
//

#import "LSCollectionViewHelper.h"
#import "UICollectionViewLayout_Warpable.h"
#import "UICollectionViewDataSource_Draggable.h"
#import "LSCollectionViewLayoutHelper.h"
#import <QuartzCore/QuartzCore.h>

static int kObservingCollectionViewLayoutContext;

#ifndef CGGEOMETRY__SUPPORT_H_
CG_INLINE CGPoint
_CGPointAdd(CGPoint point1, CGPoint point2) {
    return CGPointMake(point1.x + point2.x, point1.y + point2.y);
}
#endif

typedef NS_ENUM(NSInteger, _ScrollingDirection) {
    _ScrollingDirectionUnknown = 0,
    _ScrollingDirectionUp,
    _ScrollingDirectionDown,
    _ScrollingDirectionLeft,
    _ScrollingDirectionRight
};

@interface LSCollectionViewHelper ()
{
    NSIndexPath *lastIndexPath;
    NSIndexPath *folderDataIndexPath;
    NSIndexPath *folderVisualIndexPath;
    UIImageView *mockCell;
    CGPoint mockCenter;
    CGPoint fingerTranslation;
    CADisplayLink *timer;
    _ScrollingDirection scrollingDirection;
    BOOL canWarp;
    BOOL canScroll;
}
@property (readonly, nonatomic) LSCollectionViewLayoutHelper *layoutHelper;
@end

@implementation LSCollectionViewHelper

- (id)initWithCollectionView:(UICollectionView *)collectionView
{
    self = [super init];
    if (self) {
        _collectionView = collectionView;
        [_collectionView addObserver:self
                          forKeyPath:@"collectionViewLayout"
                             options:0
                             context:&kObservingCollectionViewLayoutContext];
        _scrollingEdgeInsets = UIEdgeInsetsMake(50.0f, 50.0f, 50.0f, 50.0f);
        _scrollingSpeed = 600.f;
        
        _longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]
                                       initWithTarget:self
                                       action:@selector(handleLongPressGesture:)];
        [_collectionView addGestureRecognizer:_longPressGestureRecognizer];
        
        _panPressGestureRecognizer = [[UIPanGestureRecognizer alloc]
                                      initWithTarget:self action:@selector(handlePanGesture:)];
        _panPressGestureRecognizer.delegate = self;

        [_collectionView addGestureRecognizer:_panPressGestureRecognizer];
        
        for (UIGestureRecognizer *gestureRecognizer in _collectionView.gestureRecognizers)
        {
            if ([gestureRecognizer isKindOfClass:[UILongPressGestureRecognizer class]] && gestureRecognizer != _longPressGestureRecognizer)
            {
                [gestureRecognizer requireGestureRecognizerToFail:_longPressGestureRecognizer];
                break;
            }
        }
        
        [self layoutChanged];
    }
    return self;
}

- (LSCollectionViewLayoutHelper *)layoutHelper
{
    return [(id <UICollectionViewLayout_Warpable>)self.collectionView.collectionViewLayout layoutHelper];
}

- (void)layoutChanged
{
    canWarp = [self.collectionView.collectionViewLayout conformsToProtocol:@protocol(UICollectionViewLayout_Warpable)];
    canScroll = [self.collectionView.collectionViewLayout respondsToSelector:@selector(scrollDirection)];
    _longPressGestureRecognizer.enabled = _panPressGestureRecognizer.enabled = canWarp && self.enabled;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (context == &kObservingCollectionViewLayoutContext) {
        [self layoutChanged];
    }
    else {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)setEnabled:(BOOL)enabled
{
    _enabled = enabled;
    _longPressGestureRecognizer.enabled = canWarp && enabled;
    _panPressGestureRecognizer.enabled = canWarp && enabled;
}

- (UIImage *)imageFromCell:(UICollectionViewCell *)cell {
    UIGraphicsBeginImageContextWithOptions(cell.bounds.size, cell.isOpaque, 0.0f);
    [cell.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return image;
}

- (void)invalidatesScrollTimer {
    if (timer != nil) {
        [timer invalidate];
        timer = nil;
    }
    scrollingDirection = _ScrollingDirectionUnknown;
}

- (void)setupScrollTimerInDirection:(_ScrollingDirection)direction {
    scrollingDirection = direction;
    if (timer == nil) {
        timer = [CADisplayLink displayLinkWithTarget:self selector:@selector(handleScroll:)];
        [timer addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
    }
    if (folderDataIndexPath)
    {
        [self highlightCellAtIndexPath:folderDataIndexPath enabled:NO];
        folderDataIndexPath = nil;
        folderVisualIndexPath = nil;
    }
}

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    if([gestureRecognizer isEqual:_panPressGestureRecognizer]) {
        return self.layoutHelper.fromIndexPath != nil;
    }
    return YES;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    if ([gestureRecognizer isEqual:_longPressGestureRecognizer]) {
        return [otherGestureRecognizer isEqual:_panPressGestureRecognizer];
    }
    
    if ([gestureRecognizer isEqual:_panPressGestureRecognizer]) {
        return [otherGestureRecognizer isEqual:_longPressGestureRecognizer];
    }
    
    return NO;
}

- (NSIndexPath *)indexPathForItemClosestToPoint:(CGPoint)point itemRectRef:(CGRect *)itemRectRef
{
    NSArray *layoutAttrsInRect;
    NSInteger closestDist = NSIntegerMax;
    NSIndexPath *indexPath;
    NSIndexPath *toIndexPath = self.layoutHelper.toIndexPath;
    
    // We need original positions of cells
    self.layoutHelper.toIndexPath = nil;
    layoutAttrsInRect = [self.collectionView.collectionViewLayout layoutAttributesForElementsInRect:self.collectionView.bounds];
    self.layoutHelper.toIndexPath = toIndexPath;
    
    // What cell are we closest to?
    for (UICollectionViewLayoutAttributes *layoutAttr in layoutAttrsInRect) {
        CGFloat xd = layoutAttr.center.x - point.x;
        CGFloat yd = layoutAttr.center.y - point.y;
        NSInteger dist = xd*xd + yd*yd;
        if (dist < closestDist) {
            closestDist = dist;
            indexPath = layoutAttr.indexPath;
            if (itemRectRef)
            {
                *itemRectRef = layoutAttr.frame;
            }
        }
    }
    
    // Are we closer to being the last cell in a different section?
    NSInteger sections = [self.collectionView numberOfSections];
    for (NSInteger i = 0; i < sections; ++i) {
        if (i == self.layoutHelper.fromIndexPath.section) {
            continue;
        }
        NSInteger items = [self.collectionView numberOfItemsInSection:i];
        NSIndexPath *nextIndexPath = [NSIndexPath indexPathForItem:items inSection:i];
        UICollectionViewLayoutAttributes *layoutAttr;
        CGFloat xd, yd;
        
        if (items > 0) {
            layoutAttr = [self.collectionView.collectionViewLayout layoutAttributesForItemAtIndexPath:nextIndexPath];
            xd = layoutAttr.center.x - point.x;
            yd = layoutAttr.center.y - point.y;
        } else {
            // Trying to use layoutAttributesForItemAtIndexPath while section is empty causes EXC_ARITHMETIC (division by zero items)
            // So we're going to ask for the header instead. It doesn't have to exist.
            layoutAttr = [self.collectionView.collectionViewLayout layoutAttributesForSupplementaryViewOfKind:UICollectionElementKindSectionHeader
                                                                                                  atIndexPath:nextIndexPath];
            xd = layoutAttr.frame.origin.x - point.x;
            yd = layoutAttr.frame.origin.y - point.y;
        }
        
        NSInteger dist = xd*xd + yd*yd;
        if (dist < closestDist) {
            closestDist = dist;
            indexPath = layoutAttr.indexPath;
            if (itemRectRef)
            {
                *itemRectRef = layoutAttr.frame;
            }
        }
    }
    return indexPath;
}

- (void)handleLongPressGesture:(UILongPressGestureRecognizer *)sender
{
    if (sender.state == UIGestureRecognizerStateChanged) {
        return;
    }
    if (![self.collectionView.dataSource conformsToProtocol:@protocol(UICollectionViewDataSource_Draggable)]) {
        return;
    }
    
    CGRect closestFrame;
    NSIndexPath *indexPath = [self indexPathForItemClosestToPoint:[sender locationInView:self.collectionView] itemRectRef:&closestFrame];
    
    switch (sender.state) {
        case UIGestureRecognizerStateBegan: {
            if (indexPath == nil) {
                return;
            }
            if (![(id<UICollectionViewDataSource_Draggable>)self.collectionView.dataSource
                  collectionView:self.collectionView
                  canMoveItemAtIndexPath:indexPath]) {
                return;
            }
            // Create mock cell to drag around
            UICollectionViewCell *cell = [self.collectionView cellForItemAtIndexPath:indexPath];
            cell.highlighted = NO;
            [mockCell removeFromSuperview];
            mockCell = [[UIImageView alloc] initWithFrame:cell.frame];
            mockCell.image = [self imageFromCell:cell];
            mockCenter = mockCell.center;
            [self.collectionView addSubview:mockCell];
            [UIView
             animateWithDuration:0.3
             animations:^{
                 mockCell.transform = CGAffineTransformMakeScale(1.1f, 1.1f);
             }
             completion:nil];
            
            // Start warping
            lastIndexPath = indexPath;
            self.layoutHelper.fromIndexPath = indexPath;
            self.layoutHelper.hideIndexPath = indexPath;
            self.layoutHelper.toIndexPath = indexPath;
            [self.collectionView.collectionViewLayout invalidateLayout];
        } break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled: {
            if(self.layoutHelper.fromIndexPath == nil) {
                return;
            }
            if (folderDataIndexPath)
            {
                [self highlightCellAtIndexPath:folderDataIndexPath enabled:NO];
            }
            // Need these for later, but need to nil out layoutHelper's references sooner
            NSIndexPath *fromIndexPath = self.layoutHelper.fromIndexPath;
            NSIndexPath *toIndexPath = self.layoutHelper.toIndexPath;
            // Tell the data source to move the item
            id<UICollectionViewDataSource_Draggable> dataSource = (id<UICollectionViewDataSource_Draggable>)self.collectionView.dataSource;
            if (folderDataIndexPath)
            {
                // move into folder
                [dataSource collectionView:self.collectionView moveItemAtIndexPath:fromIndexPath intoItemAtIndexPath:folderDataIndexPath];
                
                // Remove the item
                [self.collectionView performBatchUpdates:^{
                    [self.collectionView deleteItemsAtIndexPaths:@[fromIndexPath]];
                    self.layoutHelper.fromIndexPath = nil;
                    self.layoutHelper.toIndexPath = nil;
                } completion:nil];
            }
            else
            {
                // move to another position
                if (![toIndexPath isEqual:fromIndexPath])
                {
                    [dataSource collectionView:self.collectionView moveItemAtIndexPath:fromIndexPath toIndexPath:toIndexPath];
                }
                
                // Move the item
                [self.collectionView performBatchUpdates:^{
                    [self.collectionView moveItemAtIndexPath:fromIndexPath toIndexPath:toIndexPath];
                    self.layoutHelper.fromIndexPath = nil;
                    self.layoutHelper.toIndexPath = nil;
                } completion:^(BOOL finished) {
                    if (finished) {
                        if ([dataSource respondsToSelector:@selector(collectionView:didMoveItemAtIndexPath:toIndexPath:)]) {
                            [dataSource collectionView:self.collectionView didMoveItemAtIndexPath:fromIndexPath toIndexPath:toIndexPath];
                        }
                    }
                }];
            }
           
            
            // Switch mock for cell
            NSIndexPath *mockTargetIndexPath = folderVisualIndexPath ? folderVisualIndexPath : self.layoutHelper.hideIndexPath;
            UICollectionViewLayoutAttributes *layoutAttributes = [self.collectionView layoutAttributesForItemAtIndexPath:mockTargetIndexPath];
            [UIView
             animateWithDuration:0.3
             animations:^{
                 mockCell.center = layoutAttributes.center;
                 if (folderVisualIndexPath)
                 {
                     mockCell.transform = CGAffineTransformMakeScale(0.01f, 0.01f);
                 }
                 else
                 {
                     mockCell.transform = CGAffineTransformMakeScale(1.f, 1.f);
                 }
             }
             completion:^(BOOL finished) {
                 [mockCell removeFromSuperview];
                 mockCell = nil;
                 self.layoutHelper.hideIndexPath = nil;
                 [self.collectionView.collectionViewLayout invalidateLayout];
             }];
            
            // Reset
            [self invalidatesScrollTimer];
            lastIndexPath = nil;
            folderDataIndexPath = nil;
            folderVisualIndexPath = nil;
        } break;
        default: break;
    }
}

- (void)warpToIndexPath:(NSIndexPath *)indexPath
{
    if(indexPath == nil || [lastIndexPath isEqual:indexPath]) {
        return;
    }
    lastIndexPath = indexPath;
    
    if ([self.collectionView.dataSource respondsToSelector:@selector(collectionView:canMoveItemAtIndexPath:toIndexPath:)] == YES
        && [(id<UICollectionViewDataSource_Draggable>)self.collectionView.dataSource
            collectionView:self.collectionView
            canMoveItemAtIndexPath:self.layoutHelper.fromIndexPath
            toIndexPath:indexPath] == NO) {
        return;
    }
    [self.collectionView performBatchUpdates:^{
        self.layoutHelper.hideIndexPath = indexPath;
        self.layoutHelper.toIndexPath = indexPath;
    } completion:nil];
}

- (void)handlePanGesture:(UIPanGestureRecognizer *)sender
{
    if(sender.state == UIGestureRecognizerStateChanged) {
        // Move mock to match finger
        fingerTranslation = [sender translationInView:self.collectionView];
        mockCell.center = _CGPointAdd(mockCenter, fingerTranslation);
        
        // Scroll when necessary
        if (canScroll) {
            UICollectionViewFlowLayout *scrollLayout = (UICollectionViewFlowLayout*)self.collectionView.collectionViewLayout;
            if([scrollLayout scrollDirection] == UICollectionViewScrollDirectionVertical) {
                if (mockCell.center.y < (CGRectGetMinY(self.collectionView.bounds) + self.scrollingEdgeInsets.top)) {
                    [self setupScrollTimerInDirection:_ScrollingDirectionUp];
                }
                else {
                    if (mockCell.center.y > (CGRectGetMaxY(self.collectionView.bounds) - self.scrollingEdgeInsets.bottom)) {
                        [self setupScrollTimerInDirection:_ScrollingDirectionDown];
                    }
                    else {
                        [self invalidatesScrollTimer];
                    }
                }
            }
            else {
                if (mockCell.center.x < (CGRectGetMinX(self.collectionView.bounds) + self.scrollingEdgeInsets.left)) {
                    [self setupScrollTimerInDirection:_ScrollingDirectionLeft];
                } else {
                    if (mockCell.center.x > (CGRectGetMaxX(self.collectionView.bounds) - self.scrollingEdgeInsets.right)) {
                        [self setupScrollTimerInDirection:_ScrollingDirectionRight];
                    } else {
                        [self invalidatesScrollTimer];
                    }
                }
            }
        }
        
        // Avoid warping a second time while scrolling
        if (scrollingDirection > _ScrollingDirectionUnknown) {
            return;
        }
        
        CGPoint velocity = [sender velocityInView:self.collectionView];
        BOOL isFast = (velocity.x * velocity.x + velocity.y * velocity.y > 140.0 * 140.0);
        
        // Warp item to finger location
        CGPoint point = [sender locationInView:self.collectionView];
        
        NSIndexPath *oldFolderDataIndexPath = folderDataIndexPath;
        CGRect itemFrame;
        NSIndexPath *indexPath = [self indexPathForItemClosestToPoint:point itemRectRef:&itemFrame];
        
        CGRect innerItemFrame = itemFrame;
        innerItemFrame.size.width -= 42.0;
        innerItemFrame.size.height -= 42.0;
        innerItemFrame.origin.x += 21.0;
        
        BOOL canMoveInto = NO;
        if (CGRectContainsPoint(innerItemFrame, point))
        {
            NSIndexPath *dataIndexPath = indexPath;
            NSIndexPath *visualIndexPath = indexPath;
            canMoveInto = YES;
            if ([indexPath isEqual:self.layoutHelper.toIndexPath])
            {
                canMoveInto = NO;
            }
            else
            {
                NSInteger dataItemIndex = indexPath.item;
                NSInteger visualItemIndex = indexPath.item;
                if (   self.layoutHelper.fromIndexPath.item <= indexPath.item
                    && self.layoutHelper.toIndexPath.item > indexPath.item)
                {
                    dataItemIndex++;
                }
                else if (   self.layoutHelper.fromIndexPath.item >= indexPath.item
                         && self.layoutHelper.toIndexPath.item < indexPath.item)
                {
                    dataItemIndex--;
                }
                if (self.layoutHelper.toIndexPath.item < indexPath.item)
                {
                    visualItemIndex--;
                }
                dataIndexPath = [NSIndexPath indexPathForItem:dataItemIndex inSection:indexPath.section];
                visualIndexPath = [NSIndexPath indexPathForItem:visualItemIndex inSection:indexPath.section];
                id<UICollectionViewDataSource_Draggable> dataSource = (id<UICollectionViewDataSource_Draggable>)self.collectionView.dataSource;
                if (   [dataSource respondsToSelector:@selector(collectionView:canMoveItemAtIndexPath:intoItemAtIndexPath:)]
                    && [dataSource collectionView:self.collectionView canMoveItemAtIndexPath:self.layoutHelper.fromIndexPath intoItemAtIndexPath:dataIndexPath] == NO)
                {
                    canMoveInto = NO;
                }
            }
            
            if (canMoveInto)
            {
                folderDataIndexPath = dataIndexPath;
                folderVisualIndexPath = visualIndexPath;
            }
            else
            {
                folderDataIndexPath = nil;
                folderVisualIndexPath = nil;
            }
        }
        
        if (!isFast && !canMoveInto)
        {
            folderDataIndexPath = nil;
            folderVisualIndexPath = nil;
            [self warpToIndexPath:indexPath];
        }
        
        if (folderDataIndexPath != oldFolderDataIndexPath)
        {
            if (oldFolderDataIndexPath)
            {
                [self highlightCellAtIndexPath:oldFolderDataIndexPath enabled:NO];
            }
            if (folderDataIndexPath)
            {
                [self highlightCellAtIndexPath:folderDataIndexPath enabled:YES];
            }
        }
        
    }
}

- (void)highlightCellAtIndexPath:(NSIndexPath *)indexPath enabled:(BOOL)enabled
{
    id<UICollectionViewDataSource_Draggable> dataSource = (id<UICollectionViewDataSource_Draggable>)self.collectionView.dataSource;
    if ([dataSource respondsToSelector:@selector(collectionView:highlightItemAtIndexPath:enabled:)])
    {
        [dataSource collectionView:self.collectionView highlightItemAtIndexPath:indexPath enabled:enabled];
    }
}

- (void)handleScroll:(NSTimer *)timer {
    if (scrollingDirection == _ScrollingDirectionUnknown) {
        return;
    }
    
    CGSize frameSize = self.collectionView.bounds.size;
    CGSize contentSize = self.collectionView.contentSize;
    CGPoint contentOffset = self.collectionView.contentOffset;
    CGFloat distance = self.scrollingSpeed / 60.f;
    CGPoint translation = CGPointZero;
    
    switch(scrollingDirection) {
        case _ScrollingDirectionUp: {
            distance = -distance;
            if ((contentOffset.y + distance) <= 0.f) {
                distance = -contentOffset.y;
            }
            translation = CGPointMake(0.f, distance);
        } break;
        case _ScrollingDirectionDown: {
            CGFloat maxY = MAX(contentSize.height, frameSize.height) - frameSize.height;
            if ((contentOffset.y + distance) >= maxY) {
                distance = maxY - contentOffset.y;
            }
            translation = CGPointMake(0.f, distance);
        } break;
        case _ScrollingDirectionLeft: {
            distance = -distance;
            if ((contentOffset.x + distance) <= 0.f) {
                distance = -contentOffset.x;
            }
            translation = CGPointMake(distance, 0.f);
        } break;
        case _ScrollingDirectionRight: {
            CGFloat maxX = MAX(contentSize.width, frameSize.width) - frameSize.width;
            if ((contentOffset.x + distance) >= maxX) {
                distance = maxX - contentOffset.x;
            }
            translation = CGPointMake(distance, 0.f);
        } break;
        default: break;
    }
    
    mockCenter  = _CGPointAdd(mockCenter, translation);
    mockCell.center = _CGPointAdd(mockCenter, fingerTranslation);
    self.collectionView.contentOffset = _CGPointAdd(contentOffset, translation);
}

@end
