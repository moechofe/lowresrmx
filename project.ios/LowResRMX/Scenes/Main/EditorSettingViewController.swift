//
//  EditorSettingViewController.swift
//  LowResRMX
//
//  Created by Assistant on 24/11/24.
//  Copyright © 2024 Inutilis Software. All rights reserved.
//

import UIKit

// NOTE: These should ideally be in a shared location if used across multiple files.
extension Notification.Name {

    static let EditorIndexModeDidChange = Notification.Name("EditorIndexModeDidChange")
    static let EditorSyntaxHighlightingDidChange = Notification.Name("EditorSyntaxHighlightingDidChange")
}

@objc enum LabelIndexMode: Int, CaseIterable {
    case noLabels
    case allLabels
    case labelsWithoutUnderscore

    var title: String {
        switch self {
        case .noLabels: return "No Labels"
        case .allLabels: return "All Labels"
        case .labelsWithoutUnderscore: return "Labels w/o '_'"
        }
    }
}

@objc enum ProcedureIndexMode: Int, CaseIterable {
    case noProcedures
    case allProcedures

    var title: String {
        switch self {
        case .noProcedures: return "No Subs"
        case .allProcedures: return "All Subs"
        }
    }
}

@objc enum MarkerIndexMode: Int, CaseIterable {
    case noMarkers
    case manualMarkers

    var title: String {
        switch self {
        case .noMarkers: return "No Markers"
        case .manualMarkers: return "Manual Markers"
        }
    }
}

@objc enum SyntaxHighlightingMode: Int, CaseIterable {
    case none
    case syntax
    // case manualMarkers

    var title: String {
        switch self {
        case .none: return "None"
        case .syntax: return "Syntax"
        // case .manualMarkers: return "Markers"
        }
    }
}

class EditorSettingViewController: UIViewController {

    private let slider = UISlider()
    private let previewLabel = UILabel()
    private let sizeLabel = UILabel()

    private let labelIndexModeSegmentedControl = UISegmentedControl()
    private let procedureIndexModeSegmentedControl = UISegmentedControl()
    private let markerIndexModeSegmentedControl = UISegmentedControl()
    // private let indexModeSegmentedControl = UISegmentedControl()
    private let syntaxHighlightingSegmentedControl = UISegmentedControl()

    private let minFontSize: CGFloat = 8.0
    private let maxFontSize: CGFloat = 24.0

    override func viewDidLoad() {
        super.viewDidLoad()

        setupUI()
        updatePreview()
    }

    private func setupUI() {
        view.backgroundColor = UIColor.systemBackground

        title = "Editor Settings"

        // Navigation bar buttons
        navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .cancel, target: self, action: #selector(cancelTapped))
        navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(doneTapped))

        // Scroll View
        let scrollView = UIScrollView()
        scrollView.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(scrollView)

        // Container for content
        let containerView = UIView()
        containerView.translatesAutoresizingMaskIntoConstraints = false
        scrollView.addSubview(containerView)

        // Size label
        sizeLabel.text = String(format: "Font size: %.0f pt", AppController.shared.editorFontSize)
        sizeLabel.font = UIFont.systemFont(ofSize: 17, weight: .semibold)
        sizeLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(sizeLabel)

        // Slider
        slider.minimumValue = Float(minFontSize)
        slider.maximumValue = Float(maxFontSize)
        slider.value = Float(AppController.shared.editorFontSize)
        slider.addTarget(self, action: #selector(sliderValueChanged), for: .valueChanged)
        slider.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(slider)

        // Label Index Mode
        let labelIndexModeLabel = UILabel()
        labelIndexModeLabel.text = "Index Labels"
        labelIndexModeLabel.font = UIFont.systemFont(ofSize: 17, weight: .semibold)
        labelIndexModeLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(labelIndexModeLabel)

        for (index, mode) in LabelIndexMode.allCases.enumerated() {
            labelIndexModeSegmentedControl.insertSegment(withTitle: mode.title, at: index, animated: false)
        }
        labelIndexModeSegmentedControl.selectedSegmentIndex = AppController.shared.editorLabelIndexMode.rawValue
        labelIndexModeSegmentedControl.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(labelIndexModeSegmentedControl)

        let labelIndexModeDescriptionLabel = UILabel()
        labelIndexModeDescriptionLabel.text = "Which label declarations appear in the sidebar index."
        labelIndexModeDescriptionLabel.font = UIFont.systemFont(ofSize: 14)
        labelIndexModeDescriptionLabel.textColor = UIColor.secondaryLabel
        labelIndexModeDescriptionLabel.numberOfLines = 0
        labelIndexModeDescriptionLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(labelIndexModeDescriptionLabel)

        // Procedure Index Mode
        let procedureIndexModeLabel = UILabel()
        procedureIndexModeLabel.text = "Index Subs"
        procedureIndexModeLabel.font = UIFont.systemFont(ofSize: 17, weight: .semibold)
        procedureIndexModeLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(procedureIndexModeLabel)

        for (index, mode) in ProcedureIndexMode.allCases.enumerated() {
            procedureIndexModeSegmentedControl.insertSegment(withTitle: mode.title, at: index, animated: false)
        }
        procedureIndexModeSegmentedControl.selectedSegmentIndex = AppController.shared.editorProcedureIndexMode.rawValue
        procedureIndexModeSegmentedControl.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(procedureIndexModeSegmentedControl)

        let prodecureIndexModeDescriptionLabel = UILabel()
        prodecureIndexModeDescriptionLabel.text = "Show subroutine declarations in the sidebar index."
        prodecureIndexModeDescriptionLabel.font = UIFont.systemFont(ofSize: 14)
        prodecureIndexModeDescriptionLabel.textColor = UIColor.secondaryLabel
        prodecureIndexModeDescriptionLabel.numberOfLines = 0
        prodecureIndexModeDescriptionLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(prodecureIndexModeDescriptionLabel)

        // Marker Index Mode
        let markerIndexModeLabel = UILabel()
        markerIndexModeLabel.text = "Index Markers"
        markerIndexModeLabel.font = UIFont.systemFont(ofSize: 17, weight: .semibold)
        markerIndexModeLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(markerIndexModeLabel)

        for (index, mode) in MarkerIndexMode.allCases.enumerated() {
            markerIndexModeSegmentedControl.insertSegment(withTitle: mode.title, at: index, animated: false)
        }
        markerIndexModeSegmentedControl.selectedSegmentIndex = AppController.shared.editorManualMarkerIndexMode.rawValue
        markerIndexModeSegmentedControl.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(markerIndexModeSegmentedControl)

        let markerIndexModeDescriptionLabel = UILabel()
        markerIndexModeDescriptionLabel.text = """
Show manual markers in the sidebar index.
Manual markers start by 3 single quotes (''').
"""
        markerIndexModeDescriptionLabel.font = UIFont.systemFont(ofSize: 14)
        markerIndexModeDescriptionLabel.textColor = UIColor.secondaryLabel
        markerIndexModeDescriptionLabel.numberOfLines = 0
        markerIndexModeDescriptionLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(markerIndexModeDescriptionLabel)

        // Text Coloration
        let syntaxHighlightingLabel = UILabel()
        syntaxHighlightingLabel.text = "Text Coloration"
        syntaxHighlightingLabel.font = UIFont.systemFont(ofSize: 17, weight: .semibold)
        syntaxHighlightingLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(syntaxHighlightingLabel)

        for (index, mode) in SyntaxHighlightingMode.allCases.enumerated() {
            syntaxHighlightingSegmentedControl.insertSegment(withTitle: mode.title, at: index, animated: false)
        }
        syntaxHighlightingSegmentedControl.selectedSegmentIndex = AppController.shared.editorSyntaxHighlightingMode.rawValue
        syntaxHighlightingSegmentedControl.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(syntaxHighlightingSegmentedControl)

        // Preview label
        previewLabel.text = """
        sub r
        poke $ff00,r(((raster+timer*2.5+cos(timer*0.01)*sin(timer*0.005)*20)*0.12+(ubound(r)+1)) mod (ubound(r)+1))
        end sub

        c: data 8, 9, 10, 13, 14, 62, 63, 55, 54, 53, 52, 51, 49, 50, 44, 45, 46, 47, 48, 56, 57, 58, 59, 60, 61, 12, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 11

        dim global r(55)
        for i=0 to ubound(r)
        read r(i)
        next i

        on raster call r

        do : wait vbl : loop
        """
        previewLabel.numberOfLines = 0
        previewLabel.font = UIFont(name: "Menlo", size: AppController.shared.editorFontSize) ?? UIFont.monospacedSystemFont(ofSize: AppController.shared.editorFontSize, weight: .regular)
        previewLabel.backgroundColor = UIColor.secondarySystemBackground
        previewLabel.layer.cornerRadius = 8
        previewLabel.layer.masksToBounds = true
        previewLabel.translatesAutoresizingMaskIntoConstraints = false

        // Add padding to preview
        let previewContainer = UIView()
        previewContainer.backgroundColor = UIColor.secondarySystemBackground
        previewContainer.layer.cornerRadius = 8
        previewContainer.translatesAutoresizingMaskIntoConstraints = false
        previewContainer.addSubview(previewLabel)
        containerView.addSubview(previewContainer)

        // Constraints
        NSLayoutConstraint.activate([
            scrollView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor),
            scrollView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor),
            scrollView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor),
            scrollView.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor),

            containerView.topAnchor.constraint(equalTo: scrollView.contentLayoutGuide.topAnchor, constant: 20),
            containerView.leadingAnchor.constraint(equalTo: scrollView.contentLayoutGuide.leadingAnchor, constant: 20),
            containerView.trailingAnchor.constraint(equalTo: scrollView.contentLayoutGuide.trailingAnchor, constant: -20),
            containerView.bottomAnchor.constraint(equalTo: scrollView.contentLayoutGuide.bottomAnchor, constant: -20),
            containerView.widthAnchor.constraint(equalTo: scrollView.frameLayoutGuide.widthAnchor, constant: -40),

            sizeLabel.topAnchor.constraint(equalTo: containerView.topAnchor),
            sizeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            sizeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            slider.topAnchor.constraint(equalTo: sizeLabel.bottomAnchor, constant: 8),
            slider.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            slider.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            labelIndexModeLabel.topAnchor.constraint(equalTo: slider.bottomAnchor, constant: 30),
            labelIndexModeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            labelIndexModeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            labelIndexModeSegmentedControl.topAnchor.constraint(equalTo: labelIndexModeLabel.bottomAnchor, constant: 8),
            labelIndexModeSegmentedControl.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            labelIndexModeSegmentedControl.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            labelIndexModeDescriptionLabel.topAnchor.constraint(equalTo: labelIndexModeSegmentedControl.bottomAnchor, constant: 8),
            labelIndexModeDescriptionLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            labelIndexModeDescriptionLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            procedureIndexModeLabel.topAnchor.constraint(equalTo: labelIndexModeDescriptionLabel.bottomAnchor, constant: 20),
            procedureIndexModeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            procedureIndexModeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            procedureIndexModeSegmentedControl.topAnchor.constraint(equalTo: procedureIndexModeLabel.bottomAnchor, constant: 8),
            procedureIndexModeSegmentedControl.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            procedureIndexModeSegmentedControl.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            prodecureIndexModeDescriptionLabel.topAnchor.constraint(equalTo: procedureIndexModeSegmentedControl.bottomAnchor, constant: 8),
            prodecureIndexModeDescriptionLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            prodecureIndexModeDescriptionLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            markerIndexModeLabel.topAnchor.constraint(equalTo: prodecureIndexModeDescriptionLabel.bottomAnchor, constant: 20),
            markerIndexModeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            markerIndexModeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            markerIndexModeSegmentedControl.topAnchor.constraint(equalTo: markerIndexModeLabel.bottomAnchor, constant: 8),
            markerIndexModeSegmentedControl.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            markerIndexModeSegmentedControl.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            markerIndexModeDescriptionLabel.topAnchor.constraint(equalTo: markerIndexModeSegmentedControl.bottomAnchor, constant: 8),
            markerIndexModeDescriptionLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            markerIndexModeDescriptionLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            // indexModeLabel.topAnchor.constraint(equalTo: markerIndexModeDescriptionLabel.bottomAnchor, constant: 20),
            // indexModeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            // indexModeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            // indexModeSegmentedControl.topAnchor.constraint(equalTo: indexModeLabel.bottomAnchor, constant: 8),
            // indexModeSegmentedControl.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            // indexModeSegmentedControl.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            syntaxHighlightingLabel.topAnchor.constraint(equalTo: markerIndexModeDescriptionLabel.bottomAnchor, constant: 20),
            syntaxHighlightingLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            syntaxHighlightingLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            syntaxHighlightingSegmentedControl.topAnchor.constraint(equalTo: syntaxHighlightingLabel.bottomAnchor, constant: 8),
            syntaxHighlightingSegmentedControl.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            syntaxHighlightingSegmentedControl.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            previewContainer.topAnchor.constraint(equalTo: syntaxHighlightingSegmentedControl.bottomAnchor, constant: 30),
            previewContainer.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            previewContainer.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
            previewContainer.bottomAnchor.constraint(equalTo: containerView.bottomAnchor),

            previewLabel.topAnchor.constraint(equalTo: previewContainer.topAnchor, constant: 12),
            previewLabel.leadingAnchor.constraint(equalTo: previewContainer.leadingAnchor, constant: 12),
            previewLabel.trailingAnchor.constraint(equalTo: previewContainer.trailingAnchor, constant: -12),
            previewLabel.bottomAnchor.constraint(equalTo: previewContainer.bottomAnchor, constant: -12)
        ])
    }

    @objc private func sliderValueChanged() {
        let fontSize = CGFloat(slider.value)
        updatePreview(fontSize: fontSize)
    }

    private func updatePreview(fontSize: CGFloat? = nil) {
        let size = fontSize ?? AppController.shared.editorFontSize

        sizeLabel.text = String(format: "Font size: %.0f pt", size)
        previewLabel.font = UIFont(name: "Menlo", size: size) ?? UIFont.monospacedSystemFont(ofSize: size, weight: .regular)
    }

    @objc private func cancelTapped() {
        dismiss(animated: true, completion: nil)
    }

    @objc private func doneTapped() {
        let newFontSize = CGFloat(slider.value)
        if newFontSize != AppController.shared.editorFontSize {
            AppController.shared.editorFontSize = newFontSize
            // NotificationCenter.default.post(name: .EditorFontSizeDidChange, object: nil)
        }

        let newLabelIndexMode = LabelIndexMode(rawValue: labelIndexModeSegmentedControl.selectedSegmentIndex) ?? .allLabels
        if newLabelIndexMode != AppController.shared.editorLabelIndexMode {
            AppController.shared.editorLabelIndexMode = newLabelIndexMode
        }

        let newProcedureIndexMode = ProcedureIndexMode(rawValue: procedureIndexModeSegmentedControl.selectedSegmentIndex) ?? .allProcedures
        if newProcedureIndexMode != AppController.shared.editorProcedureIndexMode {
            AppController.shared.editorProcedureIndexMode = newProcedureIndexMode
        }

        let newMarkerIndexMode = MarkerIndexMode(rawValue: markerIndexModeSegmentedControl.selectedSegmentIndex) ?? .noMarkers
        if newMarkerIndexMode != AppController.shared.editorManualMarkerIndexMode {
            AppController.shared.editorManualMarkerIndexMode = newMarkerIndexMode
        }

        let newSyntaxMode = SyntaxHighlightingMode(rawValue: syntaxHighlightingSegmentedControl.selectedSegmentIndex) ?? .syntax
        if newSyntaxMode != AppController.shared.editorSyntaxHighlightingMode {
            AppController.shared.editorSyntaxHighlightingMode = newSyntaxMode
            NotificationCenter.default.post(name: .EditorSyntaxHighlightingDidChange, object: nil)
        }
        dismiss(animated: true, completion: nil)
    }
}
