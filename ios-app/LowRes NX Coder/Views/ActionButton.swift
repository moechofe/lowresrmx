//
//  ActionButton.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 03/04/2020.
//  Copyright © 2020 Inutilis Software. All rights reserved.
//

import UIKit

class ActionButton: RoundHitBoxButton {
    
    enum Action {
        case a
        case b
    }
    
    var action: Action = .a {
        didSet {
            updateImages()
        }
    }
    
    var isBig = false {
        didSet {
            updateImages()
        }
    }
    
    func updateImages() {
        if isBig {
            switch action {
            case .a:
                setImage(UIImage(named: "big_gamepad_a"), for: .normal)
                setImage(UIImage(named: "big_gamepad_a_sel"), for: .highlighted)
            case .b:
                setImage(UIImage(named: "big_gamepad_b"), for: .normal)
                setImage(UIImage(named: "big_gamepad_b_sel"), for: .highlighted)
            }
        } else {
            switch action {
            case .a:
                setImage(UIImage(named: "gamepad_a"), for: .normal)
                setImage(UIImage(named: "gamepad_a_sel"), for: .highlighted)
            case .b:
                setImage(UIImage(named: "gamepad_b"), for: .normal)
                setImage(UIImage(named: "gamepad_b_sel"), for: .highlighted)
            }
        }
    }
    
}
