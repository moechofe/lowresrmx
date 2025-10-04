//
//  FontSizePickerViewController.swift
//  LowResRMX
//
//  Created by Assistant on 24/11/24.
//  Copyright © 2024 Inutilis Software. All rights reserved.
//

import UIKit

class FontSizePickerViewController: UIViewController {

    private let slider = UISlider()
    private let previewLabel = UILabel()
    private let sizeLabel = UILabel()

    private let minFontSize: CGFloat = 8.0
    private let maxFontSize: CGFloat = 24.0

    override func viewDidLoad() {
        super.viewDidLoad()

        setupUI()
        updatePreview()
    }

    private func setupUI() {
        view.backgroundColor = UIColor.systemBackground

        title = "Font Size"

        // Navigation bar buttons
        navigationItem.leftBarButtonItem = UIBarButtonItem(barButtonSystemItem: .cancel, target: self, action: #selector(cancelTapped))
        navigationItem.rightBarButtonItem = UIBarButtonItem(barButtonSystemItem: .done, target: self, action: #selector(doneTapped))

        // Container for content
        let containerView = UIView()
        containerView.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(containerView)

        // Size label
        sizeLabel.text = String(format: "%.0f pt", AppController.shared.editorFontSize)
        sizeLabel.textAlignment = .center
        sizeLabel.font = UIFont.systemFont(ofSize: 16, weight: .medium)
        sizeLabel.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(sizeLabel)

        // Slider
        slider.minimumValue = Float(minFontSize)
        slider.maximumValue = Float(maxFontSize)
        slider.value = Float(AppController.shared.editorFontSize)
        slider.addTarget(self, action: #selector(sliderValueChanged), for: .valueChanged)
        slider.translatesAutoresizingMaskIntoConstraints = false
        containerView.addSubview(slider)

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
            containerView.leadingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.leadingAnchor, constant: 20),
            containerView.trailingAnchor.constraint(equalTo: view.safeAreaLayoutGuide.trailingAnchor, constant: -20),
            containerView.topAnchor.constraint(equalTo: view.safeAreaLayoutGuide.topAnchor, constant: 20),
            containerView.bottomAnchor.constraint(lessThanOrEqualTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -20),

            sizeLabel.topAnchor.constraint(equalTo: containerView.topAnchor),
            sizeLabel.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            sizeLabel.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            slider.topAnchor.constraint(equalTo: sizeLabel.bottomAnchor, constant: 20),
            slider.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            slider.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),

            previewContainer.topAnchor.constraint(equalTo: slider.bottomAnchor, constant: 30),
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

        sizeLabel.text = String(format: "%.0f pt", size)
        previewLabel.font = UIFont(name: "Menlo", size: size) ?? UIFont.monospacedSystemFont(ofSize: size, weight: .regular)
    }

    @objc private func cancelTapped() {
        dismiss(animated: true, completion: nil)
    }

    @objc private func doneTapped() {
        let newFontSize = CGFloat(slider.value)
        AppController.shared.editorFontSize = newFontSize
        NotificationCenter.default.post(name: .EditorFontSizeDidChange, object: nil)
        dismiss(animated: true, completion: nil)
    }
}
