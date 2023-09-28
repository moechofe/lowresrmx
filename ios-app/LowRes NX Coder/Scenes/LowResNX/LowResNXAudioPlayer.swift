//
//  LowResNXAudioPlayer.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 18/8/18.
//  Copyright © 2018 Inutilis Software. All rights reserved.
//

import UIKit
import AudioToolbox
import AVFoundation

class LowResNXAudioPlayer: NSObject {
    
    var coreWrapper: CoreWrapper
    private var isActive = false
    private var queue: AudioQueueRef?
    
    init(coreWrapper: CoreWrapper) {
        self.coreWrapper = coreWrapper
        super.init()
    }
    
    func start() {
        if !isActive {
            isActive = true
            
            let session = AVAudioSession.sharedInstance()
            do {
                try session.setCategory(AVAudioSession.Category.ambient)
                try session.setActive(true)
            } catch {
                print("AVAudioSession", error.localizedDescription)
            }
            
            var dataFormat = AudioStreamBasicDescription()
            dataFormat.mSampleRate = session.sampleRate
            dataFormat.mFormatID = kAudioFormatLinearPCM
            dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked
            dataFormat.mBytesPerPacket = 4
            dataFormat.mFramesPerPacket = 1
            dataFormat.mBytesPerFrame = 4
            dataFormat.mChannelsPerFrame = 2
            dataFormat.mBitsPerChannel = 16
            dataFormat.mReserved = 0
            
            AudioQueueNewOutput(&dataFormat, audioQueueCallback, &coreWrapper, nil, CFRunLoopMode.commonModes.rawValue, 0, &queue)
            
            guard let queue = queue else {
                return
            }
            
            var buffer: AudioQueueBufferRef?
            for _ in 0 ..< 2 {
                AudioQueueAllocateBuffer(queue, 1470 * dataFormat.mBytesPerFrame, &buffer)
                if let buffer = buffer {
                    let capacity = buffer.pointee.mAudioDataBytesCapacity
                    audio_renderAudio(&coreWrapper.core, buffer.pointee.mAudioData.assumingMemoryBound(to: Int16.self), Int32(buffer.pointee.mAudioDataBytesCapacity / 2), Int32(AVAudioSession.sharedInstance().sampleRate), 0)
                    buffer.pointee.mAudioDataByteSize = capacity
                    AudioQueueEnqueueBuffer(queue, buffer, 0, nil)
                }
            }
            
            AudioQueueStart(queue, nil)
        }

    }
    
    func stop() {
        if isActive {
            isActive = false
            
            if let queue = queue {
                AudioQueueStop(queue, true)
                AudioQueueDispose(queue, true)
            }
            queue = nil
            
            try? AVAudioSession.sharedInstance().setActive(false)
        }
    }
    
}

func audioQueueCallback(_ userData: UnsafeMutableRawPointer?, _ audioQueue: AudioQueueRef, _ buffer: AudioQueueBufferRef) {
    if let coreWrapper = userData?.assumingMemoryBound(to: CoreWrapper.self).pointee {
        audio_renderAudio(&coreWrapper.core, buffer.pointee.mAudioData.assumingMemoryBound(to: Int16.self), Int32(buffer.pointee.mAudioDataBytesCapacity / 2), Int32(AVAudioSession.sharedInstance().sampleRate), 0)
    }
    AudioQueueEnqueueBuffer(audioQueue, buffer, 0, nil)
}
