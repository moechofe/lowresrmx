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

class LowResRMXView: UIView
{
	var coreWrapper: CoreWrapper?

	private var data: UnsafeMutablePointer<UInt32>?
	private var dataProvider: CGDataProvider?
	private var wasTouchReleased = false

	required init?(coder aDecoder: NSCoder)
	{
		super.init(coder: aDecoder)

		let numPixels = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT)
		data = UnsafeMutablePointer<UInt32>.allocate(capacity: numPixels)
		var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
		dataProvider = CGDataProvider(directInfo: data, size: off_t(numPixels * 4), callbacks: &callbacks)
	}

	override func awakeFromNib()
	{
		super.awakeFromNib()
	}

	func render()
	{
		if let coreWrapper, let dataProvider
		{
			video_renderScreen(&coreWrapper.core, data)
			let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH) * 4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)

			layer.contents = image
			layer.magnificationFilter = CALayerContentsFilter.nearest

			// release collected touches
			if wasTouchReleased
			{
				coreWrapper.input.touch = false
				wasTouchReleased = false
			}
		}
	}

	override func touchesBegan(_ touches: Set<UITouch>, with _: UIEvent?)
	{
		if let coreWrapper, let touch = touches.first
		{
			let point = screenPoint(touch: touch)
			coreWrapper.input.touchX = Float(point.x)
			coreWrapper.input.touchY = Float(point.y)
			coreWrapper.input.touch = true
		}
	}

	override func touchesMoved(_ touches: Set<UITouch>, with _: UIEvent?)
	{
		if let coreWrapper, let touch = touches.first
		{
			let point = screenPoint(touch: touch)
			coreWrapper.input.touchX = Float(point.x)
			coreWrapper.input.touchY = Float(point.y)
		}
	}

	override func touchesEnded(_: Set<UITouch>, with _: UIEvent?)
	{
		wasTouchReleased = true
	}

	override func touchesCancelled(_: Set<UITouch>, with _: UIEvent?)
	{
		wasTouchReleased = true
	}

	private func screenPoint(touch: UITouch) -> CGPoint
	{
		let viewPoint = touch.location(in: self)
		let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / bounds.size.width
		let y = viewPoint.y * CGFloat(SCREEN_HEIGHT) / bounds.size.height
		return CGPoint(x: x, y: y)
	}
}

func getBytePointerCallback(_ data: UnsafeMutableRawPointer?) -> UnsafeRawPointer?
{
	UnsafeRawPointer(data)
}
