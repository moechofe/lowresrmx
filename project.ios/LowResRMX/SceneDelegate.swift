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
