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
	// static var baseURL: String = "http://10.10.35.216:8080"
	static var baseURL: String = "https://ret.ro.it"

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
		guard !didPrepare else { return }
		didPrepare = true

		NSLog("prepare withActivityItems %@", activityItems)

		if let path = activityItems.first as? URL {
			let programUrl = path
			let imageUrl = path.deletingPathExtension().appendingPathExtension("png")
			let programName = path.deletingPathExtension().lastPathComponent

			var programDataToSend: Data? = nil
			if let programData = try? Data(contentsOf: programUrl) {
				if let compressed = try? ZStd.compress(programData) {
					programDataToSend = compressed
				}
			}

			let url = URL(string: "\(ShareActivity.baseURL)/upload")!
			var request = URLRequest(url: url)
			request.httpMethod = "POST"

			// Prepare JSON body
			var json: [String: Any] = ["n": programName]
			if let programDataToSend = programDataToSend {
				json["p"] = programDataToSend.base64EncodedString()
			}
			if let imageData = try? Data(contentsOf: imageUrl) {
				json["i"] = imageData.base64EncodedString()
			}
			do {
				let jsonData = try JSONSerialization.data(withJSONObject: json, options: [])
				if let jsonString = String(data: jsonData, encoding: .utf8) {
					// print("jsonData: \(jsonString)")
				} else {
					print("Failed to convert jsonData to String")
				}
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
					// print("Status code: \(response.statusCode)")
					if let data = data {
						do {
							let uploadToken = try JSONSerialization.jsonObject(with: data, options: .fragmentsAllowed)
							// print("Response JSON: \(uploadToken)")

							if let uploadToken = uploadToken as? String,
							   let encodedToken = uploadToken.addingPercentEncoding(withAllowedCharacters: .alphanumerics.union(CharacterSet(charactersIn: "-")))
							{
								// print("Encoded upload token: \(encodedToken)")

								if var components = URLComponents(string: "\(ShareActivity.baseURL)/share") {
									components.queryItems = [URLQueryItem(name: "uptoken", value: encodedToken)]
									if let url = components.url {
										DispatchQueue.main.async {
											UIApplication.shared.open(url, options: [:], completionHandler: { success in
												if success {
													// print("Opened URL: \(url)")
												} else {
													print("Failed to open URL")
												}
											})
										}
									} else {
										print("Failed to create URL from components")
									}
								} else {
									print("Failed to create URL components")
								}

							} else {
								print("Failed to encode upload token")
							}

						} catch {
							print("Failed to parse JSON response: \(error)")
						}
					} else {
						print("No data received in response")
					}
				}
			}
			task.resume()
		}
	}

	override var activityViewController: UIViewController? {
		return viewController
	}
}
