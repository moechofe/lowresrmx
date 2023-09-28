//
//  SplashViewController.swift
//  LowResRMX
//
//  Created by Timo Kloss on 24/11/17.
//  Copyright © 2017-2019 Inutilis Software. All rights reserved.
//

import UIKit

class SplashViewController: UIViewController {

    @IBOutlet weak var splashImageView: UIImageView!
    @IBOutlet weak var nxView: LowResRMXView!
    
    private var coreWrapper = CoreWrapper()
    private var displayLink: CADisplayLink!
    private var audioPlayer: LowResRMXAudioPlayer!
    
    private var isSetupDone = false
    private var isAnimationDone = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        nxView.coreWrapper = coreWrapper
        audioPlayer = LowResRMXAudioPlayer(coreWrapper: coreWrapper)
        displayLink = createDisplayLink()

        loadIntro()

        ProjectManager.shared.setup {
            self.isSetupDone = true
            self.checkStart()
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        audioPlayer.start()
        displayLink.add(to: .current, forMode: .default)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        displayLink.invalidate()
        audioPlayer.stop()
    }
    
    private func loadIntro() {
        let url = Bundle.main.url(forResource: "Boot Intro", withExtension: "nx")!
        let sourceCode = try! String(contentsOf: url)
        let error = coreWrapper.compileProgram(sourceCode: sourceCode)
        guard error == nil else { fatalError() }

        core_willRunProgram(&coreWrapper.core, Int(CFAbsoluteTimeGetCurrent() - AppController.shared.bootTime))
        machine_poke(&coreWrapper.core, 0xA000, 1)
    }

    private func createDisplayLink() -> CADisplayLink {
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        if #available(iOS 10.0, *) {
            displayLink.preferredFramesPerSecond = 60
        } else {
            displayLink.frameInterval = 1
        }
        return displayLink
    }

    @objc private func update(displaylink: CADisplayLink) {
        core_update(&coreWrapper.core, &coreWrapper.input)
        nxView.render()

        if machine_peek(&coreWrapper.core, 0xA000) == 2 {
            machine_poke(&coreWrapper.core, 0xA000, 3)
            isAnimationDone = true
            checkStart()
        }
    }
    
    private func checkStart() {
        if isSetupDone && isAnimationDone {
            showApp()
        }
    }
    
    private func showApp() {
        if let window = view.window {
            if let vc = storyboard?.instantiateViewController(withIdentifier: "AppStart") {
                window.rootViewController = vc
                UIView.transition(with: window, duration: 0.3, options: .transitionCrossDissolve, animations: {
                    window.rootViewController = vc
                }, completion: nil)
            }
        }
    }
    
}
