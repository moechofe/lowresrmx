// Copyright 2016-2024 Timo Kloss
// Copyright 2021-2026 Martin Mauchauffée

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

import UIKit

class SplashViewController: UIViewController
{
	@IBOutlet var splashImageView: UIImageView!
	@IBOutlet var nxView: LowResRMXView!

	private var coreWrapper = CoreWrapper()
	private var displayLink: CADisplayLink!
	private var audioPlayer: LowResRMXAudioPlayer!

	private var isSetupDone = false
	private var isAnimationDone = false

	private var screenScale: Double = 1.0
	private var screenWidth: CGFloat = 0.0
	private var screenHeight: CGFloat = 0.0

	// safe area insets
	private var top: CGFloat = 0.0
	private var left: CGFloat = 0.0
	private var right: CGFloat = 0.0
	private var bottom: CGFloat = 0.0

	override func viewDidLoad()
	{
		super.viewDidLoad()

		nxView.coreWrapper = coreWrapper
		audioPlayer = LowResRMXAudioPlayer(coreWrapper: coreWrapper)
		displayLink = createDisplayLink()

		loadIntro()

		ProjectManager.shared.setup
		{
			self.isSetupDone = true
			self.checkStart()
		}
	}

	override func viewDidLayoutSubviews()
	{
		super.viewDidLayoutSubviews()

		view.backgroundColor = .white

		computeScreenInfos()
	}

	private func computeScreenInfos()
	{
		if #available(iOS 11.0, *)
		{
			top = view.safeAreaInsets.top
			left = view.safeAreaInsets.left
			right = view.safeAreaInsets.right
			bottom = view.safeAreaInsets.bottom
		}
		else
		{
			top = 0
			left = 0
			right = 0
			bottom = 0
		}

		// compute size of the nxview

		screenWidth = view.bounds.width
		screenHeight = view.bounds.height
		let r = screenWidth / screenHeight

		var width: CGFloat
		var height: CGFloat

		if r >= 9.0 / 16.0
		{
			width = screenWidth
			screenScale = width / 216.0
			height = 384.0 * screenScale
		}
		else
		{
			height = screenHeight
			screenScale = height / 384.0
			width = 216.0 * screenScale
		}

		nxView.frame = CGRect(
			x: 0,
			y: 0,
			width: width,
			height: height
		)
	}

	override func viewDidAppear(_ animated: Bool)
	{
		super.viewDidAppear(animated)
		audioPlayer.start()
		displayLink.add(to: .current, forMode: .default)
	}

	override func viewWillDisappear(_ animated: Bool)
	{
		super.viewWillDisappear(animated)
		displayLink.invalidate()
		audioPlayer.stop()
	}

	private func loadIntro()
	{
		let url = Bundle.main.url(forResource: "splash", withExtension: "rmx")!
		let sourceCode = try! String(contentsOf: url)
		let error = coreWrapper.compileProgram(sourceCode: sourceCode)
		guard error == nil else { fatalError() }

		computeScreenInfos()
		coreWrapper.input.width = Int32(screenWidth / screenScale)
		coreWrapper.input.height = Int32(screenHeight / screenScale)
		coreWrapper.input.left = Int32(left / screenScale)
		coreWrapper.input.top = Int32(top / screenScale)
		coreWrapper.input.right = Int32(right / screenScale)
		coreWrapper.input.bottom = Int32(bottom / screenScale)
		core_handleInput(&coreWrapper.core, &coreWrapper.input)

		core_willRunProgram(&coreWrapper.core, Int(CFAbsoluteTimeGetCurrent() - AppController.shared.bootTime))
		machine_poke(&coreWrapper.core, 0xA000, 1)
	}

	private func createDisplayLink() -> CADisplayLink
	{
		let displayLink = CADisplayLink(target: self, selector: #selector(update))
		if #available(iOS 10.0, *)
		{
			displayLink.preferredFramesPerSecond = 60
		}
		else
		{
			displayLink.frameInterval = 1
		}
		return displayLink
	}

	@objc private func update(displaylink _: CADisplayLink)
	{
		coreWrapper.input.width = Int32(screenWidth / screenScale)
		coreWrapper.input.height = Int32(screenHeight / screenScale)
		coreWrapper.input.left = Int32(left / screenScale)
		coreWrapper.input.top = Int32(top / screenScale)
		coreWrapper.input.right = Int32(right / screenScale)
		coreWrapper.input.bottom = Int32(bottom / screenScale)

		withUnsafeMutablePointer(to: &coreWrapper.input)
		{ inputPtr in
			core_update(&coreWrapper.core, inputPtr)
		}
		nxView.render()

		if machine_peek(&coreWrapper.core, 0xA000) == 2
		{
			machine_poke(&coreWrapper.core, 0xA000, 3)
			isAnimationDone = true
			checkStart()
		}
	}

	private func checkStart()
	{
		if isSetupDone, isAnimationDone
		{
			showApp()
		}
	}

	private func showApp()
	{
		if let window = view.window
		{
			if let vc = storyboard?.instantiateViewController(withIdentifier: "AppStart")
			{
				window.rootViewController = vc
				UIView.transition(with: window, duration: 0.3, options: .transitionCrossDissolve, animations: {
					window.rootViewController = vc
				}, completion: nil)
			}
		}
	}
}
