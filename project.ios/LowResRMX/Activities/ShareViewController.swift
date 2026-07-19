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

import UIKit
import WebKit

class ShareViewController: UIViewController, WKNavigationDelegate
{
	weak var activity: ShareActivity?
	var programUrl: URL?
	var imageUrl: URL?

	private var webView: WKWebView!
	private var activityView: UIActivityIndicatorView!

	override func loadView()
	{
		let config = WKWebViewConfiguration()
		webView = WKWebView(frame: CGRect.zero, configuration: config)

		webView.isOpaque = false
		webView.navigationDelegate = self
		view = webView
	}

	override func viewDidLoad()
	{
		super.viewDidLoad()

		modalPresentationStyle = .popover

		navigationItem.title = "Share with Community"
		navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .cancel, target: self, action: #selector(cancel))

		activityView = UIActivityIndicatorView(style: .medium)
		activityView.sizeToFit()
		navigationItem.rightBarButtonItem = UIBarButtonItem(customView: activityView)

		let url = URL(string: "\(AppDelegate.baseURL)/share.html?r=app")!
		let urlRequest = URLRequest(url: url)
		webView.load(urlRequest)
	}

	@objc func cancel(_: Any)
	{
		webView.stopLoading()
		activity?.activityDidFinish(false)
	}

	func showError(_ error: Error? = nil)
	{
		showAlert(withTitle: "Something Went Wrong", message: error?.localizedDescription)
		{
			self.webView.stopLoading()
			self.activity?.activityDidFinish(false)
		}
	}

	// MARK: - WKNavigationDelegate

	func webView(_: WKWebView, decidePolicyFor navigationAction: WKNavigationAction, decisionHandler: @escaping (WKNavigationActionPolicy) -> Void)
	{
		if let url = navigationAction.request.url
		{
			if url.path == "/topic.php"
			{
				// sharing done
				activityView.stopAnimating()
				decisionHandler(.cancel)

				let alert = UIAlertController(title: "Your program has been published successfully.", message: nil, preferredStyle: .alert)
				alert.addAction(UIAlertAction(title: "Open in Safari", style: .default, handler: { _ in
					UIApplication.shared.open(url, options: [:], completionHandler: nil)
					self.activity?.activityDidFinish(true)
				}))
				alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { _ in
					self.activity?.activityDidFinish(true)
				}))
				present(alert, animated: true, completion: nil)
				return
			}
			else if navigationAction.targetFrame == nil
			{
				// open in new view
				let vc = WebViewController()
				vc.url = url
				vc.isModal = false
				navigationController?.pushViewController(vc, animated: true)
			}
		}
		decisionHandler(.allow)
	}

	func webView(_: WKWebView, didStartProvisionalNavigation _: WKNavigation!)
	{
		activityView.startAnimating()
	}

	func webView(_: WKWebView, didFail _: WKNavigation!, withError error: Error)
	{
		activityView.stopAnimating()
		showError(error)
	}

	func webView(_: WKWebView, didFinish _: WKNavigation!)
	{
		activityView.stopAnimating()
	}
}
