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

class ExplorerViewController: UIViewController, UICollectionViewDelegateFlowLayout, UICollectionViewDataSource
{
	@IBOutlet var collectionView: UICollectionView!
	@IBOutlet var activityView: UIActivityIndicatorView!

	var items: [ExplorerItem]?
	var addedItem: ExplorerItem?

	private var metadataQuery: NSMetadataQuery?
	private var didAddProgramObserver: Any?
	private var queryDidFinishGatheringObserver: Any?
	private var queryDidUpdateObserver: Any?
	private var isVisible = false
	private var isContextMenuVisible = false
	private var runningFileOperations = 0
	private var needsFileListUpdate = false
	private var areQueryUpdatesEnabled = true

	override func viewDidLoad()
	{
		super.viewDidLoad()

		let addProjectItem = UIBarButtonItem(barButtonSystemItem: .add, target: self, action: #selector(onAddProjectTapped))
		let actionItem = UIBarButtonItem(image: UIImage(named: "gear"), style: .plain, target: self, action: #selector(onActionTapped))

		navigationItem.leftBarButtonItem = actionItem
		navigationItem.rightBarButtonItems = [addProjectItem]

		collectionView.dataSource = self
		collectionView.delegate = self

		collectionView.indicatorStyle = .white

		// disable iOS 26 changing navigation color because of the content
		if #available(iOS 26.0, *)
		{
			collectionView.topEdgeEffect.style = .hard
		}

		if ProjectManager.shared.isCloudEnabled
		{
			setupCloud()
		}
		else
		{
			loadLocalItems()
		}

		didAddProgramObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.ProjectManagerDidAddProgram, object: nil, queue: nil)
		{ [weak self] notification in
			guard let self, let item = notification.userInfo?["item"] as? ExplorerItem
			else
			{
				return
			}
			addedItem = item
			showAddedItem()
		}
	}

	deinit
	{
		removeCloudObservers()
		if didAddProgramObserver != nil
		{
			NotificationCenter.default.removeObserver(didAddProgramObserver!)
			didAddProgramObserver = nil
		}
	}

	override func traitCollectionDidChange(_ previousTraitCollection: UITraitCollection?)
	{
		super.traitCollectionDidChange(previousTraitCollection)
		if traitCollection.horizontalSizeClass == .regular
		{
			navigationItem.backBarButtonItem = nil
		}
		else
		{
			navigationItem.backBarButtonItem = UIBarButtonItem(title: "", style: .plain, target: nil, action: nil)
		}
	}

	override func viewDidAppear(_ animated: Bool)
	{
		super.viewDidAppear(animated)
		isVisible = true
		// no context menu can outlive the view controller going away and coming back
		isContextMenuVisible = false
		showAddedItem()
		if let indexPaths = collectionView.indexPathsForSelectedItems, !indexPaths.isEmpty
		{
			// update cell of last used program
			collectionView.performBatchUpdates({
				self.collectionView.reloadItems(at: indexPaths)
			}, completion: { _ in
				self.syncQueryUpdates()
			})
		}
		else
		{
			syncQueryUpdates()
		}
	}

	override func viewWillDisappear(_ animated: Bool)
	{
		super.viewWillDisappear(animated)
		isVisible = false
		syncQueryUpdates()
	}

	override func viewWillLayoutSubviews()
	{
		super.viewWillLayoutSubviews()
		collectionView.collectionViewLayout.invalidateLayout()
	}

	func loadLocalItems()
	{
		do
		{
			let urls = try FileManager.default.contentsOfDirectory(at: ProjectManager.shared.localDocumentsUrl, includingPropertiesForKeys: nil, options: [])
			var items = [ExplorerItem]()
			for url in urls
			{
				if url.pathExtension == "rmx"
				{
					items.append(ExplorerItem(fileUrl: url))
				}
			}
			items.sort(by: { item1, item2 -> Bool in
				item1.createdAt < item2.createdAt
			})
			self.items = items
		}
		catch
		{
			// error
			items = nil
		}
		collectionView.reloadData()
		updateFooter()
	}

	private func setupCloud()
	{
		items = nil
		collectionView.reloadData()
		updateFooter()

		activityView.startAnimating()

		let query = NSMetadataQuery()
		metadataQuery = query
		query.searchScopes = [NSMetadataQueryUbiquitousDocumentsScope]
		query.predicate = NSPredicate(format: "%K LIKE '*.rmx'", NSMetadataItemFSNameKey)

		queryDidFinishGatheringObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSMetadataQueryDidFinishGathering, object: query, queue: nil, using: { [weak self] _ in
			self?.activityView.stopAnimating()
			self?.updateCloudFileList()
		})

		queryDidUpdateObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSMetadataQueryDidUpdate, object: query, queue: nil, using: { [weak self] _ in
			self?.updateCloudFileList()
		})
		query.start()
	}

	private func removeCloudObservers()
	{
		metadataQuery?.stop()
		if queryDidFinishGatheringObserver != nil
		{
			NotificationCenter.default.removeObserver(queryDidFinishGatheringObserver!)
			queryDidFinishGatheringObserver = nil
		}
		if queryDidUpdateObserver != nil
		{
			NotificationCenter.default.removeObserver(queryDidUpdateObserver!)
			queryDidUpdateObserver = nil
		}
	}

	// MARK: - Refresh gating

	private var isCollectionViewLocked: Bool
	{
		isContextMenuVisible || runningFileOperations > 0
	}

	private func syncQueryUpdates()
	{
		let shouldEnable = isVisible && !isCollectionViewLocked
		if shouldEnable != areQueryUpdatesEnabled
		{
			areQueryUpdatesEnabled = shouldEnable
			if shouldEnable
			{
				metadataQuery?.enableUpdates()
			}
			else
			{
				metadataQuery?.disableUpdates()
			}
		}
		applyDeferredUpdates()
	}

	private func beginFileOperation()
	{
		runningFileOperations += 1
		syncQueryUpdates()
	}

	private func endFileOperation()
	{
		runningFileOperations -= 1
		syncQueryUpdates()
	}

	private func applyDeferredUpdates()
	{
		guard !isCollectionViewLocked
		else
		{
			return
		}
		if needsFileListUpdate
		{
			updateCloudFileList()
		}
		showAddedItem()
	}

	private func refreshVisibleCells()
	{
		guard let items
		else
		{
			return
		}
		for indexPath in collectionView.indexPathsForVisibleItems where indexPath.item < items.count
		{
			(collectionView.cellForItem(at: indexPath) as? ExplorerItemCell)?.item = items[indexPath.item]
		}
	}

	private func updateCloudFileList()
	{
		guard let query = metadataQuery
		else
		{
			return
		}

		guard !isCollectionViewLocked
		else
		{
			needsFileListUpdate = true
			return
		}
		needsFileListUpdate = false

		var itemsByUrl = [URL: ExplorerItem]()
		for item in items ?? []
		{
			itemsByUrl[item.fileUrl] = item
		}

		var newItems = [ExplorerItem]()
		for metadataItem in query.results as! [NSMetadataItem]
		{
			guard let url = metadataItem.value(forAttribute: NSMetadataItemURLKey) as? URL
			else
			{
				continue
			}
			let item = itemsByUrl[url] ?? ExplorerItem(fileUrl: url)
			item.metadataItem = metadataItem
			newItems.append(item)
		}
		newItems.sort(by: { item1, item2 -> Bool in
			item1.createdAt < item2.createdAt
		})

		// the query has picked up a program this app just created
		if let addedItem, newItems.contains(where: { $0.fileUrl == addedItem.fileUrl })
		{
			self.addedItem = nil
		}

		if let items, items.elementsEqual(newItems, by: { $0 === $1 })
		{
			// same programs in the same order: refresh contents without dequeueing
			refreshVisibleCells()
			return
		}

		items = newItems
		collectionView.reloadData()
		updateFooter()
	}

	func showAddedItem()
	{
		guard isVisible, !isCollectionViewLocked, let addedItem, items != nil
		else
		{
			return
		}
		items!.append(addedItem)
		let indexPath = IndexPath(item: items!.count - 1, section: 0)
		collectionView.insertItems(at: [indexPath])
		collectionView.scrollToItem(at: indexPath, at: .bottom, animated: true)
		updateFooter()
		self.addedItem = nil
	}

	private func updateFooter()
	{
		if let footerView = collectionView.supplementaryView(forElementKind: UICollectionView.elementKindSectionFooter, at: IndexPath(item: 0, section: 0))
		{
			footerView.isHidden = items?.isEmpty ?? true
		}
	}

	@objc func onAddProjectTapped(_: Any)
	{
		ProjectManager.shared.addProject(originalName: "Unnamed Program", programData: nil, imageData: nil)
		{ error in
			if let error
			{
				self.showAlert(withTitle: "Could not Add New Project", message: error.localizedDescription, block: nil)
			}
		}
	}

	// @objc func onCommunityTapped(_ sender: Any) {
	//     tabBarController?.selectedIndex = TabIndex.community.rawValue
	// }

	@objc func onActionTapped(_ sender: UIBarButtonItem)
	{
		let alert = UIAlertController(title: "Options", message: nil, preferredStyle: .actionSheet)

		let fontSizeAction = UIAlertAction(title: "Editor Settings", style: .default, handler: { [weak self] _ in
			self?.showFontSizePicker()
		})
		alert.addAction(fontSizeAction)

		let communityAction = UIAlertAction(title: "Community Website", style: .default, handler: { _ in
			if let url = URL(string: AppDelegate.baseURL)
			{
				UIApplication.shared.open(url, options: [:], completionHandler: nil)
			}
		})
		alert.addAction(communityAction)

		let addAction = UIAlertAction(title: "Reinstall Default Programs", style: .default, handler: { [weak self] _ in
			self?.onReinstallTapped()
		})
		alert.addAction(addAction)

		let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
		alert.addAction(cancelAction)

		alert.popoverPresentationController?.barButtonItem = sender
		present(alert, animated: true, completion: nil)
	}

	func onReinstallTapped()
	{
		let alert = UIAlertController(title: "Reinstall Default Programs?", message: "This may overwrite changes you made to them.", preferredStyle: .alert)
		alert.addAction(UIAlertAction(title: "Reinstall", style: .destructive, handler: { [weak self] _ in
			self?.reinstall()
		}))
		alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
		present(alert, animated: true, completion: nil)
	}

	func reinstall()
	{
		BlockerView.show()

		ProjectManager.shared.reinstallBundlePrograms
		{
			BlockerView.dismiss()

			if !ProjectManager.shared.isCloudEnabled
			{
				self.loadLocalItems()
			}
		}
	}

//
	//    func logout() {
	//        let urlString = AppDelegate.baseURL.appendingPathComponent("logout.php").absoluteString + "?webmode=app";
	//        let vc = WebViewController()
	//        vc.url = URL(string: urlString)!
	//        vc.title = "Log Out"
	//        let nc = UINavigationController(rootViewController: vc)
	//        present(nc, animated: true, completion: nil)
//
	//        AppController.shared.didLogOut()
	//    }

	func showFontSizePicker()
	{
		let fontSizePicker = EditorSettingViewController()
		let navigationController = UINavigationController(rootViewController: fontSizePicker)
		present(navigationController, animated: true, completion: nil)
	}

	func showEditor(fileUrl: URL)
	{
		let document = ProjectDocument(fileURL: fileUrl)
		let vc = storyboard!.instantiateViewController(withIdentifier: "EditorView") as! EditorViewController
		vc.document = document
		navigationController?.pushViewController(vc, animated: true)
	}

	func shareItem(_ item: ExplorerItem, from cell: UIView? = nil)
	{
		let activityItems: [Any] = [item.fileUrl]
		let shareActivity = ShareActivity()
		let activityVC = UIActivityViewController(activityItems: activityItems, applicationActivities: [shareActivity])
		activityVC.popoverPresentationController?.sourceView = cell ?? view
		if let cell
		{
			activityVC.popoverPresentationController?.sourceRect = cell.bounds
		}
		else
		{
			activityVC.popoverPresentationController?.sourceRect = CGRect(x: view.bounds.midX, y: view.bounds.midY, width: 0, height: 0)
			activityVC.popoverPresentationController?.permittedArrowDirections = []
		}
		present(activityVC, animated: true, completion: nil)
	}

	func deleteItem(_ item: ExplorerItem)
	{
		beginFileOperation()

		ProjectManager.shared.deleteProject(item: item)
		{ error in
			if let error
			{
				self.endFileOperation()
				self.showAlert(withTitle: "Could not Delete Program", message: error.localizedDescription, block: nil)
			}
			else
			{
				self.collectionView.performBatchUpdates({
					if let index = self.items?.firstIndex(of: item)
					{
						self.items?.remove(at: index)
						self.collectionView.deleteItems(at: [IndexPath(item: index, section: 0)])
					}
				}, completion: { _ in
					self.updateFooter()
					self.endFileOperation()
				})
			}
		}
	}

	func renameItem(_ item: ExplorerItem, newName: String)
	{
		beginFileOperation()

		ProjectManager.shared.renameProject(item: item, newName: newName)
		{ error in
			if let error
			{
				self.endFileOperation()
				self.showAlert(withTitle: "Could not Rename Program", message: error.localizedDescription, block: nil)
			}
			else
			{
				self.collectionView.performBatchUpdates({
					if let index = self.items?.firstIndex(of: item)
					{
						self.collectionView.reloadItems(at: [IndexPath(item: index, section: 0)])
					}
				}, completion: { _ in
					self.endFileOperation()
				})
			}
		}
	}

	func duplicateItem(_ item: ExplorerItem)
	{
		beginFileOperation()

		ProjectManager.shared.duplicateProject(item: item)
		{ error in
			self.endFileOperation()

			if let error
			{
				self.showAlert(withTitle: "Could not Duplicate Program", message: error.localizedDescription, block: nil)
			}
		}
	}

	// MARK: - UICollectionViewDataSource

	func collectionView(_: UICollectionView, numberOfItemsInSection _: Int) -> Int
	{
		items?.count ?? 0
	}

	func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell
	{
		let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProjectCell", for: indexPath) as! ExplorerItemCell
		cell.item = items?[indexPath.item]
		return cell
	}

	func collectionView(_ collectionView: UICollectionView, viewForSupplementaryElementOfKind kind: String, at indexPath: IndexPath) -> UICollectionReusableView
	{
		let footerView = collectionView.dequeueReusableSupplementaryView(ofKind: kind, withReuseIdentifier: "Footer", for: indexPath)
		footerView.isHidden = items?.isEmpty ?? true
		return footerView
	}

	// MARK: - UICollectionViewDelegateFlowLayout

	func collectionView(_: UICollectionView, didSelectItemAt indexPath: IndexPath)
	{
		let item = items![indexPath.item]
		showEditor(fileUrl: item.fileUrl)
	}

	func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt _: IndexPath) -> CGSize
	{
		var cellSize: CGSize
		cellSize = CGSize(width: 110, height: 150)
		let layout = collectionViewLayout as! UICollectionViewFlowLayout
		let width = collectionView.bounds.size.width - layout.sectionInset.left - layout.sectionInset.right
		let numItemsPerLine = floor(width / cellSize.width)
		return CGSize(width: floor(width / numItemsPerLine), height: cellSize.height)
	}

	func collectionView(_ collectionView: UICollectionView, contextMenuConfigurationForItemAt indexPath: IndexPath, point _: CGPoint) -> UIContextMenuConfiguration?
	{
		// the cell is the source of truth for what the user actually pressed
		guard let cell = collectionView.cellForItem(at: indexPath) as? ExplorerItemCell, let item = cell.item else { return nil }

		return UIContextMenuConfiguration(identifier: item.fileUrl as NSURL, previewProvider: nil)
		{ _ -> UIMenu? in
			let renameAction = UIAction(title: "Rename...", image: UIImage(systemName: "pencil"))
			{ [weak self] _ in
				self?.showRenameAlert(for: item)
			}
			let duplicateAction = UIAction(title: "Duplicate", image: UIImage(systemName: "plus.square.on.square"))
			{ [weak self] _ in
				self?.duplicateItem(item)
			}
			let deleteAction = UIAction(title: "Delete...", image: UIImage(systemName: "minus.circle"), attributes: .destructive)
			{ [weak self] _ in
				self?.showDeleteConfirmation(for: item)
			}
			let shareAction = UIAction(title: "Share...", image: UIImage(systemName: "square.and.arrow.up"))
			{ [weak self] _ in
				guard let self else { return }
				shareItem(item, from: self.cell(for: item))
			}
			return UIMenu(title: "", children: [shareAction, renameAction, duplicateAction, deleteAction])
		}
	}

	func collectionView(_: UICollectionView, willDisplayContextMenu _: UIContextMenuConfiguration, animator _: UIContextMenuInteractionAnimating?)
	{
		isContextMenuVisible = true
		syncQueryUpdates()
	}

	func collectionView(_: UICollectionView, willEndContextMenuInteraction _: UIContextMenuConfiguration, animator: UIContextMenuInteractionAnimating?)
	{
		// The lifted cell is handed back to the collection view only when the
		// dismissal animation ends, so stay locked until then.
		if let animator
		{
			animator.addCompletion
			{
				self.isContextMenuVisible = false
				self.syncQueryUpdates()
			}
		}
		else
		{
			isContextMenuVisible = false
			syncQueryUpdates()
		}
	}

	// MARK: - Item actions

	private func cell(for item: ExplorerItem) -> UICollectionViewCell?
	{
		guard let index = items?.firstIndex(of: item)
		else
		{
			return nil
		}
		return collectionView.cellForItem(at: IndexPath(item: index, section: 0))
	}

	func showRenameAlert(for item: ExplorerItem)
	{
		let alert = UIAlertController(title: "Rename “\(item.name)”", message: nil, preferredStyle: .alert)
		alert.addTextField
		{ textField in
			textField.clearButtonMode = .always
			textField.autocapitalizationType = .none
			textField.autocorrectionType = .no
			textField.spellCheckingType = .no
			textField.text = item.name
		}
		alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
		alert.addAction(UIAlertAction(title: "Accept", style: .default, handler: { _ in
			let textField = alert.textFields!.first!
			if let name = textField.text?.trimmingCharacters(in: .whitespaces)
			{
				self.renameItem(item, newName: name)
			}
		}))
		present(alert, animated: true, completion: nil)
	}

	func showDeleteConfirmation(for item: ExplorerItem)
	{
		var message: String?
		if ProjectManager.shared.isCloudEnabled
		{
			message = "This file will be deleted from iCloud Drive and all your iCloud devices."
		}
		let alert = UIAlertController(title: "Do you really want to delete “\(item.name)”?", message: message, preferredStyle: .actionSheet)
		alert.addAction(UIAlertAction(title: "Delete", style: .destructive, handler: { [unowned self] _ in
			deleteItem(item)
		}))
		alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
		if let pop = alert.popoverPresentationController
		{
			// must be set before presenting: on iPad the action sheet is a popover
			let anchorView: UIView = cell(for: item) ?? view
			pop.sourceView = anchorView
			pop.sourceRect = anchorView.bounds
			pop.permittedArrowDirections = [.down, .up]
		}
		present(alert, animated: true, completion: nil)
	}
}
