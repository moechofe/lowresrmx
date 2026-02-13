//
//  AppDelegate.swift
//  LowRes CoderRMX
//
//  Created by Timo Kloss on 21/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import StoreKit
import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

	static var baseURL: String {
		let key="USE_SHARE_HOST"
		guard ProcessInfo.processInfo.environment.contains(where: {$0.key==key}) else { return "https://ret.ro.it" }
		return ProcessInfo.processInfo.environment[key]!
	}

	var window: UIWindow?

  func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {

    AppStyle.setAppearance()

    return true
  }

  func applicationWillResignActive(_ application: UIApplication) {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
  }

  func applicationDidEnterBackground(_ application: UIApplication) {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
  }

  func applicationWillEnterForeground(_ application: UIApplication) {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
  }

  func applicationDidBecomeActive(_ application: UIApplication) {
  }

  func applicationWillTerminate(_ application: UIApplication) {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
  }

  func application(
    _ app: UIApplication, open url: URL, options: [UIApplication.OpenURLOptionsKey: Any] = [:]
  ) -> Bool {
    if url.isFileURL {
      ProjectManager.shared.importProgram(
        from: url,
        completion: { (error) in
          if let error = error {
            print("importProgram:", error.localizedDescription)
          }
        })
      return true

    } else if url.scheme == "lowresrmx" {
      if let components = URLComponents(url: url, resolvingAgainstBaseURL: false),
        let items = components.queryItems
      {
        var name: String?
        var program: URL?
        var image: URL?
        var topicId: String?

        for item in items {
          if item.name == "n" {
            name = item.value
          } else if item.name == "i" {
            topicId = item.value
//            if let topicId = topicId?.addingPercentEncoding(
//              withAllowedCharacters: .alphanumerics.union(CharacterSet(charactersIn: "-"))),
//              !topicId.isEmpty
						if let topicId = topicId?.removingPercentEncoding,
							!topicId.isEmpty
            {
              program = URL(string: "\(AppDelegate.baseURL)\(topicId).rmx")
              image = URL(string: "\(AppDelegate.baseURL)\(topicId).png")
            }
          }
        }
        if let name = name, let program = program {
          let webSource = WebSource(
            name: name, programUrl: program, imageUrl: image, topicId: topicId)
          AppController.shared.runProgram(webSource)
        }
      }
      return true
    }
    return false
  }

  // MARK: UISceneSession Lifecycle

  @available(iOS 13.0, *)
  func application(_ application: UIApplication, configurationForConnecting connectingSceneSession: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
    return UISceneConfiguration(name: "Default Configuration", sessionRole: connectingSceneSession.role)
  }

  @available(iOS 13.0, *)
  func application(_ application: UIApplication, didDiscardSceneSessions sceneSessions: Set<UISceneSession>) {}
}
