//
//  ProjectDocument.swift
//  LowResRMX
//
//  Created by Timo Kloss on 28/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

protocol ProjectDocumentDelegate: AnyObject {
    func projectDocumentContentDidUpdate(_ projectDocument: ProjectDocument)
}

class ProjectDocument: UIDocument {
    
    var sourceCode: String?
    weak var delegate: ProjectDocumentDelegate?
    
    override var localizedName: String {
        return fileURL.deletingPathExtension().lastPathComponent
    }
    
    override func contents(forType typeName: String) throws -> Any {
        if sourceCode == nil {
            sourceCode = ""
        }
        return sourceCode!.data(using: .utf8)!
    }
    
    override func load(fromContents contents: Any, ofType typeName: String?) throws {
        sourceCode = String(data: contents as! Data, encoding: .utf8)
        delegate?.projectDocumentContentDidUpdate(self)
    }
        
}
