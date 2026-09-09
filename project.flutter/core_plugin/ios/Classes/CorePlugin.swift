import Flutter
import UIKit
import CoreVideo

@objc(CorePlugin)
public class CorePlugin: NSObject, FlutterPlugin, FlutterTexture {
    private let registry: FlutterTextureRegistry
    private var textures: [Int64: CVPixelBuffer] = [:]
    /// Buffers replaced by a resize. The render isolate may still be mid-write into one, so they
    /// are kept alive until the texture is unregistered instead of being freed on the spot.
    private var retired: [CVPixelBuffer] = []
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

    /// Allocates a device-resolution surface and describes it the way the method channel does on
    /// both platforms: textureId, base address, bytes per row.
    private func makeSurface(textureId: Int64, width: Int, height: Int) -> Any? {
        var pixelBuffer: CVPixelBuffer?
        let attrs = [
            kCVPixelBufferCGImageCompatibilityKey: kCFBooleanTrue,
            kCVPixelBufferCGBitmapContextCompatibilityKey: kCFBooleanTrue,
            kCVPixelBufferIOSurfacePropertiesKey: [:]
        ] as CFDictionary

        let status = CVPixelBufferCreate(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, attrs, &pixelBuffer)
        guard status == kCVReturnSuccess, let buffer = pixelBuffer else { return nil }

        if let previous = textures[textureId] {
            retired.append(previous)
        }
        textures[textureId] = buffer

        // A non-planar CVPixelBuffer's base address is stable, so it is read under a short lock
        // and handed to the C side once; the lock is not held across the engine's use of it.
        CVPixelBufferLockBaseAddress(buffer, .readOnly)
        let address = Int(bitPattern: CVPixelBufferGetBaseAddress(buffer))
        CVPixelBufferUnlockBaseAddress(buffer, .readOnly)
        return [
            "textureId": textureId,
            "address": address,
            "bytesPerRow": CVPixelBufferGetBytesPerRow(buffer)
        ]
    }

    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "registerTexture":
            let args = call.arguments as? [String: Any]
            let width = max(args?["width"] as? Int ?? 216, 1)
            let height = max(args?["height"] as? Int ?? 384, 1)
            let textureId = registry.register(self)
            if let surface = makeSurface(textureId: textureId, width: width, height: height) {
                result(surface)
            } else {
                registry.unregisterTexture(textureId)
                result(FlutterError(code: "ERR", message: "Failed to create pixel buffer", details: nil))
            }
        case "resizeTexture":
            let args = call.arguments as? [String: Any]
            guard let textureId = (args?["textureId"] as? NSNumber)?.int64Value, textures[textureId] != nil else {
                result(FlutterError(code: "NOT_FOUND", message: "Texture not found", details: nil))
                return
            }
            let width = max(args?["width"] as? Int ?? 216, 1)
            let height = max(args?["height"] as? Int ?? 384, 1)
            if let surface = makeSurface(textureId: textureId, width: width, height: height) {
                // Let the engine pick up the new buffer size.
                registry.textureFrameAvailable(textureId)
                result(surface)
            } else {
                result(FlutterError(code: "ERR", message: "Failed to create pixel buffer", details: nil))
            }
        case "unregisterTexture":
            if let textureId = (call.arguments as? NSNumber)?.int64Value {
                textures.removeValue(forKey: textureId)
                retired.removeAll()
                registry.unregisterTexture(textureId)
            }
            result(nil)
        case "notifyFrameAvailable":
            if let textureId = (call.arguments as? NSNumber)?.int64Value {
                registry.textureFrameAvailable(textureId)
            }
            result(nil)
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

    
    @objc public func notifyFrameAvailable(textureId: Int64) {
        registry.textureFrameAvailable(textureId)
    }
}
