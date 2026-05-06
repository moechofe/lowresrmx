//
//  TabBarController.swift
//  LowResRMX
//
//  Created by Timo Kloss on 25/01/2020.
//  Copyright © 2020 Inutilis Software. All rights reserved.
//

import SwiftUI
import UIKit

enum TabIndex: Int
{
	case explorer
	case help
	case community
}

class TabBarController: UITabBarController
{
	override func viewDidLoad()
	{
		super.viewDidLoad()

		AppController.shared.tabBarController = self

		let explorerVC = storyboard!.instantiateViewController(withIdentifier: "ExplorerNav")
		let helpStoryboard = UIStoryboard(name: "Help", bundle: nil)
		let helpVC = helpStoryboard.instantiateInitialViewController()!

		let communityView = CommunityView()
		let communityVC = UIHostingController(rootView: communityView)
		let communityNav = UINavigationController(rootViewController: communityVC)
		communityVC.navigationItem.title = "Community"

		explorerVC.tabBarItem = item(title: "My Programs", imageName: "programs")
		helpVC.tabBarItem = item(title: "Help", imageName: "help")
		communityNav.tabBarItem = item(title: "Community", imageName: "community")

		viewControllers = [explorerVC, helpVC, communityNav]

		NotificationCenter.default.addObserver(self, selector: #selector(didAddProgram), name: NSNotification.Name(rawValue: "ProjectManagerDidAddProgram"), object: nil)
	}

	override func viewDidAppear(_ animated: Bool)
	{
		super.viewDidAppear(animated)
		AppController.shared.checkShowProgram()
	}

	override var keyCommands: [UIKeyCommand]?
	{
		if presentedViewController != nil
		{
			return nil
		}
		return [
			UIKeyCommand(title: "Show My Programs", action: #selector(onTab1), input: "1", modifierFlags: .command),
			UIKeyCommand(title: "Show Help", action: #selector(onTab2), input: "2", modifierFlags: .command),
			UIKeyCommand(title: "Show Community", action: #selector(onTab3), input: "3", modifierFlags: .command),
		]
	}

	func item(title: String, imageName: String) -> UITabBarItem
	{
		UITabBarItem(title: title, image: UIImage(named: imageName), selectedImage: nil)
	}

	func dismissPresentedViewController(completion: @escaping () -> Void)
	{
		var topVC = selectedViewController
		if topVC is UINavigationController
		{
			topVC = (topVC as! UINavigationController).topViewController
		}
		if topVC?.presentedViewController != nil
		{
			topVC?.dismiss(animated: true, completion: completion)
		}
		else
		{
			completion()
		}
	}

	func showExplorer(animated: Bool, root: Bool)
	{
		selectedIndex = TabIndex.explorer.rawValue
		let nav = selectedViewController as? UINavigationController
		if root
		{
			nav?.popToRootViewController(animated: animated)
		}
		else
		{
			/*
			 if (![nav.topViewController isKindOfClass:[ExplorerViewController class]])
			 {
			     [nav popViewControllerAnimated:animated];
			 }*/
		}
	}

	func showHelp(chapter: String)
	{
		selectedIndex = TabIndex.help.rawValue
		let helpVC = selectedViewController as! HelpSplitViewController
		helpVC.showChapter(chapter)
	}

	@objc func didAddProgram()
	{
		if selectedIndex != 0
		{
			selectedIndex = 0
		}
	}

	@objc func onTab1()
	{
		selectedIndex = 0
	}

	@objc func onTab2()
	{
		selectedIndex = 1
	}

	@objc func onTab3()
	{
		selectedIndex = 2
	}

	// @objc func onTab4() {
	//    selectedIndex = 3
	// }
}
