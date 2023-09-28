//
//  RoundHitBoxButton.swift
//  LowResRMX
//
//  Created by Timo Kloss on 13/2/18.
//  Copyright © 2018 Inutilis Software. All rights reserved.
//

import UIKit

class RoundHitBoxButton: UIButton {
    
    override func hitTest(_ point: CGPoint, with event: UIEvent?) -> UIView? {
        if !isHidden && isUserInteractionEnabled {
            let radius = bounds.size.width * 0.5
            let diffX = radius - point.x
            let diffY = radius - point.y
            let distance = diffX * diffX + diffY * diffY
            let extendedRadius = radius + 10
            if distance <= extendedRadius * extendedRadius {
                return self
            }
        }
        return nil
    }
    
}
