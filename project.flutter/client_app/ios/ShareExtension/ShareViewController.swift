import UIKit

/// Headless share extension: copies every shared `.rmx` into the App Group container
/// and wakes the host app, which imports and deletes the staged copies.
final class ShareViewController: UIViewController {
  private let appGroup = "group.it.ro.ret.flutter.LowResRMX"
  private let rmxType = "it.ro.ret.document.rmx"

  override func viewDidLoad() {
    super.viewDidLoad()

    let items = extensionContext?.inputItems as? [NSExtensionItem] ?? []
    let providers = items.flatMap { $0.attachments ?? [] }
      .filter { $0.hasItemConformingToTypeIdentifier(rmxType) }

    guard !providers.isEmpty else {
      finish(staged: 0)
      return
    }

    let lock = NSLock()
    var staged = 0
    let group = DispatchGroup()

    for provider in providers {
      group.enter()
      provider.loadFileRepresentation(forTypeIdentifier: rmxType) { [weak self] url, _ in
        defer { group.leave() }
        guard let self = self, let url = url else { return }
        // The handed URL is only valid for the duration of this callback.
        if self.copyToAppGroup(url) {
          lock.lock()
          staged += 1
          lock.unlock()
        }
      }
    }

    group.notify(queue: .main) { [weak self] in
      self?.finish(staged: staged)
    }
  }

  private func copyToAppGroup(_ url: URL) -> Bool {
    guard
      let container = FileManager.default.containerURL(
        forSecurityApplicationGroupIdentifier: appGroup)?
        .appendingPathComponent("import", isDirectory: true)
    else { return false }

    do {
      try FileManager.default.createDirectory(at: container, withIntermediateDirectories: true)
      // ImportBridge.drainAppGroup splits on "__" to recover the display name.
      let target = container.appendingPathComponent(
        "\(UUID().uuidString)__\(url.lastPathComponent)")
      try FileManager.default.copyItem(at: url, to: target)
      return true
    } catch {
      NSLog("ShareExtension: could not stage \(url.lastPathComponent): \(error)")
      return false
    }
  }

  private func finish(staged: Int) {
    if staged > 0 {
      openHostApp()
    }
    extensionContext?.completeRequest(returningItems: nil, completionHandler: nil)
  }

  /// Walks the responder chain for `openURL:`. If nothing answers, the staged files stay in
  /// the App Group container and the app drains them on its next foreground.
  private func openHostApp() {
    guard let url = URL(string: "lowresrmx://import") else { return }
    let selector = sel_registerName("openURL:")
    var responder: UIResponder? = self
    while let current = responder {
      if current.responds(to: selector) {
        current.perform(selector, with: url)
        return
      }
      responder = current.next
    }
  }
}
