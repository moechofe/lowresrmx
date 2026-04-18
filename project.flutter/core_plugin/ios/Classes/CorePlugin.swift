import Flutter
import UIKit
import CoreVideo

@objc(CorePlugin)
public class CorePlugin: NSObject, FlutterPlugin, FlutterTexture {
    private let registry: FlutterTextureRegistry
    private var textures: [Int64: CVPixelBuffer] = [:]
    private let channel: FlutterMethodChannel

    init(registry: FlutterTextureRegistry, channel: FlutterMethodChannel) {
        self.registry = registry
        self.channel = channel
        super.init()
    }

    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: "com.lowresrmx/core_plugin", binaryMessenger: registrar.messenger())
        let instance = CorePlugin(registry: registrar.textures(), channel: channel)
        registrar.addMethodCallDelegate(instance, channel: channel)
    }

    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "registerTexture":
            let width = 216
            let height = 384
            var pixelBuffer: CVPixelBuffer?
            let attrs = [
                kCVPixelBufferCGImageCompatibilityKey: kCFBooleanTrue,
                kCVPixelBufferCGBitmapContextCompatibilityKey: kCFBooleanTrue,
                kCVPixelBufferIOSurfacePropertiesKey: [:]
            ] as CFDictionary
            
            let status = CVPixelBufferCreate(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, attrs, &pixelBuffer)
            
            if status == kCVReturnSuccess, let buffer = pixelBuffer {
                let textureId = registry.register(self)
                textures[textureId] = buffer
                result(textureId)
            } else {
                result(FlutterError(code: "ERR", message: "Failed to create pixel buffer", details: nil))
            }
        case "unregisterTexture":
            if let textureId = call.arguments as? Int64 {
                textures.removeValue(forKey: textureId)
                registry.unregisterTexture(textureId)
            }
            result(nil)
        case "notifyFrameAvailable":
            if let textureId = call.arguments as? Int64 {
                registry.textureFrameAvailable(textureId)
            }
            result(nil)
        case "getBufferAddress":
            if let textureId = call.arguments as? Int64 {
                result(getBufferAddress(textureId: textureId))
            } else {
                result(FlutterError(code: "ERR", message: "Invalid textureId", details: nil))
            }
        default:
            result(FlutterMethodNotImplemented)
        }
    }

    public func copyPixelBuffer() -> Unmanaged<CVPixelBuffer>? {
        // This is tricky because we need to know which textureId is being requested.
        // For now, if there's only one, we return it.
        if let buffer = textures.values.first {
            return Unmanaged.passRetained(buffer)
        }
        return nil
    }
    
    // FFI accessible method to get the buffer address
    @objc public func getBufferAddress(textureId: Int64) -> Int {
        if let buffer = textures[textureId] {
            CVPixelBufferLockBaseAddress(buffer, .readOnly)
            let addr = Int(bitPattern: CVPixelBufferGetBaseAddress(buffer))
            CVPixelBufferUnlockBaseAddress(buffer, .readOnly)
            return addr
        }
        return 0
    }
    
    @objc public func notifyFrameAvailable(textureId: Int64) {
        registry.textureFrameAvailable(textureId)
    }
}
