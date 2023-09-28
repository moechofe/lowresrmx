//
//  WebViewController.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 16/03/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit
import WebKit

class WebViewController: UIViewController {

    private var webView: WKWebView!
    
    var url: URL?
    var isModal = true
    
    override func loadView() {
        let config = WKWebViewConfiguration()
        config.processPool = AppController.shared.webProcessPool
        webView = WKWebView(frame: CGRect.zero, configuration: config)
        
//        webView.backgroundColor = AppStyle.darkGrayColor()
        webView.isOpaque = false
//        webView.scrollView.indicatorStyle = .white
        view = webView
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if isModal {
            navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(done))
        }
        
        if let url = url {
            let urlRequest = URLRequest(url: url)
            webView.load(urlRequest)
        }
    }
    
    @objc func done(_ sender: Any) {
        webView.stopLoading()
        presentingViewController?.dismiss(animated: true, completion: nil)
    }
    
}
