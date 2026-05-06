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

import UIKit

@available(iOS 13.0, *)
class SceneDelegate: UIResponder, UIWindowSceneDelegate
{
	var window: UIWindow?

	func scene(_ scene: UIScene, willConnectTo _: UISceneSession, options connectionOptions: UIScene.ConnectionOptions)
	{
		if let urlContext = connectionOptions.urlContexts.first
		{
			(UIApplication.shared.delegate as? AppDelegate)?.handle(url: urlContext.url)
		}
		else
		{
			guard let _ = (scene as? UIWindowScene) else { return }
		}
	}

	func scene(_: UIScene, openURLContexts URLContexts: Set<UIOpenURLContext>)
	{
		// This is called when the app is already running and a URL is opened.
		guard let url = URLContexts.first?.url else { return }
		(UIApplication.shared.delegate as? AppDelegate)?.handle(url: url)
	}

	func sceneDidDisconnect(_: UIScene)
	{}

	func sceneDidBecomeActive(_: UIScene)
	{}

	func sceneWillResignActive(_: UIScene)
	{}

	func sceneWillEnterForeground(_: UIScene)
	{}

	func sceneDidEnterBackground(_: UIScene)
	{}
}
