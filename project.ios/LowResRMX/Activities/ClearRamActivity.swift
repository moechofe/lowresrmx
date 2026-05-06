//
//  ClearRamActivity.swift
//  LowResRMX
//
//  Created by Timo Kloss on 26/06/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit

class ClearRamActivity: UIActivity
{
	static let clearRam = UIActivity.ActivityType("lowResNxClearRam")

	private var viewController: UIViewController?

	override var activityType: UIActivity.ActivityType?
	{
		ClearRamActivity.clearRam
	}

	override var activityTitle: String?
	{
		"Clear Persistent RAM"
	}

	override var activityImage: UIImage?
	{
		UIImage(named: "clear_ram")
	}

	override func canPerform(withActivityItems _: [Any]) -> Bool
	{
		true
	}

	override func prepare(withActivityItems activityItems: [Any])
	{
		let programUrl = activityItems.first as? URL

		let alert = UIAlertController(title: "Clear Persistent RAM?", message: "This may delete data like the game state or high scores of this program.", preferredStyle: .actionSheet)

		alert.addAction(UIAlertAction(title: "Clear", style: .destructive, handler: { [weak self] _ in
			if let programUrl
			{
				ProjectManager.shared.deletePersistentRam(programUrl: programUrl)
			}
			self?.activityDidFinish(true)
		}))

		alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: { [weak self] _ in
			self?.activityDidFinish(false)
		}))

		viewController = alert
	}

	override var activityViewController: UIViewController?
	{
		viewController
	}
}
