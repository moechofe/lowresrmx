import Flutter
import UIKit

@main
@objc class AppDelegate: FlutterAppDelegate, FlutterImplicitEngineDelegate {
  override func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {
    return super.application(application, didFinishLaunchingWithOptions: launchOptions)
  }

  func didInitializeImplicitFlutterEngine(_ engineBridge: FlutterImplicitEngineBridge) {
    GeneratedPluginRegistrant.register(with: engineBridge.pluginRegistry)
    ImportBridge.shared.attach(messenger: engineBridge.applicationRegistrar.messenger())
    BackgroundTaskBridge.shared.attach(messenger: engineBridge.applicationRegistrar.messenger())
  }
}

/// Holds a UIKit background task while Dart reports a Drive sync in flight, so
/// leaving the app does not suspend it mid-upload.
final class BackgroundTaskBridge {
  static let shared = BackgroundTaskBridge()

  private var task: UIBackgroundTaskIdentifier = .invalid

  func attach(messenger: FlutterBinaryMessenger) {
    FlutterMethodChannel(name: "com.lowresrmx/background", binaryMessenger: messenger)
      .setMethodCallHandler { call, result in
        switch call.method {
        case "begin":
          self.begin()
          result(nil)
        case "end":
          self.end()
          result(nil)
        default:
          result(FlutterMethodNotImplemented)
        }
      }
  }

  private func begin() {
    guard task == .invalid else { return }
    task = UIApplication.shared.beginBackgroundTask(withName: "sync") { [weak self] in
      self?.end()
    }
  }

  private func end() {
    guard task != .invalid else { return }
    UIApplication.shared.endBackgroundTask(task)
    task = .invalid
  }
}
