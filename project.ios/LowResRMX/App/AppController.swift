//
//  AppController.swift
//  LowResRMX
//
//  Created by Timo Kloss on 22/02/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit
import StoreKit

extension Notification.Name {
    static let EditorFontSizeDidChange = Notification.Name("EditorFontSizeDidChange")
}

@objc class AppController: NSObject {

    private static let hasDontatedKey = "hasDontated"
    private static let isSafeScaleEnabledKey = "isSafeScaleEnabled"
    private static let forcesSmallGamepadKey = "forcesSmallGamepad"
    private static let numRunProgramsThisVersionKey = "numRunProgramsThisVersion"
    private static let lastVersionKey = "lastVersion"
    private static let lastVersionPromptedForReviewKey = "lastVersionPromptedForReview"
    private static let userIdKey = "userIdKey"
    private static let usernameKey = "usernameKey"
    private static let editorFontSizeKey = "editorFontSize"
    private static let editorLabelIndexModeKey = "editorLabelIndexMode"
    private static let editorProcedureIndexModeKey = "editorProcedureIndexMode"
    private static let editorManualMarkerIndexModeKey = "editorManualMarkerIndexMode"
    private static let editorIndexModeKey = "editorIndexMode"
    private static let editorSyntaxHighlightingModeKey = "editorSyntaxHighlightingMode"

    @objc static let shared = AppController()

    @objc weak var tabBarController: TabBarController!

    @objc let helpContent: HelpContent
    @objc let bootTime: CFAbsoluteTime

    private var webSource: WebSource?

    var hasDontated: Bool {
        get {
            return UserDefaults.standard.bool(forKey: AppController.hasDontatedKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.hasDontatedKey)
        }
    }

    var isSafeScaleEnabled: Bool {
        get {
            return UserDefaults.standard.bool(forKey: AppController.isSafeScaleEnabledKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.isSafeScaleEnabledKey)
        }
    }

    var forcesSmallGamepad: Bool {
        get {
            return UserDefaults.standard.bool(forKey: AppController.forcesSmallGamepadKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.forcesSmallGamepadKey)
        }
    }

    var numRunProgramsThisVersion: Int {
        get {
            return UserDefaults.standard.integer(forKey: AppController.numRunProgramsThisVersionKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.numRunProgramsThisVersionKey)
        }
    }

    private(set) var userId: String? {
        get {
            return UserDefaults.standard.string(forKey: AppController.userIdKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.userIdKey)
        }
    }

    private(set) var username: String? {
        get {
            return UserDefaults.standard.string(forKey: AppController.usernameKey)
        }
        set {
            UserDefaults.standard.set(newValue, forKey: AppController.usernameKey)
        }
    }

    var editorFontSize: CGFloat {
        get {
            let size = UserDefaults.standard.double(forKey: AppController.editorFontSizeKey)
            return size > 0 ? CGFloat(size) : 14.0 // default font size
        }
        set {
            UserDefaults.standard.set(Double(newValue), forKey: AppController.editorFontSizeKey)
        }
    }

    var editorLabelIndexMode: LabelIndexMode {
        get {
            let value = UserDefaults.standard.integer(forKey: AppController.editorLabelIndexModeKey)
            return LabelIndexMode(rawValue: value) ?? .allLabels
        }
        set {
            UserDefaults.standard.set(newValue.rawValue, forKey: AppController.editorLabelIndexModeKey)
        }
    }

    var editorProcedureIndexMode: ProcedureIndexMode {
        get {
            let value = UserDefaults.standard.integer(forKey: AppController.editorProcedureIndexModeKey)
            return ProcedureIndexMode(rawValue: value) ?? .allProcedures
        }
        set {
            UserDefaults.standard.set(newValue.rawValue, forKey: AppController.editorProcedureIndexModeKey)
        }
    }

    var editorManualMarkerIndexMode: MarkerIndexMode {
        get {
            let value = UserDefaults.standard.integer(forKey: AppController.editorManualMarkerIndexModeKey)
            return MarkerIndexMode(rawValue: value) ?? .noMarkers
        }
        set {
            UserDefaults.standard.set(newValue.rawValue, forKey: AppController.editorManualMarkerIndexModeKey)
        }
    }

    var editorIndexMode: IndexMode {
        get {
            let value = UserDefaults.standard.integer(forKey: AppController.editorIndexModeKey)
            return IndexMode(rawValue: value) ?? .labelsAndProcedures
        }
        set {
            UserDefaults.standard.set(newValue.rawValue, forKey: AppController.editorIndexModeKey)
        }
    }

    @objc var editorSyntaxHighlightingMode: SyntaxHighlightingMode {
        get {
            let value = UserDefaults.standard.integer(forKey: AppController.editorSyntaxHighlightingModeKey)
            return SyntaxHighlightingMode(rawValue: value) ?? .none
        }
        set {
            UserDefaults.standard.set(newValue.rawValue, forKey: AppController.editorSyntaxHighlightingModeKey)
        }
    }

    var currentVersion: String {
        let version = Bundle.main.object(forInfoDictionaryKey: kCFBundleVersionKey as String) as! String
        return version
    }

    private override init() {
        let url = Bundle.main.url(forResource: "manual", withExtension: "html", subdirectory:"asset.manual")!
        helpContent = HelpContent(url: url)

        bootTime = CFAbsoluteTimeGetCurrent()

        super.init()

        UserDefaults.standard.register(defaults: [
            AppController.editorIndexModeKey: IndexMode.manualMarkers.rawValue,
            AppController.editorSyntaxHighlightingModeKey: SyntaxHighlightingMode.syntax.rawValue
        ])

        let lastVersion = UserDefaults.standard.string(forKey: AppController.lastVersionKey)
        if currentVersion != lastVersion {
            numRunProgramsThisVersion = 0
            UserDefaults.standard.set(currentVersion, forKey: AppController.lastVersionKey)
        }
    }

    func requestAppStoreReview() {
        let lastVersionPromptedForReview = UserDefaults.standard.string(forKey: AppController.lastVersionPromptedForReviewKey)
        if currentVersion != lastVersionPromptedForReview {
            // iOS 14+ only (minimum supported is 15.5)
            if let windowScene = UIApplication.shared.connectedScenes
                .compactMap({ $0 as? UIWindowScene })
                .first(where: { $0.activationState == .foregroundActive }) {
                SKStoreReviewController.requestReview(in: windowScene)
            }
            // Do nothing if no active scene is found
            UserDefaults.standard.set(currentVersion, forKey: AppController.lastVersionPromptedForReviewKey)
        }
    }

    func runProgram(_ webSource: WebSource) {
        if tabBarController != nil {
            showProgram(webSource)
        } else {
            self.webSource = webSource
        }
    }

    @objc func checkShowProgram() {
        if let webSource = webSource {
            showProgram(webSource)
            self.webSource = nil
        }
    }

    private func showProgram(_ webSource: WebSource) {
        let storyboard = UIStoryboard(name: "LowResRMX", bundle: nil)
        let vc = storyboard.instantiateInitialViewController() as! LowResRMXViewController
        vc.webSource = webSource

        if tabBarController.presentedViewController != nil {
            tabBarController.dismiss(animated: false, completion: nil)
        }
        tabBarController.present(vc, animated: true, completion: nil)
    }

    func didLogIn(userId: String, username: String) {
        self.userId = userId
        self.username = username
    }

    func didLogOut() {
        self.userId = nil
        self.username = nil
    }

}
