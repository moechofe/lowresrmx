//
//  ShareActivity.swift
//  LowResRMX
//
//  Created by Timo Kloss on 15/03/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit

class ShareActivity: UIActivity {
	static let postToForum = UIActivity.ActivityType("lowResNxPostToForum")

	private var viewController: UIViewController?

	override var activityType: UIActivity.ActivityType? {
		return ShareActivity.postToForum
	}

	override var activityTitle: String? {
		return "Share with Community"
	}

	override var activityImage: UIImage? {
		return UIImage(named: "sharecommunity")
	}

	override func canPerform(withActivityItems _: [Any]) -> Bool {
		return true
	}

	override func prepare(withActivityItems activityItems: [Any]) {
		NSLog("prepare withActivityItems %@", activityItems)
		// TODO: what's inside withActivityItems?

		if let path = activityItems.first as? URL {
			let programUrl = path
			let imageUrl = path.deletingPathExtension().appendingPathExtension("png")
            let programName = path.deletingPathExtension().lastPathComponent

			NSLog("programUrl %@", [programUrl])
			NSLog("imageUrl %@", [imageUrl])

			var programEncoded = ""
			var imageEncoded = ""

			if let programData = try? Data(contentsOf: programUrl) {
				programEncoded = programData.base64EncodedString()
			}

			if let imageData = try? Data(contentsOf: imageUrl) {
				imageEncoded = imageData.base64EncodedString()
			}

			NSLog("programEncoded %@", [programEncoded])
			NSLog("imageEncoded %@", [imageEncoded])

			guard let url = URL(string: String(format: "http://lowresrmx.top:8080/upload?p=%@&t=%@&n=%@", programEncoded, imageEncoded, programName)) else {
				print("Failed to create url")
				return
			}

			if UIApplication.shared.canOpenURL(url) {
									if #available(iOS 10.0, *) {
											UIApplication.shared.open(url, options: [:], completionHandler: {(success) in
													if success {
															print("Opened url")
													} else {
															print("Failed to open url")
													}
											})
									} else {
											// Fallback on earlier versions
									}
			} else {
				print("Cannot open url")
			}
		}

		// if let text = activityItems.first as? String {
		// 	if let data = text.data(using: .utf8) {
		// 		let base64String = data.base64EncodedString()
		// 		// Use the base64String as needed
		// 	}
		// }

		// guard let url = URL(string: "http://10.10.35.216:8080/share?type=prg&content=gabuzomeu") else {
		// 	print("Failed to create url")
		// 	return
		// }

		// if UIApplication.shared.canOpenURL(url) {
		//             if #available(iOS 10.0, *) {
		//                 UIApplication.shared.open(url, options: [:], completionHandler: {(success) in
		//                     if success {
		//                         print("Opened url")
		//                     } else {
		//                         print("Failed to open url")
		//                     }
		//                 })
		//             } else {
		//                 // Fallback on earlier versions
		//             }
		// } else {
		// 	print("Cannot open url")
		// }

		// let vc = ShareViewController()
		// vc.activity = self
		// if let programUrl = activityItems.first as? URL {
		//     vc.programUrl = programUrl
		//     vc.imageUrl = programUrl.deletingPathExtension().appendingPathExtension("png")
		// }
		// viewController = UINavigationController(rootViewController: vc)
	}

	override var activityViewController: UIViewController? {
		return viewController
	}
}
