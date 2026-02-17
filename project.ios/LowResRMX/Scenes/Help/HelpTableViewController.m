//
//  HelpTableViewController.m
//  Pixels
//
//  Created by Timo Kloss on 29/11/14.
//  Copyright (c) 2014 Inutilis Software. All rights reserved.
//

#import "HelpTableViewController.h"
#import "HelpTextViewController.h"
#import "HelpContent.h"
#import "HelpSplitViewController.h"
#import "App-Swift.h"

@interface HelpTableViewController ()

@property HelpContent *helpContent;
@property UIBarButtonItem *cancelItem;

@end


@implementation HelpTableViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    _cancelItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(onCancelTapped:)];

    self.tableView.indicatorStyle = UIScrollViewIndicatorStyleWhite;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    self.helpContent = AppController.shared.helpContent;

    [self updateBarButtonCollapsed:self.splitViewController.collapsed];
}

- (void)updateBarButtonCollapsed:(BOOL)collapsed
{
    if (collapsed)
    {
        self.navigationItem.rightBarButtonItem = self.cancelItem;
    }
    else
    {
        self.navigationItem.rightBarButtonItem = nil;
    }
}

- (void)onCancelTapped:(id)sender
{
    HelpSplitViewController *helpVC = (HelpSplitViewController *)self.splitViewController;
    [self.splitViewController showDetailViewController:helpVC.detailNavigationController sender:self];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return self.helpContent.chapters.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    HelpChapter *chapter = self.helpContent.chapters[indexPath.row];
    NSString *cellIdentifier;
    if (chapter.level == 0)
    {
        cellIdentifier = @"ChapterCell";
    }
    else if (chapter.level == 1)
    {
        cellIdentifier = @"SubchapterCell";
    }
    else
    {
        cellIdentifier = @"CommandCell";
    }
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier forIndexPath:indexPath];

    cell.textLabel.text = chapter.title;
    cell.indentationLevel = chapter.level;

    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    HelpChapter *chapter = self.helpContent.chapters[indexPath.row];
    HelpSplitViewController *helpVC = (HelpSplitViewController *)self.splitViewController;
    [helpVC showChapter:chapter.htmlChapter];

    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
