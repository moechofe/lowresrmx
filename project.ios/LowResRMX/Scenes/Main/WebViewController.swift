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

class WebViewController: UIViewController
{
	private var webView: WKWebView!

	var url: URL?
	var isModal = true

	override func loadView()
	{
		let config = WKWebViewConfiguration()
		webView = WKWebView(frame: CGRect.zero, configuration: config)

		//        webView.backgroundColor = AppStyle.darkGrayColor()
		webView.isOpaque = false
		//        webView.scrollView.indicatorStyle = .white
		view = webView
	}

	override func viewDidLoad()
	{
		super.viewDidLoad()

		if isModal
		{
			navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(done))
		}

		if let url
		{
			let urlRequest = URLRequest(url: url)
			webView.load(urlRequest)
		}
	}

	@objc func done(_: Any)
	{
		webView.stopLoading()
		presentingViewController?.dismiss(animated: true, completion: nil)
	}
}
