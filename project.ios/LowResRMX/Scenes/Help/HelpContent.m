//
//  HelpContent.m
//  Pixels
//
//  Created by Timo Kloss on 26/12/14.
//  Copyright (c) 2014 Inutilis Software. All rights reserved.
//

#import "HelpContent.h"

@interface HelpContent ()
@property NSArray *headerTags;
@property NSString *currentTag;
@property NSString *currentTagId;
@property NSString *currentTagName;
@property NSString *currentTagTitle;
@property HelpChapter *currentChapter;
@end

@implementation HelpContent

- (instancetype)initWithURL:(NSURL *)url
{
    if (self = [super init])
    {
        self.headerTags = @[/*@"h1", */@"h2", @"h3", @"h4"/*, @"h5", @"h6"*/];
        _url = url;
        _manualHtml = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil];
        _chapters = [NSMutableArray array];

        NSXMLParser *parser = [[NSXMLParser alloc] initWithContentsOfURL:url];
        parser.delegate = self;
        [parser parse];
    }
    return self;
}

- (NSArray<HelpChapter *> *)chaptersForSearchText:(NSString *)text
{
    text = [[text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] uppercaseString];
    NSMutableArray *results = [NSMutableArray array];
    for (HelpChapter *chapter in self.chapters)
    {
				if (chapter.level == 2)
				{
						if (chapter.keywords && [chapter.keywords indexOfObject:text] != NSNotFound)
						{
								[results addObject:chapter];
						}
				}
    }
    return results;
}

#pragma mark - XML Parser

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
//    NSLog(@"tag: %@",elementName);
    if ([self.headerTags indexOfObject:elementName.lowercaseString] != NSNotFound)
    {
//				NSLog(@"header id: %@",attributeDict[@"id"]);
        self.currentTag = elementName;
        self.currentTagId = attributeDict[@"id"];
        self.currentTagName = attributeDict[@"data-keyword"];
				self.currentTagTitle = attributeDict[@"data-title"];
    }
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
//		NSLog(@"char: %@",string);
    if (self.currentTag)
    {
				if (_currentChapter == nil)
				{
//						NSLog(@"header tag: %@",self.headerTags);
						_currentChapter = [[HelpChapter alloc] init];
						if (self.currentTagTitle && self.currentTagTitle.length > 0) {
							_currentChapter.title = self.currentTagTitle;
						} else {
							_currentChapter.title = string;
						}
						_currentChapter.htmlChapter = self.currentTagId;
						_currentChapter.keywords = [self.currentTagName componentsSeparatedByString:@","];
						_currentChapter.level = (int)[self.headerTags indexOfObject:self.currentTag.lowercaseString];
				}
				else
				{
						if (self.currentTagTitle && self.currentTagTitle.length > 0) {
							_currentChapter.title = self.currentTagTitle;
						} else {
							_currentChapter.title = string;
						}
				}
        //
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    if ([elementName isEqualToString:self.currentTag])
    {
				[self.chapters addObject:_currentChapter];
				self.currentChapter = nil;
        self.currentTag = nil;
        self.currentTagId = nil;
        self.currentTagName = nil;
    }
}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    NSLog(@"Help parse error: %@ code: %ld", parseError.localizedDescription,(long)parseError.code);
}

@end

@implementation HelpChapter

@end
