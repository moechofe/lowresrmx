//
//  HelpTextViewController.m
//  Pixels
//
//  Created by Timo Kloss on 25/12/14.
//  Copyright (c) 2014 Inutilis Software. All rights reserved.
//

#import "HelpTextViewController.h"
#import "HelpTableViewController.h"
#import "HelpContent.h"
#import "HelpSplitViewController.h"
#import "App-Swift.h"
#import <WebKit/WebKit.h>

@interface HelpTextViewController () <WKNavigationDelegate, UIScrollViewDelegate, UISearchBarDelegate>

@property (weak, nonatomic) IBOutlet WKWebView *webView;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityView;
@property (weak, nonatomic) IBOutlet UISearchBar *searchBar;

@property (strong, nonatomic) HelpChapter *lastFoundChapter;
@property (strong, nonatomic) NSArray<HelpChapter *> *lastSearchResults;

@end

@implementation HelpTextViewController

- (void)viewDidLoad
{
	[super viewDidLoad];

	self.webView.navigationDelegate = self;
	self.webView.scrollView.alwaysBounceHorizontal = NO;
	self.webView.scrollView.bounces = NO;
	self.webView.scrollView.showsHorizontalScrollIndicator = NO;
	self.webView.scrollView.contentSize = CGSizeMake(self.webView.frame.size.width, self.webView.scrollView.contentSize.height);
	self.webView.scrollView.delegate = self; // If you want to enforce further

	self.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
	self.navigationItem.leftItemsSupplementBackButton = YES;

	self.searchBar.delegate = self; // In case not set in storyboard

	HelpContent *helpContent = AppController.shared.helpContent;
	[self.webView loadHTMLString:helpContent.manualHtml baseURL:helpContent.url];
}

- (void)setChapter:(NSString *)chapter
{
	_chapter = chapter;
	if (!self.webView.isLoading)
	{
		[self jumpToChapter:self.chapter];
	}
}

- (void)jumpToChapter:(NSString *)chapter
{
	NSString *script = [NSString stringWithFormat:@"document.getElementById('%@').scrollIntoView(true);", chapter];
	[self.webView evaluateJavaScript:script completionHandler:nil];
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
{
	if (!navigationAction.request.URL.isFileURL)
	{
		// open links in Safari
		[[UIApplication sharedApplication] openURL:navigationAction.request.URL options:@{} completionHandler:nil];
		decisionHandler(WKNavigationActionPolicyCancel);
		return;
	}
	decisionHandler(WKNavigationActionPolicyAllow);
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation
{
	[self.activityView startAnimating];
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
	[self.activityView stopAnimating];
	if (self.chapter)
	{
		[self jumpToChapter:self.chapter];
	}
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
	CGPoint offset = scrollView.contentOffset;
	if (offset.x != 0) {
		offset.x = 0;
		scrollView.contentOffset = offset;
	}
}

- (void)searchBar:(UISearchBar *)searchBar textDidChange:(NSString *)searchText {
    HelpContent *helpContent = AppController.shared.helpContent;
    NSArray<HelpChapter *> *results = [helpContent chaptersForSearchAny:searchText];
    self.lastSearchResults = results;
		self.lastFoundChapter = nil;
    if (results.count > 0) {
        self.lastFoundChapter = results[0];

    }
}

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar {
    if (self.lastSearchResults.count > 0) {
				if (self.lastFoundChapter == nil) {
						self.lastFoundChapter = self.lastSearchResults[0];
				} else {
						NSUInteger index = [self.lastSearchResults indexOfObject:self.lastFoundChapter];
						if (index < self.lastSearchResults.count - 1) {
								self.lastFoundChapter = self.lastSearchResults[index + 1];
						} else {
								self.lastFoundChapter = self.lastSearchResults[0];
						}
				}
        HelpSplitViewController *helpVC = (HelpSplitViewController *)self.splitViewController;
        [helpVC showChapter:self.lastFoundChapter.htmlChapter];
    }
}

@end
