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

@interface HelpTextViewController () <WKNavigationDelegate>

@property (weak, nonatomic) IBOutlet WKWebView *webView;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityView;

@end

@implementation HelpTextViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.webView.navigationDelegate = self;

    self.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
    self.navigationItem.leftItemsSupplementBackButton = YES;

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

@end
