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
	return [self filteredChapters].count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	HelpChapter *chapter = [self filteredChapters][indexPath.row];
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
		// Normally, not used
		cellIdentifier = @"CommandCell";
	}
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier forIndexPath:indexPath];

	cell.textLabel.text = chapter.title;
	cell.indentationLevel = chapter.level;

	return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	HelpChapter *chapter = [self filteredChapters][indexPath.row];
	HelpSplitViewController *helpVC = (HelpSplitViewController *)self.splitViewController;
	[helpVC showChapter:chapter.htmlChapter];

	[tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (NSArray<HelpChapter *> *)filteredChapters {
	NSMutableArray<HelpChapter *> *filtered = [NSMutableArray array];
	for (HelpChapter *chapter in self.helpContent.chapters) {
		if (chapter.level == 0 || chapter.level == 1) {
			[filtered addObject:chapter];
		}
	}
	return filtered;
}

@end
