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

import AudioToolbox
import AVFoundation
import UIKit

/// Immutable state handed to the AudioQueue render callback.
///
/// It exists so the callback never has to reach for `AVAudioSession.sharedInstance().sampleRate`:
/// that is an ObjC property access which can take a lock and cross to the media server, and the
/// callback runs on a realtime thread where neither is acceptable.
final class AudioRenderContext
{
	let coreWrapper: CoreWrapper
	let sampleRate: Int32

	init(coreWrapper: CoreWrapper, sampleRate: Int32)
	{
		self.coreWrapper = coreWrapper
		self.sampleRate = sampleRate
	}
}

class LowResRMXAudioPlayer: NSObject
{
	var coreWrapper: CoreWrapper
	private var isActive = false
	private var queue: AudioQueueRef?
	/// Held for as long as the queue lives, because the callback holds it unretained.
	private var context: AudioRenderContext?

	init(coreWrapper: CoreWrapper)
	{
		self.coreWrapper = coreWrapper
		super.init()
	}

	func start()
	{
		if !isActive
		{
			isActive = true

			let session = AVAudioSession.sharedInstance()
			do
			{
				try session.setCategory(AVAudioSession.Category.ambient)
				try session.setActive(true)
			}
			catch
			{
				print("AVAudioSession", error.localizedDescription)
			}

			// read once, after the session is active, and reuse for both the stream format
			// and every render call
			let sampleRate = session.sampleRate
			let context = AudioRenderContext(coreWrapper: coreWrapper, sampleRate: Int32(sampleRate))
			self.context = context

			var dataFormat = AudioStreamBasicDescription()
			dataFormat.mSampleRate = sampleRate
			dataFormat.mFormatID = kAudioFormatLinearPCM
			dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked
			dataFormat.mBytesPerPacket = 4
			dataFormat.mFramesPerPacket = 1
			dataFormat.mBytesPerFrame = 4
			dataFormat.mChannelsPerFrame = 2
			dataFormat.mBitsPerChannel = 16
			dataFormat.mReserved = 0

			let unmanagedContext = Unmanaged.passUnretained(context).toOpaque()
			AudioQueueNewOutput(&dataFormat, audioQueueCallback, unmanagedContext, nil, CFRunLoopMode.commonModes.rawValue, 0, &queue)

			guard let queue
			else
			{
				return
			}

			var buffer: AudioQueueBufferRef?
			for _ in 0 ..< 2
			{
				AudioQueueAllocateBuffer(queue, 1470 * dataFormat.mBytesPerFrame, &buffer)
				if let buffer
				{
					let capacity = buffer.pointee.mAudioDataBytesCapacity
					audio_renderAudio(&coreWrapper.core, buffer.pointee.mAudioData.assumingMemoryBound(to: Int16.self), Int32(capacity / 2), context.sampleRate, 0)
					buffer.pointee.mAudioDataByteSize = capacity
					AudioQueueEnqueueBuffer(queue, buffer, 0, nil)
				}
			}

			AudioQueueStart(queue, nil)
		}
	}

	func stop()
	{
		if isActive
		{
			isActive = false

			if let queue
			{
				// both synchronous, so no callback can still be in flight afterwards and the
				// context is safe to drop
				AudioQueueStop(queue, true)
				AudioQueueDispose(queue, true)
			}
			queue = nil
			context = nil

			try? AVAudioSession.sharedInstance().setActive(false)
		}
	}
}

func audioQueueCallback(_ userData: UnsafeMutableRawPointer?, _ audioQueue: AudioQueueRef, _ buffer: AudioQueueBufferRef)
{
	if let userData
	{
		let context = Unmanaged<AudioRenderContext>.fromOpaque(userData).takeUnretainedValue()
		audio_renderAudio(&context.coreWrapper.core, buffer.pointee.mAudioData.assumingMemoryBound(to: Int16.self), Int32(buffer.pointee.mAudioDataBytesCapacity / 2), context.sampleRate, 0)
	}
	AudioQueueEnqueueBuffer(audioQueue, buffer, 0, nil)
}
