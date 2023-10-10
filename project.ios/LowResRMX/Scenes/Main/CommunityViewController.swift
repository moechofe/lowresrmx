//
//  CommunityViewController.swift
//  LowResRMX
//
//  Created by Timo Kloss on 24/05/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit

class CommunityViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    @IBAction func openAction(_ sender: Any) {
        let url = URL(string: "https://lowresnx.inutilis.com/programs.php")!
        UIApplication.shared.openURL(url)
    }
    
}
