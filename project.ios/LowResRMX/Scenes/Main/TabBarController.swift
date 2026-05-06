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
