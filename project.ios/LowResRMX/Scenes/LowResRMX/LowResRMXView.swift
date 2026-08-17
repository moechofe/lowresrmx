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

import CoreVideo
import UIKit

class LowResRMXView: UIView
{
	var coreWrapper: CoreWrapper?

	/// Rotating pool of IOSurface-backed screen buffers.
	///
	/// `video_renderScreen()` writes straight into one of these and the surface is handed to
	/// CoreAnimation unchanged, so there is no per-frame `CGImage` to allocate and nothing for
	/// CoreAnimation to copy or convert. The pixel format matches `ABGR=0` in the target's
	/// `GCC_PREPROCESSOR_DEFINITIONS` and CoreAnimation's own surface format.
	///
	/// More than one buffer is needed for two independent reasons: CoreAnimation may still be
	/// reading the previous surface while the next frame is being written, and assigning the
	/// *same* object to `layer.contents` is not guaranteed to register as a content change.
	private static let numBuffers = 3

	private var pixelBuffers: [CVPixelBuffer] = []
	private var nextBufferIndex = 0
	private var wasTouchReleased = false

	required init?(coder aDecoder: NSCoder)
	{
		super.init(coder: aDecoder)

		// constant for the lifetime of the view, so set once here rather than on every frame
		layer.magnificationFilter = CALayerContentsFilter.nearest
		// the core writes 0xff alpha for every pixel, so the compositor can skip blending
		layer.isOpaque = true

		let attributes = [
			kCVPixelBufferIOSurfacePropertiesKey: [:] as CFDictionary,
		] as CFDictionary

		for _ in 0 ..< LowResRMXView.numBuffers
		{
			var pixelBuffer: CVPixelBuffer?
			let status = CVPixelBufferCreate(
				kCFAllocatorDefault,
				Int(SCREEN_WIDTH),
				Int(SCREEN_HEIGHT),
				kCVPixelFormatType_32BGRA,
				attributes,
				&pixelBuffer
			)
			if status == kCVReturnSuccess, let pixelBuffer
			{
				LowResRMXView.clear(pixelBuffer: pixelBuffer)
				pixelBuffers.append(pixelBuffer)
			}
		}

		assert(pixelBuffers.count == LowResRMXView.numBuffers, "could not create the screen buffers")
	}

	/// Fills a fresh buffer with opaque black.
	///
	/// CVPixelBufferCreate() does not zero its allocation, and in COMPAT mode the core writes
	/// only the shown region of the buffer — the rest normally hangs off the device screen. If
	/// the shown region ever grows, an unwritten area could be revealed for one frame, so it
	/// must hold something deliberate rather than uninitialised memory. Black and not zero
	/// because CoreAnimation reads the alpha channel of a 32BGRA surface: the core always
	/// writes 0xff there (every `better_palette` entry is 0xffRRGGBB) and so must this.
	private static func clear(pixelBuffer: CVPixelBuffer)
	{
		CVPixelBufferLockBaseAddress(pixelBuffer, [])
		if let baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer)
		{
			let opaqueBlack: UInt32 = 0xFF00_0000
			let bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer)
			let height = CVPixelBufferGetHeight(pixelBuffer)
			for y in 0 ..< height
			{
				let row = (baseAddress + y * bytesPerRow).assumingMemoryBound(to: UInt32.self)
				for x in 0 ..< bytesPerRow / 4
				{
					row[x] = opaqueBlack
				}
			}
		}
		CVPixelBufferUnlockBaseAddress(pixelBuffer, [])
	}

	override func awakeFromNib()
	{
		super.awakeFromNib()
	}

	func render()
	{
		guard let coreWrapper, !pixelBuffers.isEmpty
		else
		{
			return
		}

		let pixelBuffer = pixelBuffers[nextBufferIndex]
		nextBufferIndex = (nextBufferIndex + 1) % pixelBuffers.count

		CVPixelBufferLockBaseAddress(pixelBuffer, [])
		if let baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer)
		{
			// the core takes the pitch in bytes and honours it for every row, so the row
			// padding CoreVideo adds costs nothing
			video_renderScreen(
				&coreWrapper.core,
				baseAddress.assumingMemoryBound(to: UInt32.self),
				Int32(CVPixelBufferGetBytesPerRow(pixelBuffer))
			)
		}
		CVPixelBufferUnlockBaseAddress(pixelBuffer, [])

		layer.contents = CVPixelBufferGetIOSurface(pixelBuffer)?.takeUnretainedValue()

		// release collected touches
		if wasTouchReleased
		{
			coreWrapper.input.touch = false
			wasTouchReleased = false
		}
	}

	/// A `CGImage` copy of the most recently rendered frame, for screenshots and program icons.
	///
	/// This exists because `layer.contents` holds an IOSurface now and can no longer be cast to
	/// a `CGImage`. The pixel data is copied rather than aliased, since the buffer it comes from
	/// will be overwritten a few frames from now.
	func snapshot() -> CGImage?
	{
		guard !pixelBuffers.isEmpty
		else
		{
			return nil
		}

		// the frame just rendered is the one *before* the next buffer to be written
		let pixelBuffer = pixelBuffers[(nextBufferIndex + pixelBuffers.count - 1) % pixelBuffers.count]

		CVPixelBufferLockBaseAddress(pixelBuffer, .readOnly)
		defer
		{
			CVPixelBufferUnlockBaseAddress(pixelBuffer, .readOnly)
		}

		let bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer)
		guard let baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer),
		      let data = CFDataCreate(nil, baseAddress.assumingMemoryBound(to: UInt8.self), bytesPerRow * CVPixelBufferGetHeight(pixelBuffer)),
		      let provider = CGDataProvider(data: data)
		else
		{
			return nil
		}

		return CGImage(
			width: Int(SCREEN_WIDTH),
			height: Int(SCREEN_HEIGHT),
			bitsPerComponent: 8,
			bitsPerPixel: 32,
			bytesPerRow: bytesPerRow,
			space: CGColorSpaceCreateDeviceRGB(),
			// the surface is BGRA; the alpha is always 0xff, so it can be skipped
			bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue),
			provider: provider,
			decode: nil,
			shouldInterpolate: false,
			intent: .defaultIntent
		)
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
