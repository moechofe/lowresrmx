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

	private var didPrepare = false

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

	private func showErrorAlert(code: String) {
		let alert = UIAlertController(
			title: "POKE 53280,1",
			message: "Feature 1 has been defeated. #\(code)",
			preferredStyle: .alert
		)
		alert.addAction(UIAlertAction(title: "Ok", style: .default) { [weak self] _ in
			self?.activityDidFinish(false)
		})
		viewController = alert
	}

	override func prepare(withActivityItems activityItems: [Any]) {
		guard !didPrepare else { return }
		didPrepare = true

		NSLog("prepare withActivityItems %@", activityItems)

		if let path = activityItems.first as? URL {
			let programUrl = path
			let imageUrl = programUrl.deletingPathExtension().appendingPathExtension("png")

			if !FileManager.default.fileExists(atPath: imageUrl.path) {
				let alert = UIAlertController(
					title: "No Program Icon",
					message:
						"Please capture a program icon before sharing. You can do this from the menu while the program is running.",
					preferredStyle: .alert)
				alert.addAction(UIAlertAction(title: "OK", style: .default) { [weak self] _ in
					self?.activityDidFinish(false)
				})
				viewController = alert
				return
			}

			let programName = programUrl.deletingPathExtension().lastPathComponent

			var programDataToSend: Data? = nil
			guard let programData = try? Data(contentsOf: programUrl) else {
				showErrorAlert(code: "LPD") // Load Program Data
				return
			}
			guard let compressed = try? ZStd.compress(programData) else {
				showErrorAlert(code: "CPD") // Compress Program Data
				return
			}
			programDataToSend = compressed

			guard let url = URL(string: "\(AppDelegate.baseURL)/upload") else {
				showErrorAlert(code: "CBU") // Create Base URL
				return
			}
			var request = URLRequest(url: url)
			request.httpMethod = "POST"

			// Prepare JSON body
			var json: [String: Any] = ["n": programName]
			json["p"] = programDataToSend?.base64EncodedString()

			guard let imageData = try? Data(contentsOf: imageUrl) else {
				showErrorAlert(code: "LID") // Load Image Data
				return
			}
			json["i"] = imageData.base64EncodedString()

			do {
				let jsonData = try JSONSerialization.data(withJSONObject: json, options: [])
				request.httpBody = jsonData
				request.setValue("application/json", forHTTPHeaderField: "Content-Type")
			} catch {
				print("Failed to encode JSON: \(error)")
				showErrorAlert(code: "EJS") // Encode JSON
				return
			}

			let semaphore = DispatchSemaphore(value: 0)
			var networkError: String?

			let task = URLSession.shared.dataTask(with: request) { data, response, error in
				defer { semaphore.signal() }

				if let error = error {
					print("Error: \(error)")
					networkError = "NWE" // Network Error
					return
				}

				guard let httpResponse = response as? HTTPURLResponse, (200 ... 299).contains(httpResponse.statusCode) else {
					print("Invalid response: \(String(describing: response))")
					networkError = "IVR" // Invalid Response
					return
				}

				guard let data = data else {
					print("No data received in response")
					networkError = "NDA" // No Data
					return
				}

				do {
					let uploadToken = try JSONSerialization.jsonObject(with: data, options: .fragmentsAllowed)

					guard let uploadTokenStr = uploadToken as? String,
					      let encodedToken = uploadTokenStr.addingPercentEncoding(withAllowedCharacters: .alphanumerics.union(CharacterSet(charactersIn: "-")))
					else {
						networkError = "IVT" // Invalid Token
						return
					}

					guard var components = URLComponents(string: "\(AppDelegate.baseURL)/share") else {
						networkError = "FUC" // Failed URL Components
						return
					}

					components.queryItems = [URLQueryItem(name: "uptoken", value: encodedToken)]
					guard let url = components.url else {
						networkError = "FUC"
						return
					}

					DispatchQueue.main.async {
						UIApplication.shared.open(url, options: [:]) { success in
							self.activityDidFinish(success)
						}
					}
				} catch {
					print("Failed to parse JSON response: \(error)")
					networkError = "PJE" // Parse JSON Error
				}
			}
			task.resume()
			semaphore.wait()

			if let errorCode = networkError {
				showErrorAlert(code: errorCode)
			}
		} else {
			showErrorAlert(code: "FPU") // Failed to get Program URL
		}
	}

	override var activityViewController: UIViewController? {
		return viewController
	}
}
