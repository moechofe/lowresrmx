//
//  LowResNXView.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class LowResNXView: UIView {
    var coreWrapper: CoreWrapper?

    private var data: UnsafeMutablePointer<UInt32>?
    private var dataProvider: CGDataProvider?
    private var wasTouchReleased = false
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        
        let numPixels = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT);
        data = UnsafeMutablePointer<UInt32>.allocate(capacity: numPixels)
        var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
        dataProvider = CGDataProvider(directInfo: data, size: off_t(numPixels * 4), callbacks: &callbacks)
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
    }
    
    func render() {
        if let coreWrapper = coreWrapper, let dataProvider = dataProvider {
            video_renderScreen(&coreWrapper.core, data)
            let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH)*4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
            
            layer.contents = image
            layer.magnificationFilter = CALayerContentsFilter.nearest
            
            // release collected touches
            if wasTouchReleased {
                coreWrapper.input.touch = false
                wasTouchReleased = false
            }
        }
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if let coreWrapper = coreWrapper, let touch = touches.first {
            let point = screenPoint(touch: touch)
            coreWrapper.input.touchX = Float(point.x)
            coreWrapper.input.touchY = Float(point.y)
            coreWrapper.input.touch = true
        }
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        if let coreWrapper = coreWrapper, let touch = touches.first {
            let point = screenPoint(touch: touch)
            coreWrapper.input.touchX = Float(point.x)
            coreWrapper.input.touchY = Float(point.y)
        }
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        wasTouchReleased = true
    }
    
    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        wasTouchReleased = true
    }
    
    private func screenPoint(touch: UITouch) -> CGPoint {
        let viewPoint = touch.location(in: self)
        let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / bounds.size.width;
        let y = viewPoint.y * CGFloat(SCREEN_HEIGHT) / bounds.size.height;
        return CGPoint(x: x, y: y);
    }

}

func getBytePointerCallback(_ data: UnsafeMutableRawPointer?) -> UnsafeRawPointer? {
    return UnsafeRawPointer(data)
}
