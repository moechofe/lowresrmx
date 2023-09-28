//
//  Gamepad.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class Dpad: UIControl {
    
    enum Image: Int {
        case normal
        case up
        case upRight
        case right
        case downRight
        case down
        case downLeft
        case left
        case upLeft
    }
    
    var isDirUp = false
    var isDirDown = false
    var isDirLeft = false
    var isDirRight = false
    
    var isBig = false {
        didSet {
            updateImage()
            invalidateIntrinsicContentSize()
            setNeedsLayout()
        }
    }
    
    private let imageView: UIImageView
    
    private let images: [UIImage] = [
        UIImage(named:"gamepad_dpad")!,
        UIImage(named:"gamepad_dpad_up")!,
        UIImage(named:"gamepad_dpad_up_right")!,
        UIImage(named:"gamepad_dpad_right")!,
        UIImage(named:"gamepad_dpad_down_right")!,
        UIImage(named:"gamepad_dpad_down")!,
        UIImage(named:"gamepad_dpad_down_left")!,
        UIImage(named:"gamepad_dpad_left")!,
        UIImage(named:"gamepad_dpad_up_left")!
    ]
    
    private let bigImages: [UIImage] = [
        UIImage(named:"big_gamepad_dpad")!,
        UIImage(named:"big_gamepad_dpad_up")!,
        UIImage(named:"big_gamepad_dpad_up_right")!,
        UIImage(named:"big_gamepad_dpad_right")!,
        UIImage(named:"big_gamepad_dpad_down_right")!,
        UIImage(named:"big_gamepad_dpad_down")!,
        UIImage(named:"big_gamepad_dpad_down_left")!,
        UIImage(named:"big_gamepad_dpad_left")!,
        UIImage(named:"big_gamepad_dpad_up_left")!
    ]
    
    required init?(coder aDecoder: NSCoder) {
        imageView = UIImageView()
        super.init(coder: aDecoder)
        
        imageView.contentMode = .center
        addSubview(imageView)
        backgroundColor = UIColor.clear
        updateImage()
    }
    
    override var intrinsicContentSize: CGSize {
        if isBig {
            return bigImages.first!.size
        } else {
            return images.first!.size
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        imageView.frame = bounds
    }
    
    override func hitTest(_ point: CGPoint, with event: UIEvent?) -> UIView? {
        if !isHidden && isUserInteractionEnabled {
            let errorMargin: CGFloat = 32;
            let largerFrame = CGRect(x: -errorMargin, y: -errorMargin, width: frame.size.width + 2 * errorMargin, height: frame.size.height + 2 * errorMargin)
            return largerFrame.contains(point) ? self : nil
        }
        return nil
    }
    
    override func beginTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        let begin = super.beginTracking(touch, with: event)
        if begin {
            updateDirections(touch: touch)
        }
        return begin
    }
    
    override func continueTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        let cont = super.continueTracking(touch, with: event)
        if cont {
            updateDirections(touch: touch)
        }
        return cont
    }
    
    override func endTracking(_ touch: UITouch?, with event: UIEvent?) {
        resetDirections()
        super.endTracking(touch, with: event)
    }
    
    override func cancelTracking(with event: UIEvent?) {
        resetDirections()
        super.cancelTracking(with: event)
    }
    
    private func updateDirections(touch: UITouch) {
        var point = touch.location(in: self)
        point.x -= bounds.size.width * 0.5;
        point.y -= bounds.size.height * 0.5;
        let centerSize: CGFloat = 10.0
        isDirUp = (point.y < -centerSize) && abs(point.x / point.y) < 2.0;
        isDirDown = (point.y > centerSize) && abs(point.x / point.y) < 2.0;
        isDirLeft = (point.x < -centerSize) && abs(point.y / point.x) < 2.0;
        isDirRight = (point.x > centerSize) && abs(point.y / point.x) < 2.0;
        updateImage()
    }
    
    private func resetDirections() {
        isDirUp = false
        isDirDown = false
        isDirLeft = false
        isDirRight = false
        updateImage()
    }
    
    private func updateImage() {
        var gi = Image.normal
        if isDirUp {
            if isDirLeft {
                gi = .upLeft
            } else if isDirRight {
                gi = .upRight
            } else {
                gi = .up
            }
        } else if isDirDown {
            if isDirLeft {
                gi = .downLeft
            } else if isDirRight {
                gi = .downRight
            } else {
                gi = .down
            }
        } else if isDirLeft {
            gi = .left
        } else if isDirRight {
            gi = .right
        }
        
        if isBig {
            imageView.image = bigImages[gi.rawValue]
        } else {
            imageView.image = images[gi.rawValue]
        }
    }
    
}

