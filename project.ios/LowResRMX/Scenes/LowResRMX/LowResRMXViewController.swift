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

import GameController
import ReplayKit
import UIKit

protocol LowResRMXViewControllerDelegate: AnyObject
{
	func didChangeDebugMode(enabled: Bool)
	func didEndWithError(_ error: LowResRMXError)
	func didEndWithKeyCommand()
}

protocol LowResRMXViewControllerToolDelegate: AnyObject
{
	func nxSourceCodeForVirtualDisk() -> String
	func nxDidSaveVirtualDisk(sourceCode: String)
}

struct WebSource
{
	let name: String
	let programUrl: URL
	let imageUrl: URL?
	let topicId: String?
}

class LowResRMXViewController: UIViewController, UIKeyInput, CoreWrapperDelegate,
	RPPreviewViewControllerDelegate
{
	let screenshotScaleFactor: CGFloat = 4.0

	@IBOutlet var exitButton: UIButton!
	@IBOutlet var menuButton: UIButton!
	@IBOutlet var nxView: LowResRMXView!

	weak var delegate: LowResRMXViewControllerDelegate?
	weak var toolDelegate: LowResRMXViewControllerToolDelegate?
	var webSource: WebSource?
	var document: ProjectDocument?
	var diskDocument: ProjectDocument?
	var coreWrapper: CoreWrapper?
	var imageData: Data?

	var screenScale: Double = 1.0
	var screenWidth: CGFloat = 0.0
	var screenHeight: CGFloat = 0.0

	var isCompatMode: Bool = false

	// safe area insets
	var top: CGFloat = 0.0
	var left: CGFloat = 0.0
	var right: CGFloat = 0.0
	var bottom: CGFloat = 0.0

	var isDebugEnabled = false
	{
		didSet
		{
			if let coreWrapper
			{
				core_setDebug(&coreWrapper.core, isDebugEnabled)
			}
		}
	}

	var isSafeScaleEnabled = false

	private var controlsInfo: ControlsInfo = .init()
	private var displayLink: CADisplayLink?
	private var errorToShow: Error?
	private var recognizer: UITapGestureRecognizer?
	private var startDate: Date!
	private var audioPlayer: LowResRMXAudioPlayer!
	private var numOnscreenGamepads = 0
	private var keyboardTop: CGFloat?
	private var keyboardWasShown = false
	private var keyboardWasHidden = false
	private var keyboardNewHeight = 0

	override func viewDidLoad()
	{
		super.viewDidLoad()

		//    addKeyCommand(
		//      UIKeyCommand(
		//        input: "e", modifierFlags: .command, action: #selector(onExitTapped),
		//        discoverabilityTitle: "Exit Program"))
		//    addKeyCommand(
		//      UIKeyCommand(
		//        input: "d", modifierFlags: .command, action: #selector(toggleDebugMode),
		//        discoverabilityTitle: "Debug Mode"))
		//    addKeyCommand(
		//      UIKeyCommand(
		//        input: "s", modifierFlags: .command, action: #selector(shareScreenshot),
		//        discoverabilityTitle: "Share Screenshot"))
		//    addKeyCommand(
		//      UIKeyCommand(
		//        input: "i", modifierFlags: .command, action: #selector(captureProgramIcon),
		//        discoverabilityTitle: "Capture Icon"))

		startDate = Date()

		isSafeScaleEnabled = AppController.shared.isSafeScaleEnabled

		if let coreWrapper
		{
			computeScreenInfos()
			coreWrapper.input.width = Int32(screenWidth / screenScale)
			coreWrapper.input.height = Int32(screenHeight / screenScale)
			coreWrapper.input.left = Int32(left / screenScale)
			coreWrapper.input.top = Int32(top / screenScale)
			coreWrapper.input.right = Int32(right / screenScale)
			coreWrapper.input.bottom = Int32(bottom / screenScale)
			core_handleInput(&coreWrapper.core, &coreWrapper.input)

			// program already compiled
			core_willRunProgram(
				&coreWrapper.core, Int(CFAbsoluteTimeGetCurrent() - AppController.shared.bootTime)
			)
			core_setDebug(&coreWrapper.core, isDebugEnabled)
		}
		else if let webSource
		{
			// load program from web
			coreWrapper = CoreWrapper()

			let group = DispatchGroup()
			var sourceCode: String?
			var groupError: Error?

			group.enter()
			DispatchQueue.global().async
			{
				do
				{
					sourceCode = try String(contentsOf: webSource.programUrl, encoding: .utf8)
				}
				catch
				{
					groupError = error
				}
				group.leave()
			}

			if let imageUrl = webSource.imageUrl
			{
				group.enter()
				DispatchQueue.global().async
				{
					self.imageData = try? Data(contentsOf: imageUrl)
					group.leave()
				}
			}

			group.notify(queue: .main)
			{
				if let sourceCode
				{
					//          if let topicId = webSource.topicId {
					////            self.countPlay(topicId: topicId)
					//          }
					let error = self.compileAndStartProgram(sourceCode: sourceCode)
					if let error
					{
						self.showError(error)
					}
				}
				else if let error = groupError
				{
					self.showError(error)
				}
			}
		}
		else
		{
			// program not yet compiled, open document and compile...
			coreWrapper = CoreWrapper()

			guard let document
			else
			{
				fatalError("CoreWrapper or Document required")
			}

			if document.documentState == .closed
			{
				document.open(completionHandler: { [weak self] success in
					guard let strongSelf = self
					else
					{
						return
					}
					var error: NSError? = if success, let sourceCode = document.sourceCode
					{
						strongSelf.compileAndStartProgram(sourceCode: sourceCode)
					}
					else
					{
						NSError(
							domain: "LowResRMX", code: 0,
							userInfo: [NSLocalizedDescriptionKey: "Could not Open File"]
						)
					}
					if let error
					{
						strongSelf.showError(error)
					}
				})
			}
			else if document.documentState == .normal
			{
				if let sourceCode = document.sourceCode
				{
					errorToShow = compileAndStartProgram(sourceCode: sourceCode)
				}
			}
		}

		guard let coreWrapper
		else
		{
			assertionFailure()
			return
		}

		nxView.coreWrapper = coreWrapper
		audioPlayer = LowResRMXAudioPlayer(coreWrapper: coreWrapper)

		coreWrapper.delegate = self

		inputAssistantItem.leadingBarButtonGroups = []
		inputAssistantItem.trailingBarButtonGroups = []

		let recognizer = UITapGestureRecognizer(target: self, action: #selector(handleTap))
		recognizer.isEnabled = false
		view.addGestureRecognizer(recognizer)
		self.recognizer = recognizer

		let displayLink = CADisplayLink(target: self, selector: #selector(update))
		if #available(iOS 10.0, *)
		{
			displayLink.preferredFramesPerSecond = 60
		}
		else
		{
			displayLink.frameInterval = 1
		}
		self.displayLink = displayLink

		NotificationCenter.default.addObserver(
			self, selector: #selector(keyboardWillShow), name: UIResponder.keyboardWillShowNotification,
			object: nil
		)
		NotificationCenter.default.addObserver(
			self, selector: #selector(keyboardDidShow), name: UIResponder.keyboardDidShowNotification,
			object: nil
		)
		NotificationCenter.default.addObserver(
			self, selector: #selector(keyboardWillHide), name: UIResponder.keyboardWillHideNotification,
			object: nil
		)
		NotificationCenter.default.addObserver(
			self, selector: #selector(keyboardDidHide), name: UIResponder.keyboardDidHideNotification,
			object: nil
		)

		// Sauce: https://developer.apple.com/forums/thread/110064
		let value = UIInterfaceOrientation.portrait.rawValue
		UIDevice.current.setValue(value, forKey: "orientation")
	}

	// Saurce: https://developer.apple.com/forums/thread/110064
	override var supportedInterfaceOrientations: UIInterfaceOrientationMask
	{
		.portrait
	}

	// Saurce: https://developer.apple.com/forums/thread/110064
	override var shouldAutorotate: Bool
	{
		true
	}

	deinit
	{
		NotificationCenter.default.removeObserver(self)
	}

	override func viewDidAppear(_ animated: Bool)
	{
		super.viewDidAppear(animated)
		displayLink?.add(to: .current, forMode: .default)
		checkShowError()
	}

	override func viewWillDisappear(_ animated: Bool)
	{
		super.viewWillDisappear(animated)
		view.endEditing(true)

		displayLink?.invalidate()

		audioPlayer.stop()

		diskDocument?.close(completionHandler: nil)
		diskDocument = nil

		if let coreWrapper
		{
			core_willSuspendProgram(&coreWrapper.core)
		}
	}

	override func viewWillTransition(to _: CGSize, with _: any UIViewControllerTransitionCoordinator)
	{
		if let coreWrapper
		{
			core_orientationChanged(&coreWrapper.core)
		}
	}

	override var prefersStatusBarHidden: Bool
	{
		//        if #available(iOS 11.0, *) {
		//            if let window = UIApplication.shared.delegate?.window {
		//                if window?.safeAreaInsets.top != 0 {
		//                    return false
		//                }
		//            }
		//        }
		true
	}

	//    override var preferredStatusBarStyle: UIStatusBarStyle {
	//        return .lightContent
	//    }

	override var preferredScreenEdgesDeferringSystemGestures: UIRectEdge
	{
		.all
	}

	func computeScreenInfos()
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

	override func viewDidLayoutSubviews()
	{
		super.viewDidLayoutSubviews()

		view.backgroundColor = .black

		computeScreenInfos()
	}

	func compileAndStartProgram(sourceCode: String) -> LowResRMXError?
	{
		guard let coreWrapper
		else
		{
			assertionFailure()
			return nil
		}

		let error = coreWrapper.compileProgram(sourceCode: sourceCode)
		if error == nil
		{
			core_willRunProgram(
				&coreWrapper.core, Int(CFAbsoluteTimeGetCurrent() - AppController.shared.bootTime)
			)
			core_setDebug(&coreWrapper.core, isDebugEnabled)
		}
		return error
	}

	@objc func toggleDebugMode()
	{
		if isDebugEnabled
		{
			isDebugEnabled = false
			delegate?.didChangeDebugMode(enabled: false)
		}
		else
		{
			isDebugEnabled = true
			delegate?.didChangeDebugMode(enabled: true)
		}
	}

	@objc func captureProgramIcon()
	{
		guard let document
		else
		{
			assertionFailure()
			return
		}
		if let cgImage = nxView.layer.contents as! CGImage?
		{
			let cropRect = CGRect(
				x: 0, // (uiImage.size.width-216)/2,
				y: 0, // (uiImage.size.height-384)/2,
				width: 180, // 216,
				height: 180 // 384
			).integral
			let croppedImage = UIImage(
				cgImage: cgImage.cropping(
					to: cropRect
				)!
			)
			ProjectManager.shared.saveProjectIcon(programUrl: document.fileURL, image: croppedImage)
		}
	}

	@objc func shareScreenshot()
	{
		if let cgImage = nxView.layer.contents as! CGImage?
		{
			let uiImage = UIImage(cgImage: cgImage)

			// rescale
			let size = CGSize(
				width: CGFloat(216) * screenshotScaleFactor, height: CGFloat(384) * screenshotScaleFactor
			)

			let format = UIGraphicsImageRendererFormat()
			format.scale = 1.0
			format.opaque = true
			let renderer = UIGraphicsImageRenderer(size: size, format: format)
			let scaledImage = renderer.image
			{ context in
				context.cgContext.interpolationQuality = .none
				uiImage.draw(in: CGRect(origin: .zero, size: size))
			}

			// share
			let url = FileManager.default.temporaryDirectory.appendingPathComponent("screenshot.png")
			try? scaledImage.pngData()?.write(to: url)
			let activityVC = UIActivityViewController(
				activityItems: [url], applicationActivities: nil
			)
			activityVC.popoverPresentationController?.sourceView = menuButton
			activityVC.popoverPresentationController?.sourceRect = menuButton.bounds
			present(activityVC, animated: true, completion: nil)
		}
	}

	func recordVideo()
	{
		guard RPScreenRecorder.shared().isAvailable
		else
		{
			showAlert(withTitle: "Video Recording Currently Not Available", message: nil, block: nil)
			return
		}
		let recorder = RPScreenRecorder.shared()
		recorder.isMicrophoneEnabled = false
		recorder.startRecording
		{ error in
			if let error
			{
				DispatchQueue.main.async
				{
					self.showAlert(
						withTitle: "Cannot Record Video", message: error.localizedDescription, block: nil
					)
				}
			}
		}
	}

	func stopVideoRecording()
	{
		RPScreenRecorder.shared().stopRecording
		{ vc, error in
			if let vc
			{
				vc.previewControllerDelegate = self
				vc.modalPresentationStyle = .fullScreen
				self.present(vc, animated: true, completion: nil)
			}
			else
			{
				self.showAlert(
					withTitle: "Could Not Record Video", message: error?.localizedDescription,
					block: {
						self.presentingViewController?.dismiss(animated: true, completion: nil)
					}
				)
			}
		}
	}

	func saveProgramFromWeb()
	{
		guard
			let webSource,
			let sourceCode = coreWrapper?.sourceCode,
			let programData = sourceCode.data(using: .utf8)
		else
		{
			assertionFailure()
			return
		}
		BlockerView.show()
		ProjectManager.shared.addProject(
			originalName: webSource.name, programData: programData, imageData: imageData
		)
		{ _ in
			BlockerView.dismiss()
			self.exit()
		}
	}

//
	//  func countPlay(topicId: String) {
	//    var urlRequest = URLRequest(
	//      url: URL(string: "\(ShareViewController.baseURL)ajax/count_play.php")!)
	//    urlRequest.httpMethod = "POST"
	//    urlRequest.httpBody = "topic_id=\(topicId)".data(using: .utf8)
	//    let task = URLSession.shared.dataTask(with: urlRequest) { (data, response, error) in
	//      print("count play \(topicId):", error ?? "ok")
	//    }
	//    task.resume()
	//  }

	@objc func update(displaylink _: CADisplayLink)
	{
		guard let coreWrapper
		else
		{
			return
		}

		// pause when any alerts are visible
		if presentedViewController != nil
		{
			return
		}

		coreWrapper.input.width = Int32(screenWidth / screenScale)
		coreWrapper.input.height = Int32(screenHeight / screenScale)

		coreWrapper.input.left = Int32(left / screenScale)
		coreWrapper.input.top = Int32(top / screenScale)
		coreWrapper.input.right = Int32(right / screenScale)
		coreWrapper.input.bottom = Int32(bottom / screenScale)

		if keyboardWasShown
		{
			coreWrapper.input.keyboardChange = 1
			coreWrapper.input.keyboardHeight = Int32(keyboardNewHeight)
		}
		else if keyboardWasHidden
		{
			coreWrapper.input.keyboardChange = -1
			coreWrapper.input.keyboardHeight = Int32(keyboardNewHeight)
		}
		else
		{
			coreWrapper.input.keyboardChange = 0
		}
		keyboardWasShown = false
		keyboardWasHidden = false

		withUnsafeMutablePointer(to: &coreWrapper.input)
		{ inputPtr in
			core_update(&coreWrapper.core, inputPtr)
		}

		if core_shouldRender(&coreWrapper.core)
		{
			nxView.render()
		}
	}

	func exit()
	{
		if RPScreenRecorder.shared().isRecording
		{
			stopVideoRecording()
		}
		else
		{
			presentingViewController?.dismiss(animated: true, completion: nil)
		}
	}

	@objc func handleTap(sender: UITapGestureRecognizer)
	{
		if sender.state == .ended
		{
			becomeFirstResponder()
		}
	}

	@IBAction func pauseTapped(_: Any)
	{
		guard let coreWrapper
		else
		{
			return
		}

		coreWrapper.input.pause = true
	}

	override var canBecomeFirstResponder: Bool
	{
		controlsInfo.keyboardMode == KeyboardModeOn
	}

	override func pressesBegan(_ presses: Set<UIPress>, with event: UIPressesEvent?)
	{
		let handled = false
		for press in presses
		{
			if let key = press.key
			{
				// key strings
				switch key.charactersIgnoringModifiers
				{
				// case "p", "\r":
				// coreWrapper?.input.pause = true
				case UIKeyCommand.inputEscape:
					onExitTapped(UIKeyCommand())
				default:
					break
				}
			}
		}
		if !handled, event != nil
		{
			super.pressesBegan(presses, with: event)
		}
	}

	private func showError(_ error: Error)
	{
		errorToShow = error
		checkShowError()
	}

	private func checkShowError()
	{
		guard let error = errorToShow else { return }
		errorToShow = nil

		if let nxError = error as? LowResRMXError
		{
			// RMX Error
			let alert = UIAlertController(
				title: nxError.message, message: nxError.line, preferredStyle: .alert
			)
			alert.addAction(
				UIAlertAction(
					title: "OK", style: .cancel,
					handler: { _ in
						self.exit()
					}
				)
			)
			if delegate != nil
			{
				alert.addAction(
					UIAlertAction(
						title: "Go to Error", style: .default,
						handler: { _ in
							self.delegate?.didEndWithError(nxError)
							self.exit()
						}
					)
				)
			}
			present(alert, animated: true, completion: nil)
		}
		else
		{
			// System Error
			let alert = UIAlertController(
				title: error.localizedDescription, message: nil, preferredStyle: .alert
			)
			alert.addAction(
				UIAlertAction(
					title: "OK", style: .default,
					handler: { _ in
						self.exit()
					}
				)
			)
			present(alert, animated: true, completion: nil)
		}
	}

	@objc func keyboardWillShow(_ notification: NSNotification)
	{
		if let frameValue = notification.userInfo?[UIResponder.keyboardFrameEndUserInfoKey] as? NSValue
		{
			let frame = frameValue.cgRectValue
			keyboardTop = frame.origin.y
			view.setNeedsLayout()
			UIView.animate(
				withDuration: 0.3,
				animations: {
					self.view.layoutIfNeeded()
				}
			)
		}
	}

	@objc func keyboardDidShow(_ notification: NSNotification)
	{
		if let userInfo = notification.userInfo,
		   let keyboardRectangle = userInfo[UIResponder.keyboardFrameEndUserInfoKey] as? CGRect
		{
			if let coreWrapper
			{
				keyboardWasShown = true
				keyboardNewHeight = Int(Int32(ceil(keyboardRectangle.height / screenScale)))
			}
		}
	}

	@objc func keyboardDidHide(_: NSNotification)
	{
		if let coreWrapper
		{
			keyboardWasHidden = true
			keyboardNewHeight = 0
		}
	}

	@objc func keyboardWillHide(_: NSNotification)
	{
		keyboardTop = nil
		view.setNeedsLayout()
		UIView.animate(
			withDuration: 0.3,
			animations: {
				self.view.layoutIfNeeded()
			}
		)
	}

	@IBAction func onExitTapped(_ sender: Any?)
	{
		if sender is UIKeyCommand
		{
			delegate?.didEndWithKeyCommand()
		}

		let timeSinceStart = Date().timeIntervalSince(startDate)

		if timeSinceStart >= 60
		{
			let alert = UIAlertController(
				title: "Do you really want to exit this program?", message: nil, preferredStyle: .alert
			)
			alert.addAction(
				UIAlertAction(
					title: "Exit", style: .default,
					handler: { [unowned self] _ in
						exit()
					}
				)
			)
			alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
			present(alert, animated: true, completion: nil)
		}
		else
		{
			exit()
		}
	}

	@IBAction func settingsTapped(_ sender: Any)
	{
		let alert = UIAlertController(title: nil, message: nil, preferredStyle: .actionSheet)

		if document != nil
		{
			alert.addAction(
				UIAlertAction(
					title: "Capture Program Icon", style: .default,
					handler: { [unowned self] _ in
						captureProgramIcon()
					}
				)
			)
		}

		alert.addAction(
			UIAlertAction(
				title: "Share Screenshot", style: .default,
				handler: { [unowned self] _ in
					shareScreenshot()
				}
			)
		)

		if !RPScreenRecorder.shared().isRecording
		{
			alert.addAction(
				UIAlertAction(
					title: "Record Video", style: .default,
					handler: { [unowned self] _ in
						recordVideo()
					}
				)
			)
		}

		if webSource != nil
		{
			alert.addAction(
				UIAlertAction(
					title: "Save to My Programs", style: .default,
					handler: { [unowned self] _ in
						saveProgramFromWeb()
					}
				)
			)
		}

		if isDebugEnabled
		{
			alert.addAction(
				UIAlertAction(
					title: "Disable Debug Mode", style: .default,
					handler: { [unowned self] _ in
						isDebugEnabled = false
						delegate?.didChangeDebugMode(enabled: false)
					}
				)
			)
		}
		else
		{
			alert.addAction(
				UIAlertAction(
					title: "Enable Debug Mode", style: .default,
					handler: { [unowned self] _ in
						isDebugEnabled = true
						delegate?.didChangeDebugMode(enabled: true)
					}
				)
			)
		}

		alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))

		if let pop = alert.popoverPresentationController
		{
			let button = sender as! UIView
			pop.sourceView = button
			pop.sourceRect = button.bounds
		}
		present(alert, animated: true, completion: nil)
	}

	// MARK: - Core Wrapper Delegate

	func coreInterpreterDidFail(coreError: CoreError)
	{
		guard let coreWrapper
		else
		{
			assertionFailure()
			return
		}
		let interpreterError = LowResRMXError(error: coreError, sourceCode: coreWrapper.sourceCode!)
		showError(interpreterError)
	}

	func coreDiskDriveWillAccess(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool
	{
		if let delegate = toolDelegate
		{
			// tool editing current program
			let diskSourceCode = delegate.nxSourceCodeForVirtualDisk()
			let cDiskSourceCode = diskSourceCode.cString(using: .utf8)
			data_import(diskDataManager, cDiskSourceCode, true)
		}
		else
		{
			// tool editing shared disk file
			if let diskDocument
			{
				let cDiskSourceCode = (diskDocument.sourceCode ?? "").cString(using: .utf8)
				data_import(diskDataManager, cDiskSourceCode, true)
			}
			else
			{
				ProjectManager.shared.getDiskDocument(completion: { document, error in
					if let document
					{
						self.diskDocument = document
						let cDiskSourceCode = (document.sourceCode ?? "").cString(using: .utf8)
						data_import(diskDataManager, cDiskSourceCode, true)
						// TODO: hidding alert for now
						//                        self.showAlert(withTitle: "Using “Disk.rmx” as Virtual Disk", message: nil, block: {
						//                            core_diskLoaded(&self.coreWrapper!.core)
						//                        })
						core_diskLoaded(&self.coreWrapper!.core)
					}
					else
					{
						self.showAlert(
							withTitle: "Could not Access Virtual Disk", message: error?.localizedDescription,
							block: {
								self.exit()
							}
						)
					}
				})
				return false
			}
		}
		return true
	}

	func coreDiskDriveDidSave(diskDataManager: UnsafeMutablePointer<DataManager>?)
	{
		let output = data_export(diskDataManager)
		if let output, let diskSourceCode = String(cString: output, encoding: .utf8)
		{
			if let delegate = toolDelegate
			{
				// tool editing current program
				delegate.nxDidSaveVirtualDisk(sourceCode: diskSourceCode)
			}
			else
			{
				// tool editing shared disk file
				if let diskDocument
				{
					diskDocument.sourceCode = diskSourceCode
					diskDocument.updateChangeCount(.done)
				}
				else
				{
					print("No virtual disk available.")
				}
			}
		}
		free(output)
	}

	func coreDiskDriveIsFull(diskDataManager _: UnsafeMutablePointer<DataManager>?)
	{
		showAlert(withTitle: "Not Enough Space on Virtual Disk", message: nil) {}
	}

	func coreControlsDidChange(controlsInfo: ControlsInfo)
	{
		DispatchQueue.main.async
		{
			self.controlsInfo = controlsInfo
			if controlsInfo.keyboardMode == KeyboardModeOn
			{
				self.recognizer?.isEnabled = true
				self.becomeFirstResponder()
			}
			else
			{
				self.recognizer?.isEnabled = false
				self.resignFirstResponder()
			}
			if controlsInfo.isAudioEnabled
			{
				self.audioPlayer.start()
			}
			if controlsInfo.hapticMode == Error
			{
				UINotificationFeedbackGenerator().notificationOccurred(.error)
			}
			else if controlsInfo.hapticMode == Warning
			{
				UINotificationFeedbackGenerator().notificationOccurred(.warning)
			}
			else if controlsInfo.hapticMode == Success
			{
				UINotificationFeedbackGenerator().notificationOccurred(.success)
			}
			else if controlsInfo.hapticMode == Heavy
			{
				UIImpactFeedbackGenerator(style: .heavy).impactOccurred()
			}
			else if controlsInfo.hapticMode == Light
			{
				UIImpactFeedbackGenerator(style: .light).impactOccurred()
			}
			else if controlsInfo.hapticMode == Medium
			{
				UIImpactFeedbackGenerator(style: .medium).impactOccurred()
			}
			else if controlsInfo.hapticMode == Rigid
			{
				UIImpactFeedbackGenerator(style: .rigid).impactOccurred()
			}
			else if controlsInfo.hapticMode == Soft
			{
				UIImpactFeedbackGenerator(style: .soft).impactOccurred()
			}
			else if controlsInfo.hapticMode == Selection
			{
				UISelectionFeedbackGenerator().selectionChanged()
			}
			if controlsInfo.isCompatMode
			{
				self.isCompatMode = true
				self.view.setNeedsLayout()
			}
		}
	}

	func persistentRamWillAccess(destination: UnsafeMutablePointer<UInt8>?, size: Int32)
	{
		guard let document else { return }
		guard let destination
		else
		{
			assertionFailure()
			return
		}

		if let data = ProjectManager.shared.loadPersistentRam(programUrl: document.fileURL)
		{
			data.copyBytes(to: destination, count: min(data.count, Int(size)))
		}
	}

	func persistentRamDidChange(_ data: Data)
	{
		guard let document else { return }
		ProjectManager.shared.savePersistentRam(programUrl: document.fileURL, data: data)
	}

	// MARK: - UIKeyInput

	var autocorrectionType: UITextAutocorrectionType = .no
	var spellCheckingType: UITextSpellCheckingType = .no
	var keyboardAppearance: UIKeyboardAppearance = .dark

	var hasText: Bool
	{
		true
	}

	func insertText(_ text: String)
	{
		guard let coreWrapper
		else
		{
			return
		}

		if text == "\n"
		{
			coreWrapper.input.key = CoreInputKeyReturn
		}
		else if let key = text.unicodeScalars.first?.value
		{
			if key < 127
			{
				coreWrapper.input.key = Int8(key)
			}
		}
	}

	func deleteBackward()
	{
		guard let coreWrapper
		else
		{
			return
		}

		coreWrapper.input.key = CoreInputKeyBackspace
	}

	/// this is from UITextInput, needed because of crash on iPhone 6 keyboard (left/right arrows)
	var selectedTextRange: UITextRange?
	{
		nil
	}

	var keyboardType: UIKeyboardType = .asciiCapable

	// MARK: - RPPreviewViewControllerDelegate

	func previewControllerDidFinish(_: RPPreviewViewController)
	{
		DispatchQueue.main.async
		{
			self.dismiss(animated: true)
			{
				self.presentingViewController?.dismiss(animated: true, completion: nil)
			}
		}
	}
}
