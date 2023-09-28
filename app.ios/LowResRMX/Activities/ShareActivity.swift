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
    
    override func canPerform(withActivityItems activityItems: [Any]) -> Bool {
        return true
    }
    
    override func prepare(withActivityItems activityItems: [Any]) {
        let vc = ShareViewController()
        vc.activity = self
        if let programUrl = activityItems.first as? URL {
            vc.programUrl = programUrl
            vc.imageUrl = programUrl.deletingPathExtension().appendingPathExtension("png")
        }
        viewController = UINavigationController(rootViewController: vc)
    }
    
    override var activityViewController: UIViewController? {
        return viewController
    }
    
}
