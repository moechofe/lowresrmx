//
//  CoreWrapper.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 2/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Foundation

protocol CoreWrapperDelegate: class {
    func coreInterpreterDidFail(coreError: CoreError) -> Void
    func coreDiskDriveWillAccess(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool
    func coreDiskDriveDidSave(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void
    func coreDiskDriveIsFull(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void
    func coreControlsDidChange(controlsInfo: ControlsInfo) -> Void
    func persistentRamWillAccess(destination: UnsafeMutablePointer<UInt8>?, size: Int32) -> Void
    func persistentRamDidChange(_ data: Data) -> Void
}

class CoreWrapper: NSObject {
    
    weak var delegate: CoreWrapperDelegate?
    
    var core = Core()
    var input = CoreInput()
    private(set) var sourceCode: String?
    private var coreDelegate = CoreDelegate()
    
    override init() {
        super.init()
        core_init(&core)
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        coreDelegate.diskDriveIsFull = diskDriveIsFull
        coreDelegate.controlsDidChange = controlsDidChange
        coreDelegate.persistentRamWillAccess = persistentRamWillAccess
        coreDelegate.persistentRamDidChange = persistentRamDidChange
        core_setDelegate(&core, &coreDelegate)
    }
    
    deinit {
        core_deinit(&core)
    }
    
    func compileProgram(sourceCode: String) -> LowResNXError? {
        self.sourceCode = sourceCode
        let cString = sourceCode.cString(using: .utf8)
        let error = core_compileProgram(&core, cString, true)
        if error.code != ErrorNone {
            return LowResNXError(error: error, sourceCode: sourceCode)
        }
        return nil
    }
    
}

class LowResNXError: NSError {
    
    let coreError: CoreError
    let message: String
    let line: String
    
    init(error: CoreError, sourceCode: String) {
        coreError = error
        if error.sourcePosition < sourceCode.count {
            let index = sourceCode.index(sourceCode.startIndex, offsetBy: Int(error.sourcePosition))
            let lineRange = sourceCode.lineRange(for: index ..< index)
            line = sourceCode[lineRange].trimmingCharacters(in: CharacterSet.whitespaces)
        } else {
            line = ""
        }
        message = String(cString:err_getString(error.code))
        super.init(domain: "LowResNX", code: Int(coreError.code.rawValue), userInfo: [NSLocalizedDescriptionKey: "\(message): \(line)"])
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

//MARK: - Core Delegate Functions Wrapper

func interpreterDidFail(context: UnsafeMutableRawPointer?, coreError: CoreError) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreInterpreterDidFail(coreError: coreError)
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    return wrapper.delegate?.coreDiskDriveWillAccess(diskDataManager: diskDataManager) ?? true
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreDiskDriveDidSave(diskDataManager: diskDataManager)
}

func diskDriveIsFull(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreDiskDriveIsFull(diskDataManager: diskDataManager)
}

func controlsDidChange(context: UnsafeMutableRawPointer?, controlsInfo: ControlsInfo) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreControlsDidChange(controlsInfo: controlsInfo)
}

func persistentRamWillAccess(context: UnsafeMutableRawPointer?, destination: UnsafeMutablePointer<UInt8>?, size: Int32) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.persistentRamWillAccess(destination: destination, size: size)
}

func persistentRamDidChange(context: UnsafeMutableRawPointer?, data: UnsafeMutablePointer<UInt8>?, size: Int32) -> Void {
    guard let data = data else { return }
    
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    let ramData = Data(bytes: data, count: Int(size))
    wrapper.delegate?.persistentRamDidChange(ramData)
}
