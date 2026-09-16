import Flutter
import UIKit

/// Bridges files and URLs handed to the app by iOS onto the `com.lowresrmx/import`
/// method channel, queueing everything that arrives before Dart is listening.
final class ImportBridge {
  static let shared = ImportBridge()

  static let appGroup = "group.it.ro.ret.flutter.LowResRMX"
  static let rmxType = "it.ro.ret.document.rmx"

  private var channel: FlutterMethodChannel?
  private var dartReady = false
  private var pending: [[String: Any]] = []
  private var emitted = Set<String>()

  func attach(messenger: FlutterBinaryMessenger) {
    let channel = FlutterMethodChannel(name: "com.lowresrmx/import", binaryMessenger: messenger)
    channel.setMethodCallHandler { [weak self] call, result in
      guard let self = self else {
        result(FlutterMethodNotImplemented)
        return
      }
      if call.method == "drainPendingImports" {
        self.dartReady = true
        result(self.pending)
        self.pending = []
      } else {
        result(FlutterMethodNotImplemented)
      }
    }
    self.channel = channel
  }

  func handle(urls: [URL]) {
    for url in urls {
      if url.scheme == "lowresrmx" {
        if url.host == "import" {
          drainAppGroup()
        } else {
          emit(["kind": "uri", "uri": url.absoluteString])
        }
      } else if url.isFileURL, url.pathExtension.lowercased() == "rmx" {
        stage(url)
      }
    }
  }

  /// Copies a security-scoped or provider-backed file into a directory the app owns,
  /// then hands the copy to Dart, which deletes it when done.
  private func stage(_ url: URL) {
    let scoped = url.startAccessingSecurityScopedResource()
    defer {
      if scoped { url.stopAccessingSecurityScopedResource() }
    }

    let directory = FileManager.default.temporaryDirectory
      .appendingPathComponent("import", isDirectory: true)
    let target = directory.appendingPathComponent(UUID().uuidString + ".rmx")

    do {
      try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
    } catch {
      NSLog("ImportBridge: could not create import directory: \(error)")
      return
    }

    var coordinationError: NSError?
    var copyError: Error?
    NSFileCoordinator().coordinate(readingItemAt: url, options: [], error: &coordinationError) {
      readURL in
      do {
        try FileManager.default.copyItem(at: readURL, to: target)
      } catch {
        copyError = error
      }
    }
    if let error = coordinationError ?? copyError {
      NSLog("ImportBridge: could not copy shared file: \(error)")
      return
    }

    emit([
      "kind": "file",
      "path": target.path,
      "name": url.deletingPathExtension().lastPathComponent,
    ])

    // "Copy to app" documents land in Documents/Inbox and are never cleaned up by iOS.
    if url.path.contains("/Documents/Inbox/") {
      try? FileManager.default.removeItem(at: url)
    }
  }

  /// Emits every `.rmx` staged by the Share Extension in the App Group container.
  /// Dart reads and deletes those files in place; `emitted` guards the window between
  /// the extension's `openURL` hop and the foreground pass.
  func drainAppGroup() {
    guard
      let container = FileManager.default.containerURL(
        forSecurityApplicationGroupIdentifier: Self.appGroup)?
        .appendingPathComponent("import", isDirectory: true),
      let entries = try? FileManager.default.contentsOfDirectory(
        at: container, includingPropertiesForKeys: nil)
    else { return }

    for entry in entries {
      guard entry.pathExtension.lowercased() == "rmx" else { continue }
      guard !emitted.contains(entry.path) else { continue }
      emitted.insert(entry.path)
      emit([
        "kind": "file",
        "path": entry.path,
        "name": Self.displayName(of: entry),
      ])
    }
  }

  /// The extension writes `<uuid>__<original name>.rmx`.
  private static func displayName(of url: URL) -> String {
    let base = url.deletingPathExtension().lastPathComponent
    if let separator = base.range(of: "__") {
      return String(base[separator.upperBound...])
    }
    return base
  }

  private func emit(_ payload: [String: Any]) {
    let deliver = { [weak self] in
      guard let self = self else { return }
      if self.dartReady {
        self.channel?.invokeMethod("import", arguments: payload)
      } else {
        self.pending.append(payload)
      }
    }
    if Thread.isMainThread {
      deliver()
    } else {
      DispatchQueue.main.async(execute: deliver)
    }
  }
}

class SceneDelegate: FlutterSceneDelegate {
  override func scene(
    _ scene: UIScene,
    willConnectTo session: UISceneSession,
    options connectionOptions: UIScene.ConnectionOptions
  ) {
    super.scene(scene, willConnectTo: session, options: connectionOptions)
    ImportBridge.shared.handle(urls: connectionOptions.urlContexts.map { $0.url })
    ImportBridge.shared.drainAppGroup()
  }

  override func scene(_ scene: UIScene, openURLContexts URLContexts: Set<UIOpenURLContext>) {
    super.scene(scene, openURLContexts: URLContexts)
    ImportBridge.shared.handle(urls: URLContexts.map { $0.url })
  }

  override func sceneWillEnterForeground(_ scene: UIScene) {
    super.sceneWillEnterForeground(scene)
    ImportBridge.shared.drainAppGroup()
  }
}
