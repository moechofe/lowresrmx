//
//  CoreStatsWrapper.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 18/04/2020.
//  Copyright © 2020 Inutilis Software. All rights reserved.
//

import UIKit

class CoreStatsWrapper: NSObject {
    
    var stats = Stats()
    
    override init() {
        super.init()
        stats_init(&stats)
    }
    
    deinit {
        stats_deinit(&stats)
    }
    
    func update(sourceCode: String) -> LowResNXError? {
        let cString = sourceCode.cString(using: .utf8)
        let error = stats_update(&stats, cString)
        if error.code != ErrorNone {
            return LowResNXError(error: error, sourceCode: sourceCode)
        }
        return nil
    }
    
}
