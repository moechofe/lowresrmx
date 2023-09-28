//
//  ExplorerProgramCell.swift
//  LowRes Coder NX
//
//  Created by Timo Kloss on 24/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

protocol ExplorerItemCellDelegate: class {
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectRename item: ExplorerItem)
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectDelete item: ExplorerItem)
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectDuplicate item: ExplorerItem)
}

class ExplorerItemCell: UICollectionViewCell {
    
    @IBOutlet weak var nameLabel: UILabel!
    @IBOutlet weak var shadowView: UIView!
    @IBOutlet weak var previewImageView: UIImageView!
    
    weak var delegate: ExplorerItemCellDelegate?
    
    var item: ExplorerItem? {
        didSet {
            if let item = item {
                nameLabel.text = item.name
                previewImageView.image = item.image
            }
        }
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        let imageLayer = previewImageView.layer
        imageLayer.cornerRadius = 2
        imageLayer.masksToBounds = true
//        previewImageView.backgroundColor = AppStyle.mediumTintColor()
        
        shadowView.layer.cornerRadius = 2
        shadowView.layer.shadowOffset = CGSize(width: 0, height: 1)
        shadowView.layer.shadowOpacity = 1.0
        shadowView.layer.shadowRadius = 2.0
        
//        nameLabel.shadowColor = UIColor.black
        //nameLabel.shadowOffset = CGSize(width: 0, height: 2)
        
//        backgroundColor = AppStyle.darkGrayColor()
    }
    
    @objc func renameItem(_ sender: Any?) {
        if let delegate = delegate {
            delegate.explorerItemCell(self, didSelectRename: item!)
        }
    }
    
    @objc func deleteItem(_ sender: Any?) {
        if let delegate = delegate {
            delegate.explorerItemCell(self, didSelectDelete: item!)
        }
    }
    
    @objc func duplicateItem(_ sender: Any?) {
        if let delegate = delegate {
            delegate.explorerItemCell(self, didSelectDuplicate: item!)
        }
    }
    
}
