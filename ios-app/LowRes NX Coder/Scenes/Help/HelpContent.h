//
//  HelpContent.h
//  Pixels
//
//  Created by Timo Kloss on 26/12/14.
//  Copyright (c) 2014 Inutilis Software. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class HelpChapter;

@interface HelpContent : NSObject <NSXMLParserDelegate>

@property (readonly) NSURL *url;
@property (readonly) NSString *manualHtml;
@property (readonly) NSMutableArray *chapters;


- (instancetype)initWithURL:(NSURL *)url;
- (NSArray<HelpChapter *> *)chaptersForSearchText:(NSString *)text;

@end

@interface HelpChapter : NSObject
@property NSString *title;
@property NSString *htmlChapter;
@property NSArray *keywords;
@property int level;
@end

NS_ASSUME_NONNULL_END
