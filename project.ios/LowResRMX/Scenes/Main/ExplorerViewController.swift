//
//  ExplorerViewController.swift
//  LowRes CoderRMX
//
//  Created by Timo Kloss on 24/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class ExplorerViewController: UIViewController, UICollectionViewDelegateFlowLayout, UICollectionViewDataSource, ExplorerItemCellDelegate, NSMetadataQueryDelegate {
    
    @IBOutlet weak var collectionView: UICollectionView!
    @IBOutlet weak var activityView: UIActivityIndicatorView!
    
    var items: [ExplorerItem]?
    var addedItem: ExplorerItem?
    
    private var metadataQuery: NSMetadataQuery?
    private var didAddProgramObserver: Any?
    private var queryDidFinishGatheringObserver: Any?
    private var queryDidUpdateObserver: Any?
    private var willShowMenuObserver: Any?
    private var didHideMenuObserver: Any?
    private var isVisible: Bool = false
    private var unassignedItems = [URL: ExplorerItem]()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
//        view.backgroundColor = AppStyle.darkGrayColor()
        
        let addProjectItem = UIBarButtonItem(barButtonSystemItem: .add, target: self, action: #selector(onAddProjectTapped))
        let actionItem = UIBarButtonItem(image: UIImage(named: "gear"), style: .plain, target: self, action: #selector(onActionTapped))
        
        navigationItem.leftBarButtonItem = actionItem
        navigationItem.rightBarButtonItem = addProjectItem
        
        collectionView.dataSource = self
        collectionView.delegate = self
        
        collectionView.indicatorStyle = .white
        
        if ProjectManager.shared.isCloudEnabled {
            setupCloud()
        } else {
            loadLocalItems()
        }
        
        didAddProgramObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.ProjectManagerDidAddProgram, object: nil, queue: nil) { (notification) in
            let item: ExplorerItem! = notification.userInfo!["item"] as! ExplorerItem?
            self.unassignedItems[item.fileUrl] = item
            self.addedItem = item
            if self.isVisible {
                self.showAddedItem()
            }
        }
    }
    
    deinit {
        removeCloudObservers()
        if didAddProgramObserver != nil {
            NotificationCenter.default.removeObserver(didAddProgramObserver!)
            didAddProgramObserver = nil
        }
    }
    
    override func traitCollectionDidChange(_ previousTraitCollection: UITraitCollection?) {
        super.traitCollectionDidChange(previousTraitCollection)
        if traitCollection.horizontalSizeClass == .regular {
            navigationItem.backBarButtonItem = nil
        } else {
            navigationItem.backBarButtonItem = UIBarButtonItem(title: "", style: .plain, target: nil, action: nil)
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        isVisible = true
        showAddedItem()
        if let indexPaths = collectionView.indexPathsForSelectedItems, !indexPaths.isEmpty {
            // update cell of last used program
            collectionView.performBatchUpdates({
                self.collectionView.reloadItems(at: indexPaths)
            }, completion: { (finished) in
                self.metadataQuery?.enableUpdates()
            })
        } else {
            metadataQuery?.enableUpdates()
        }
        
        willShowMenuObserver = NotificationCenter.default.addObserver(
            forName: UIMenuController.willShowMenuNotification,
            object: nil,
            queue: nil
        ) { [weak self] (notification) in
            self?.metadataQuery?.disableUpdates()
        }
        didHideMenuObserver = NotificationCenter.default.addObserver(
            forName: UIMenuController.didHideMenuNotification,
            object: nil,
            queue: nil
        ) { [weak self] (notification) in
            self?.metadataQuery?.enableUpdates()
        }
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        isVisible = false
        metadataQuery?.disableUpdates()
        
        if willShowMenuObserver != nil {
            NotificationCenter.default.removeObserver(willShowMenuObserver!)
            willShowMenuObserver = nil
        }
        if didHideMenuObserver != nil {
            NotificationCenter.default.removeObserver(didHideMenuObserver!)
            didHideMenuObserver = nil
        }
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        collectionView.collectionViewLayout.invalidateLayout()
    }
    
    func loadLocalItems() {
        do {
            let urls = try FileManager.default.contentsOfDirectory(at: ProjectManager.shared.localDocumentsUrl, includingPropertiesForKeys: nil, options: [])
            var items = [ExplorerItem]()
            for url in urls {
                if url.pathExtension == "nx" {
                    items.append(ExplorerItem(fileUrl: url))
                }
            }
            items.sort(by: { (item1, item2) -> Bool in
                return item1.createdAt < item2.createdAt
            })
            self.items = items
        } catch {
            // error
            items = nil
        }
        collectionView.reloadData()
        updateFooter()
    }
    
    private func setupCloud() {
        self.items = nil
        collectionView.reloadData()
        updateFooter()
        
        activityView.startAnimating()
        
        let query = NSMetadataQuery()
        metadataQuery = query
        query.searchScopes = [NSMetadataQueryUbiquitousDocumentsScope]
        query.predicate = NSPredicate(format: "%K LIKE '*.nx'", NSMetadataItemFSNameKey)
        query.delegate = self
        
        queryDidFinishGatheringObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSMetadataQueryDidFinishGathering, object: query, queue: nil, using: { [weak self] (notification) in
            self?.activityView.stopAnimating()
            self?.updateCloudFileList()
        })
        
        queryDidUpdateObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name.NSMetadataQueryDidUpdate, object: query, queue: nil, using: { [weak self] (notification) in
            self?.updateCloudFileList()
        })
        query.start()
    }
    
    private func removeCloudObservers() {
        metadataQuery?.stop()
        if queryDidFinishGatheringObserver != nil {
            NotificationCenter.default.removeObserver(queryDidFinishGatheringObserver!)
            queryDidFinishGatheringObserver = nil
        }
        if queryDidUpdateObserver != nil {
            NotificationCenter.default.removeObserver(queryDidUpdateObserver!)
            queryDidUpdateObserver = nil
        }
    }
    
    private func updateCloudFileList() {
        guard let query = metadataQuery else {
            return
        }
        
        query.disableUpdates()
        
        var items = query.results as! [ExplorerItem]
        items.sort(by: { (item1, item2) -> Bool in
            return item1.createdAt < item2.createdAt
        })
        self.items = items
        collectionView.reloadData()
        updateFooter()
        
        query.enableUpdates()
    }
    
    func showAddedItem() {
        if let addedItem = addedItem, items != nil {
            items!.append(addedItem)
            let indexPath = IndexPath(item: items!.count - 1, section: 0)
            collectionView.insertItems(at: [indexPath])
            collectionView.scrollToItem(at: indexPath, at: .bottom, animated: true)
            updateFooter()
            self.addedItem = nil
        }
    }
    
    private func updateFooter() {
        if let footerView = collectionView.supplementaryView(forElementKind: UICollectionView.elementKindSectionFooter, at: IndexPath(item: 0, section: 0)) {
            footerView.isHidden = items?.isEmpty ?? true
        }
    }
    
    @objc func onAddProjectTapped(_ sender: Any) {
        ProjectManager.shared.addProject(originalName: "Unnamed Program", programData: nil, imageData: nil) { (error) in
            if let error = error {
                self.showAlert(withTitle: "Could not Add New Project", message: error.localizedDescription, block: nil)
            }
        }
    }
    
    @objc func onActionTapped(_ sender: UIBarButtonItem) {
        let alert = UIAlertController(title: "Options", message: nil, preferredStyle: .actionSheet)
        
        let addAction = UIAlertAction(title: "Reinstall Default Programs", style: .default, handler: { [weak self] (action) in
            self?.onReinstallTapped()
        })
        alert.addAction(addAction)
        
        if let username = AppController.shared.username {
            let logoutAction = UIAlertAction(title: "Log Out (\(username))", style: .default, handler: { [weak self] (action) in
                self?.logout()
            })
            alert.addAction(logoutAction)
        }
        
        let cancelAction = UIAlertAction(title:"Cancel", style: .cancel, handler: nil)
        alert.addAction(cancelAction)

        alert.popoverPresentationController?.barButtonItem = sender
        present(alert, animated: true, completion: nil)
    }
    
    func onReinstallTapped() {
        let alert = UIAlertController(title: "Reinstall Default Programs?", message: "This may overwrite changes you made to them.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Reinstall", style: .destructive, handler: { [weak self] (action) in
            self?.reinstall()
        }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        present(alert, animated: true, completion: nil)
    }
    
    func reinstall() {
        BlockerView.show()
        
        ProjectManager.shared.reinstallBundlePrograms {
            BlockerView.dismiss()
            
            if !ProjectManager.shared.isCloudEnabled {
                self.loadLocalItems()
            }
        }
    }
    
    func logout() {
        let urlString = ShareViewController.baseUrl.appendingPathComponent("logout.php").absoluteString + "?webmode=app";
        let vc = WebViewController()
        vc.url = URL(string: urlString)!
        vc.title = "Log Out"
        let nc = UINavigationController(rootViewController: vc)
        present(nc, animated: true, completion: nil)
        
        AppController.shared.didLogOut()
    }
    
    func showEditor(fileUrl: URL) {
        let document = ProjectDocument(fileURL: fileUrl)
        let vc = storyboard!.instantiateViewController(withIdentifier: "EditorView") as! EditorViewController
        vc.document = document
        navigationController?.pushViewController(vc, animated: true)
    }
    
    func deleteItem(_ item: ExplorerItem) {
        metadataQuery?.disableUpdates()
        
        ProjectManager.shared.deleteProject(item: item) { (error) in
            if let error = error {
                self.metadataQuery?.enableUpdates()
                self.showAlert(withTitle: "Could not Delete Program", message: error.localizedDescription, block: nil)
            } else {
                self.collectionView.performBatchUpdates({
                    if let index = self.items?.firstIndex(of: item) {
                        self.items?.remove(at: index)
                        self.collectionView.deleteItems(at: [IndexPath(item: index, section: 0)])
                    }
                }, completion: { (finished) in
                    self.updateFooter()
                    self.metadataQuery?.enableUpdates()
                })
            }
        }
    }
    
    func renameItem(_ item: ExplorerItem, newName: String) {
        metadataQuery?.disableUpdates()
        
        ProjectManager.shared.renameProject(item: item, newName: newName) { (error) in
            if let error = error {
                self.metadataQuery?.enableUpdates()
                self.showAlert(withTitle: "Could not Rename Program", message: error.localizedDescription, block: nil)
            } else {
                self.collectionView.performBatchUpdates({
                    if let index = self.items?.firstIndex(of: item) {
                        self.collectionView.reloadItems(at: [IndexPath(item: index, section: 0)])
                    }
                }, completion: { (finished) in
                    self.metadataQuery?.enableUpdates()
                })
            }
        }
    }
    
    func duplicateItem(_ item: ExplorerItem) {
        metadataQuery?.disableUpdates()
        
        ProjectManager.shared.duplicateProject(item: item) { (error) in
            self.metadataQuery?.enableUpdates()
            
            if let error = error {
                self.showAlert(withTitle: "Could not Duplicate Program", message: error.localizedDescription, block: nil)
            }
        }
    }
    
    //MARK: - UICollectionViewDataSource
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.items?.count ?? 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProjectCell", for: indexPath) as! ExplorerItemCell
        cell.item = self.items?[indexPath.item]
        cell.delegate = self
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, viewForSupplementaryElementOfKind kind: String, at indexPath: IndexPath) -> UICollectionReusableView {
        let footerView = collectionView.dequeueReusableSupplementaryView(ofKind: kind, withReuseIdentifier: "Footer", for: indexPath)
        footerView.isHidden = items?.isEmpty ?? true
        return footerView
    }
    
    //MARK: - UICollectionViewDelegateFlowLayout
    
    public func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        let item = items![indexPath.item]
        showEditor(fileUrl: item.fileUrl)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        //let traits = collectionView.traitCollection
        var cellSize: CGSize
//        if traits.horizontalSizeClass == .regular && traits.verticalSizeClass == .regular {
//            cellSize = CGSize(width: 180, height: 170)
//        } else {
//            cellSize = CGSize(width: 110, height: 105)
//        }
        // cellSize = CGSize(width:110, height: 185)
        cellSize = CGSize(width:110, height: 150)
        let layout = collectionViewLayout as! UICollectionViewFlowLayout
        let width = collectionView.bounds.size.width - layout.sectionInset.left - layout.sectionInset.right
        let numItemsPerLine = floor(width / cellSize.width)
        return CGSize(width: floor(width / numItemsPerLine), height: cellSize.height)
    }
    
    @available(iOS 13.0, *)
    func collectionView(_ collectionView: UICollectionView, contextMenuConfigurationForItemAt indexPath: IndexPath, point: CGPoint) -> UIContextMenuConfiguration? {
        guard let item = items?[indexPath.item], let cell = collectionView.cellForItem(at: indexPath) as? ExplorerItemCell else { return nil }
        
        return UIContextMenuConfiguration(identifier: nil, previewProvider: nil) { actions -> UIMenu? in
            let renameAction = UIAction(title: "Rename...", image: UIImage(systemName: "pencil")) { [weak self] (action) in
                self?.explorerItemCell(cell, didSelectRename: item)
            }
            let duplicateAction = UIAction(title: "Duplicate", image: UIImage(systemName: "plus.square.on.square")) { [weak self] (action) in
                self?.explorerItemCell(cell, didSelectDuplicate: item)
            }
            let deleteAction = UIAction(title: "Delete...", image: UIImage(systemName: "minus.circle"), attributes: .destructive) { [weak self] (action) in
                self?.explorerItemCell(cell, didSelectDelete: item)
            }
            return UIMenu(title: "", children: [renameAction, duplicateAction, deleteAction])
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, shouldShowMenuForItemAt indexPath: IndexPath) -> Bool {
        UIMenuController.shared.menuItems = [
            UIMenuItem(title: "Rename...", action: #selector(ExplorerItemCell.renameItem)),
            UIMenuItem(title: "Duplicate", action: #selector(ExplorerItemCell.duplicateItem)),
            UIMenuItem(title: "Delete...", action: #selector(ExplorerItemCell.deleteItem))
        ]
        return true
    }
    
    func collectionView(_ collectionView: UICollectionView, canPerformAction action: Selector, forItemAt indexPath: IndexPath, withSender sender: Any?) -> Bool {
        if action == #selector(ExplorerItemCell.renameItem) || action == #selector(ExplorerItemCell.deleteItem) || action == #selector(ExplorerItemCell.duplicateItem) {
            return true
        }
        return false
    }
    
    func collectionView(_ collectionView: UICollectionView, performAction action: Selector, forItemAt indexPath: IndexPath, withSender sender: Any?) {
    }
    
    //MARK: - ExplorerItemCellDelegate
    
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectRename item: ExplorerItem) {
        let alert = UIAlertController(title: "Rename “\(item.name)”", message: nil, preferredStyle: .alert)
        alert.addTextField { (textField) in
            textField.clearButtonMode = .always
            textField.autocapitalizationType = .words
            textField.autocorrectionType = .no
            textField.spellCheckingType = .no
            textField.text = item.name
        }
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        alert.addAction(UIAlertAction(title: "Accept", style: .default, handler: { (action) in
            let textField = alert.textFields!.first!
            if let name = textField.text?.trimmingCharacters(in: .whitespaces) {
                self.renameItem(item, newName: name)
            }
        }))
        present(alert, animated: true, completion: nil)
    }
    
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectDelete item: ExplorerItem) {
        var message: String?
        if ProjectManager.shared.isCloudEnabled {
            message = "This file will be deleted from iCloud Drive and all your iCloud devices."
        }
        let alert = UIAlertController(title: "Do you really want to delete “\(item.name)”?", message: message, preferredStyle: .actionSheet)
        alert.addAction(UIAlertAction(title: "Delete", style: .destructive, handler: { [unowned self] (action) in
            self.deleteItem(item)
        }))
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
        present(alert, animated: true, completion: nil)
        if let pop = alert.popoverPresentationController {
            pop.sourceView = cell
            pop.sourceRect = cell.bounds
            pop.permittedArrowDirections = [.down, .up]
        }
    }
    
    func explorerItemCell(_ cell: ExplorerItemCell, didSelectDuplicate item: ExplorerItem) {
        duplicateItem(item)
    }
    
    //MARK: - NSMetadataQueryDelegate
    
    func metadataQuery(_ query: NSMetadataQuery, replacementObjectForResultObject result: NSMetadataItem) -> Any {
        var resultItem: ExplorerItem
        let url = result.value(forAttribute: NSMetadataItemURLKey) as! URL
        if let item = unassignedItems[url] {
            unassignedItems.removeValue(forKey: url)
            resultItem = item
        } else {
            resultItem = ExplorerItem(fileUrl: url)
        }
        resultItem.metadataItem = result
        return resultItem
    }
    
}
