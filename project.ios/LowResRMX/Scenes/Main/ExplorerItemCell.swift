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

protocol ExplorerItemCellDelegate: AnyObject
{
	func explorerItemCell(_ cell: ExplorerItemCell, didSelectRename item: ExplorerItem)
	func explorerItemCell(_ cell: ExplorerItemCell, didSelectDelete item: ExplorerItem)
	func explorerItemCell(_ cell: ExplorerItemCell, didSelectDuplicate item: ExplorerItem)
	func explorerItemCell(_ cell: ExplorerItemCell, didSelectShare item: ExplorerItem)
}

class ExplorerItemCell: UICollectionViewCell
{
	@IBOutlet var nameLabel: UILabel!
	@IBOutlet var shadowView: UIView!
	@IBOutlet var previewImageView: UIImageView!

	weak var delegate: ExplorerItemCellDelegate?

	var item: ExplorerItem?
	{
		didSet
		{
			if let item
			{
				nameLabel.text = item.name
				previewImageView.image = item.image
			}
		}
	}

	override func awakeFromNib()
	{
		super.awakeFromNib()
		let imageLayer = previewImageView.layer
		imageLayer.cornerRadius = 2
		imageLayer.masksToBounds = true
		//        previewImageView.backgroundColor = AppStyle.mediumTintColor()

		shadowView.layer.cornerRadius = 0
		shadowView.layer.shadowOffset = CGSize(width: 0, height: 1)
		shadowView.layer.shadowOpacity = 1.0
		shadowView.layer.shadowRadius = 2.0

		//        nameLabel.shadowColor = UIColor.black
		// nameLabel.shadowOffset = CGSize(width: 0, height: 2)

		//        backgroundColor = AppStyle.darkGrayColor()
	}

	@objc func shareItem(_: Any?)
	{
		if let delegate
		{
			delegate.explorerItemCell(self, didSelectShare: item!)
		}
	}

	@objc func renameItem(_: Any?)
	{
		if let delegate
		{
			delegate.explorerItemCell(self, didSelectRename: item!)
		}
	}

	@objc func deleteItem(_: Any?)
	{
		if let delegate
		{
			delegate.explorerItemCell(self, didSelectDelete: item!)
		}
	}

	@objc func duplicateItem(_: Any?)
	{
		if let delegate
		{
			delegate.explorerItemCell(self, didSelectDuplicate: item!)
		}
	}
}
