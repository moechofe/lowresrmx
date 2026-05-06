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

class ExplorerItem: NSObject
{
	var fileUrl: URL
	var isDefault = false
	var metadataItem: NSMetadataItem?

	var name: String
	{
		fileUrl.deletingPathExtension().lastPathComponent
	}

	var imageUrl: URL
	{
		fileUrl.deletingPathExtension().appendingPathExtension("png")
	}

	var hasImage: Bool
	{
		FileManager.default.fileExists(atPath: imageUrl.path)
	}

	var image: UIImage?
	{
		do
		{
			let imageData = try Data(contentsOf: imageUrl)
			if let image = UIImage(data: imageData)
			{
				return image
			}
		}
		catch
		{}
		return nil
	}

	var createdAt: Date
	{
		if let metadataItem
		{
			if let date = metadataItem.value(forAttribute: NSMetadataItemFSCreationDateKey) as! Date?
			{
				return date
			}
		}
		if let attrs = try? FileManager.default.attributesOfItem(atPath: fileUrl.path)
		{
			return attrs[FileAttributeKey.creationDate] as! Date
		}
		return Date.distantFuture
	}

	init(fileUrl: URL)
	{
		self.fileUrl = fileUrl
		super.init()
	}

	func updateFromMetadata()
	{
		fileUrl = metadataItem!.value(forAttribute: NSMetadataItemURLKey) as! URL
	}
}
