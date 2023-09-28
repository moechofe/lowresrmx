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

@interface HelpTextViewController ()

@property (weak, nonatomic) IBOutlet UIWebView *webView;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityView;

@end

@implementation HelpTextViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.webView.delegate = self;
    self.webView.scalesPageToFit = NO;
    self.webView.dataDetectorTypes = UIDataDetectorTypeNone;
    
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
    [self.webView stringByEvaluatingJavaScriptFromString:script];
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
    if (!request.URL.isFileURL)
    {
        // open links in Safari
        [[UIApplication sharedApplication] openURL:[request URL]];
        return NO;
    }
    return YES;
}

- (void)webViewDidStartLoad:(UIWebView *)webView
{
    [self.activityView startAnimating];
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
    [self.activityView stopAnimating];
    if (self.chapter)
    {
        [self jumpToChapter:self.chapter];
    }
}

@end
