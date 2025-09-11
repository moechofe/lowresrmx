//
//  ShareActivity.swift
//  LowResRMX
//
//  Created by Timo Kloss on 15/03/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit
import zstd

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

			// NSLog("programUrl %@", [programUrl])
			// NSLog("imageUrl %@", [imageUrl])


			var programDataToSend: Data? = nil
			if let programData = try? Data(contentsOf: programUrl) {
					if let compressed = try? ZStd.compress(programData) {
							programDataToSend = compressed
					}
			}

			// var imageEncoded = ""
			// if let imageData = try? Data(contentsOf: imageUrl) {
			// 	imageEncoded = imageData.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_")
			// }


            let url = URL(string: "https://ret.ro.it/upload")!
            var request = URLRequest(url: url)
            request.httpMethod = "POST"

            // Prepare JSON body
            var json: [String: Any] = ["name": programName]
            if let programDataToSend = programDataToSend {
                json["program"] = programDataToSend.base64EncodedString()
            }
            if let imageData = try? Data(contentsOf: imageUrl) {
                json["image"] = imageData.base64EncodedString()
            }
            do {
                let jsonData = try JSONSerialization.data(withJSONObject: json, options: [])
                request.httpBody = jsonData
                request.setValue("application/json", forHTTPHeaderField: "Content-Type")
            } catch {
                print("Failed to encode JSON: \(error)")
                return
            }

            let task = URLSession.shared.dataTask(with: request) { data, response, error in
                if let error = error {
                    print("Error: \(error)")
                } else if let response = response as? HTTPURLResponse {
                    print("Status code: \(response.statusCode)")
                }
            }
            task.resume()
//
//			guard let url = components.url else {
//				print("Failed to create url")
//				return
//			}
//
//			print("Generated URL: \(url.absoluteString)")
//
//			if UIApplication.shared.canOpenURL(url) {
//				if #available(iOS 10.0, *) {
//						UIApplication.shared.open(url, options: [:], completionHandler: {(success) in
//								if success {
//										print("Opened url")
//								} else {
//										print("Failed to open url")
//								}
//						})
//				} else {
//						// Fallback on earlier versions
//				}
//			} else {
//				print("Cannot open url")
//
//			}
		}
	}

	override var activityViewController: UIViewController? {
		return viewController
	}
}

















/*
override func prepare(withActivityItems activityItems: [Any]) {
    NSLog("prepare withActivityItems %@", activityItems)
    // TODO: what's inside withActivityItems?

    if let path = activityItems.first as? URL {
        let programUrl = path
        let imageUrl = path.deletingPathExtension().appendingPathExtension("png")
        let programName = path.deletingPathExtension().lastPathComponent

        var programEncoded = ""
        if let programData = try? Data(contentsOf: programUrl) {
            programEncoded = programData.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_")
        }

        var imageEncoded = ""
        if let imageData = try? Data(contentsOf: imageUrl) {
            imageEncoded = imageData.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_")
        }

        // Prepare the data to be posted
        let postData1: [String: Any] = [
            "programName": programName,
            "programEncoded": programEncoded
        ]

        let postData2: [String: Any] = [
            "imageName": programName,
            "imageEncoded": imageEncoded
        ]

        // Convert the data to JSON
        guard let jsonData1 = try? JSONSerialization.data(withJSONObject: postData1, options: []),
              let jsonData2 = try? JSONSerialization.data(withJSONObject: postData2, options: []) else {
            NSLog("Failed to serialize JSON")
            return
        }

        // Create the URLs for the POST requests
        guard let url1 = URL(string: "https://yourserver.com/api/uploadProgram"),
              let url2 = URL(string: "https://yourserver.com/api/uploadImage") else {
            NSLog("Invalid URL")
            return
        }

        // Create the requests
        var request1 = URLRequest(url: url1)
        request1.httpMethod = "POST"
        request1.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request1.httpBody = jsonData1

        var request2 = URLRequest(url: url2)
        request2.httpMethod = "POST"
        request2.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request2.httpBody = jsonData2

        // Show a dialog to the user
        let alertController = UIAlertController(title: "Uploading", message: "Please wait while your data is being uploaded.", preferredStyle: .alert)
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            // Cancel the upload tasks
            task1.cancel()
            task2.cancel()
        }
        alertController.addAction(cancelAction)

        // Present the alert controller
        if let viewController = UIApplication.shared.keyWindow?.rootViewController {
            viewController.present(alertController, animated: true, completion: nil)
        }

        // Create a DispatchGroup to manage the completion of both tasks
        let dispatchGroup = DispatchGroup()

        // Perform the first POST request
        dispatchGroup.enter()
        let task1 = URLSession.shared.dataTask(with: request1) { data, response, error in
            if let error = error {
                NSLog("POST request 1 failed: %@", error.localizedDescription)
            } else if let httpResponse = response as? HTTPURLResponse, httpResponse.statusCode == 200 {
                NSLog("POST request 1 succeeded")
            } else {
                NSLog("Unexpected response for POST request 1")
            }
            dispatchGroup.leave()
        }

        // Perform the second POST request
        dispatchGroup.enter()
        let task2 = URLSession.shared.dataTask(with: request2) { data, response, error in
            if let error = error {
                NSLog("POST request 2 failed: %@", error.localizedDescription)
            } else if let httpResponse = response as? HTTPURLResponse, httpResponse.statusCode == 200 {
                NSLog("POST request 2 succeeded")
            } else {
                NSLog("Unexpected response for POST request 2")
            }
            dispatchGroup.leave()
        }

        // Start both tasks
        task1.resume()
        task2.resume()

        // Wait for both tasks to complete
        dispatchGroup.notify(queue: .main) {
            alertController.dismiss(animated: true, completion: nil)
            // Open the URL after both POST requests are successful
            if let url = URL(string: "https://yourserver.com/success") {
                UIApplication.shared.open(url, options: [:], completionHandler: nil)
            }
        }
    }
}
*/
