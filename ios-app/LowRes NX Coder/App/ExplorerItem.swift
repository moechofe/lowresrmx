//
//  ExplorerItem.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 28/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class ExplorerItem: NSObject {
    
    var fileUrl: URL
    var isDefault = false
    var metadataItem: NSMetadataItem?
    
    var name: String {
        return fileUrl.deletingPathExtension().lastPathComponent
    }
    
    var imageUrl: URL {
        return fileUrl.deletingPathExtension().appendingPathExtension("png")
    }
    
    var hasImage: Bool {
        return FileManager.default.fileExists(atPath: imageUrl.path)
    }
    
    var image: UIImage? {
        do {
            let imageData = try Data(contentsOf: imageUrl)
            if let image = UIImage(data: imageData) {
                return image
            }
        } catch {
        }
        return nil
    }
    
    var createdAt: Date {
        if let metadataItem = metadataItem {
            if let date = metadataItem.value(forAttribute: NSMetadataItemFSCreationDateKey) as! Date? {
                return date
            }
        }
        if let attrs = try? FileManager.default.attributesOfItem(atPath: fileUrl.path) {
            return attrs[FileAttributeKey.creationDate] as! Date
        }
        return Date.distantFuture
    }
    
    init(fileUrl: URL) {
        self.fileUrl = fileUrl
        super.init()
    }
    
    func updateFromMetadata() {
        fileUrl = metadataItem!.value(forAttribute: NSMetadataItemURLKey) as! URL
    }
    
}
