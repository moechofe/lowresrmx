//
//  ProjectManager.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 24/11/17.
//  Copyright © 2017-2019 Inutilis Software. All rights reserved.
//

import UIKit

extension Notification.Name {
    static let ProjectManagerDidAddProgram = Notification.Name("ProjectManagerDidAddProgram")
}

class ProjectManager: NSObject {
    
    static let shared = ProjectManager()
    
    var isCloudEnabled: Bool {
        // TODO: disable iCloud for now
        return false
        return FileManager.default.ubiquityIdentityToken != nil
    }
    
    private(set) lazy var localDocumentsUrl: URL = {
        return FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
    }()
    
    private(set) lazy var applicationSupportUrl: URL = {
        return FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
    }()
    
    private(set) var ubiquitousContainerUrl: URL?
    
    var ubiquitousDocumentsUrl: URL? {
        return ubiquitousContainerUrl?.appendingPathComponent("Documents")
    }
    
    var currentDocumentsUrl: URL {
        if isCloudEnabled {
            return ubiquitousDocumentsUrl!
        } else {
            return localDocumentsUrl
        }
    }
    
    private let queue = OperationQueue()
    
    func setup(completion: @escaping (() -> Void)) {
        print("localDocumentsUrl:", localDocumentsUrl)
        
        queue.addOperation {
            if self.isCloudEnabled {
                self.ubiquitousContainerUrl = FileManager.default.url(forUbiquityContainerIdentifier: nil)
                
                // make sure Documents directory exists
                if let ubiquitousDocumentsUrl = self.ubiquitousDocumentsUrl {
                    do {
                        if !FileManager.default.fileExists(atPath: ubiquitousDocumentsUrl.path) {
                            print("create dir:", ubiquitousDocumentsUrl.path)
                            try FileManager.default.createDirectory(at: ubiquitousDocumentsUrl, withIntermediateDirectories: true, attributes: nil)
                        }
                    } catch {
                        print(error.localizedDescription)
                    }
                }
                
                self.moveLocalProjectsToCloud()
            }
            
            self.copyBundleProgramsIfNeeded(overwrite: false)
            
            OperationQueue.main.addOperation {
                if self.isCloudEnabled {
                    self.setupCloudIcons()
                }
                completion()
            }
        }
    }
    
    func reinstallBundlePrograms(completion: @escaping (() -> Void)) {
        UserDefaults.standard.set(nil, forKey: copiedBundleProgramsKey)
        
        queue.addOperation {
            self.copyBundleProgramsIfNeeded(overwrite: true)
            OperationQueue.main.addOperation {
                completion()
            }
        }
    }
    
    //MARK: - Programs
    
    func importProgram(from url: URL, completion: @escaping ((Error?) -> Void)) {
        let originalName = url.deletingPathExtension().lastPathComponent
        let name = self.availableProgramName(original: originalName)
        let destUrl = self.currentDocumentsUrl.appendingPathComponent(name).appendingPathExtension("nx")
        
        let readIntent = NSFileAccessIntent.readingIntent(with: url, options: [])
        let writeIntent = NSFileAccessIntent.writingIntent(with: destUrl, options: .forReplacing)
        
        NSFileCoordinator().coordinate(with: [readIntent, writeIntent], queue: self.queue) { (error) in
            guard error == nil else {
                OperationQueue.main.addOperation { completion(error) }
                return
            }
            
            do {
                try FileManager.default.copyItem(at: readIntent.url, to: writeIntent.url)
                let item = ExplorerItem(fileUrl: writeIntent.url)
                
                OperationQueue.main.addOperation {
                    completion(nil)
                    self.postNotification(name: .ProjectManagerDidAddProgram, for: item)
                }
            } catch {
                OperationQueue.main.addOperation {
                    completion(error)
                }
            }
        }
    }
    
    func addProject(originalName: String, programData: Data?, imageData: Data?, completion: @escaping ((Error?) -> Void)) {
        let safeName = originalName.replacingOccurrences(of: "/", with: "")
        let name = self.availableProgramName(original: safeName)
        let programUrl = self.currentDocumentsUrl.appendingPathComponent(name).appendingPathExtension("nx")
        let imageUrl = self.currentDocumentsUrl.appendingPathComponent(name).appendingPathExtension("png")
        
        let programWriteIntent = NSFileAccessIntent.writingIntent(with: programUrl, options: .forReplacing)
        let imageWriteIntent = NSFileAccessIntent.writingIntent(with: imageUrl, options: .forReplacing)
        
        NSFileCoordinator().coordinate(with: [programWriteIntent, imageWriteIntent], queue: self.queue) { (error) in
            guard error == nil else {
                OperationQueue.main.addOperation { completion(error) }
                return
            }
            
            var resultItem: ExplorerItem?
            
            // program
            if FileManager.default.createFile(atPath: programWriteIntent.url.path, contents: programData, attributes: nil) {
                resultItem = ExplorerItem(fileUrl: programWriteIntent.url)
                
                // image
                if let imageData = imageData {
                    FileManager.default.createFile(atPath: imageWriteIntent.url.path, contents: imageData, attributes: nil)
                }
            }
            
            OperationQueue.main.addOperation {
                completion(nil)
                if let item = resultItem {
                    self.postNotification(name: .ProjectManagerDidAddProgram, for: item)
                }
            }
        }
    }
    
    private func availableProgramName(original: String) -> String {
        var name = original
        var count = 1
        
        var url = currentDocumentsUrl.appendingPathComponent(name).appendingPathExtension("nx")
        
        while (try? url.checkPromisedItemIsReachable()) ?? false {
            count += 1
            name = "\(original) (\(count))"
            url = currentDocumentsUrl.appendingPathComponent(name).appendingPathExtension("nx")
        }

        return name
    }
    
    func deleteProject(item: ExplorerItem, completion: @escaping (Error?) -> Void) {
        self.deletePersistentRam(programUrl: item.fileUrl)
        
        let programWriteIntent = NSFileAccessIntent.writingIntent(with: item.fileUrl, options: .forDeleting)
        let imageWriteIntent = NSFileAccessIntent.writingIntent(with: item.imageUrl, options: .forDeleting)
        
        NSFileCoordinator().coordinate(with: [programWriteIntent, imageWriteIntent], queue: self.queue) { (error) in
            guard error == nil else {
                OperationQueue.main.addOperation { completion(error) }
                return
            }
            
            do {
                // program file
                try FileManager.default.removeItem(at: programWriteIntent.url)
                
                // image file
                try? FileManager.default.removeItem(at: imageWriteIntent.url)
                
                OperationQueue.main.addOperation {
                    completion(nil)
                }
            } catch {
                OperationQueue.main.addOperation {
                    completion(error)
                }
            }
        }
    }
    
    func renameProject(item: ExplorerItem, newName: String, completion: @escaping (Error?) -> Void) {
        let safeName = newName.replacingOccurrences(of: "/", with: "")
        let programDestUrl = item.fileUrl.deletingLastPathComponent().appendingPathComponent(safeName).appendingPathExtension("nx")
        let imageDestUrl = item.fileUrl.deletingLastPathComponent().appendingPathComponent(safeName).appendingPathExtension("png")
        
        let programSrcWriteIntent = NSFileAccessIntent.writingIntent(with: item.fileUrl, options: .forMoving)
        let programDestWriteIntent = NSFileAccessIntent.writingIntent(with: programDestUrl, options: .forReplacing)
        let imageSrcWriteIntent = NSFileAccessIntent.writingIntent(with: item.imageUrl, options: .forMoving)
        let imageDestWriteIntent = NSFileAccessIntent.writingIntent(with: imageDestUrl, options: .forReplacing)

        NSFileCoordinator().coordinate(with: [programSrcWriteIntent, programDestWriteIntent, imageSrcWriteIntent, imageDestWriteIntent], queue: self.queue) { (error) in
            guard error == nil else {
                OperationQueue.main.addOperation { completion(error) }
                return
            }
            
            do {
                // program file
                try FileManager.default.moveItem(at: programSrcWriteIntent.url, to: programDestWriteIntent.url)
                item.fileUrl = programDestWriteIntent.url
                
                // image file
                try? FileManager.default.moveItem(at: imageSrcWriteIntent.url, to: imageDestWriteIntent.url)
                
                OperationQueue.main.addOperation {
                    completion(nil)
                }
            } catch {
                OperationQueue.main.addOperation {
                    completion(error)
                }
            }
        }
    }
    
    func duplicateProject(item: ExplorerItem, completion: @escaping (Error?) -> Void) {
        let newName = availableProgramName(original: "Copy of \(item.name)")
        let programDestUrl = self.currentDocumentsUrl.appendingPathComponent(newName).appendingPathExtension("nx")
        let imageDestUrl = self.currentDocumentsUrl.appendingPathComponent(newName).appendingPathExtension("png")
        
        let programReadIntent = NSFileAccessIntent.readingIntent(with: item.fileUrl, options: [])
        let programWriteIntent = NSFileAccessIntent.writingIntent(with: programDestUrl, options: .forReplacing)
        let imageReadIntent = NSFileAccessIntent.readingIntent(with: item.imageUrl, options: [])
        let imageWriteIntent = NSFileAccessIntent.writingIntent(with: imageDestUrl, options: .forReplacing)
        
        NSFileCoordinator().coordinate(with: [programReadIntent, programWriteIntent, imageReadIntent, imageWriteIntent], queue: self.queue) { (error) in
            guard error == nil else {
                OperationQueue.main.addOperation { completion(error) }
                return
            }
            
            do {
                // program file
                try FileManager.default.copyItem(at: programReadIntent.url, to: programWriteIntent.url)
                let item = ExplorerItem(fileUrl: programWriteIntent.url)
                
                // image file
                try? FileManager.default.copyItem(at: imageReadIntent.url, to: imageWriteIntent.url)
                
                OperationQueue.main.addOperation {
                    completion(nil)
                    self.postNotification(name: .ProjectManagerDidAddProgram, for: item)
                }
            } catch {
                OperationQueue.main.addOperation {
                    completion(error)
                }
            }
        }
    }
    
    func saveProjectIcon(programUrl: URL, image: UIImage) {
        guard let imageData = image.pngData() else { return }
    
        let imageUrl = programUrl.deletingPathExtension().appendingPathExtension("png")
        let writeIntent = NSFileAccessIntent.writingIntent(with: imageUrl, options: .forReplacing)
        
        NSFileCoordinator().coordinate(with: [writeIntent], queue: self.queue) { (error) in
            guard error == nil else { return }
            FileManager.default.createFile(atPath: writeIntent.url.path, contents: imageData, attributes: nil)
        }
    }
    
    func savePersistentRam(programUrl: URL, data: Data) {
        let fileUrl = persistentRamUrl(programUrl: programUrl)
        do {
            try FileManager.default.createDirectory(at: applicationSupportUrl, withIntermediateDirectories: true, attributes: nil)
            try data.write(to: fileUrl)
        } catch {
            print("savePersistentRam:", error.localizedDescription)
        }
    }
    
    func loadPersistentRam(programUrl: URL) -> Data? {
        let fileUrl = persistentRamUrl(programUrl: programUrl)
        return try? Data(contentsOf: fileUrl)
    }
    
    func deletePersistentRam(programUrl: URL) {
        let fileUrl = persistentRamUrl(programUrl: programUrl)
        try? FileManager.default.removeItem(at: fileUrl)
    }
    
    private func persistentRamUrl(programUrl: URL) -> URL {
        let filename = programUrl.deletingPathExtension().appendingPathExtension("dat").lastPathComponent
        return applicationSupportUrl.appendingPathComponent(filename)
    }
    
    func getDiskDocument(completion: @escaping (ProjectDocument?, Error?) -> Void) {
        let fileUrl = currentDocumentsUrl.appendingPathComponent("Disk.nx")
        let document = ProjectDocument(fileURL: fileUrl)
        document.open { (success) in
            if success {
                if document.documentState == .normal {
                    completion(document, nil)
                } else {
                    completion(nil, NSError(domain: "LowResNXCoder", code: 0, userInfo: [NSLocalizedDescriptionKey: "“Disk.nx” is currently not editable."]))
                }
            } else {
                document.save(to: document.fileURL, for: .forCreating, completionHandler: { (success) in
                    if success {
                        self.postNotification(name: .ProjectManagerDidAddProgram, for: ExplorerItem(fileUrl: document.fileURL))
                        completion(document, nil)
                    } else {
                        completion(nil, NSError(domain: "LowResNXCoder", code: 0, userInfo: [NSLocalizedDescriptionKey: "“Disk.nx” could not be created."]))
                    }
                })
            }
        }
    }
    
    private func postNotification(name: NSNotification.Name, for item: ExplorerItem) {
        NotificationCenter.default.post(name: name, object: self, userInfo: ["item": item])
    }
    
    //MARK: - Bundle Programs
    
    private func copyBundleProgramsIfNeeded(overwrite: Bool) {
        do {
            let programsUrl = Bundle.main.bundleURL.appendingPathComponent("programs", isDirectory: true)
            let urls = try FileManager.default.contentsOfDirectory(at: programsUrl, includingPropertiesForKeys: nil, options: [])
            
            for url in urls {
                let filename = url.lastPathComponent
                
                if shouldCopyBundleProgram(filename: filename) {
                    let targetUrl = currentDocumentsUrl.appendingPathComponent(filename)
                    
                    if !overwrite && (try? targetUrl.checkPromisedItemIsReachable()) ?? false {
                        print("bundle - already exists:", targetUrl.path)
                        didCopyBundleProgram(filename: filename)
                    } else {
                        let tempUrl = URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true).appendingPathComponent(UUID().uuidString)
                        try FileManager.default.copyItem(at: url, to: tempUrl)
                        
                        var coordError: NSError?
                        NSFileCoordinator().coordinate(writingItemAt: tempUrl, options: [], writingItemAt: targetUrl, options: .forReplacing, error: &coordError) { (fromUrl, toUrl) in
                            do {
                                print("bundle - replace:", targetUrl.path)
                                let _ = try FileManager.default.replaceItemAt(toUrl, withItemAt: fromUrl, backupItemName: nil, options: [])
                                self.didCopyBundleProgram(filename: filename)
                            } catch {
                                print(error.localizedDescription)
                            }
                        }
                        if let error = coordError {
                            print(error.localizedDescription)
                        }
                    }
                }
            }
        } catch {
            print(error.localizedDescription)
        }
    }
    
    private let copiedBundleProgramsKey = "CopiedBundlePrograms"
    
    private func shouldCopyBundleProgram(filename: String) -> Bool {
        let copiedPrograms: [String] = UserDefaults.standard.array(forKey: copiedBundleProgramsKey) as? [String] ?? []
        return !copiedPrograms.contains(filename)
    }
    
    private func didCopyBundleProgram(filename: String) {
        var copiedPrograms: [String] = UserDefaults.standard.array(forKey: copiedBundleProgramsKey) as? [String] ?? []
        copiedPrograms.append(filename)
        UserDefaults.standard.set(copiedPrograms, forKey: copiedBundleProgramsKey)
    }
    
    //MARK: - Cloud
    
    private func moveLocalProjectsToCloud() {
        guard let ubiquitousDocumentsUrl = self.ubiquitousDocumentsUrl else {
            assertionFailure()
            return
        }
        do {
            let urls = try FileManager.default.contentsOfDirectory(at: self.localDocumentsUrl, includingPropertiesForKeys: nil, options: [.skipsHiddenFiles])
            
            for url in urls {
                if !url.hasDirectoryPath && url.pathExtension == "nx" {
                    var coordError: NSError?
                    
                    let originalName = url.deletingPathExtension().lastPathComponent
                    let name = availableProgramName(original: originalName)
                    
                    // program file
                    let destUrl = ubiquitousDocumentsUrl.appendingPathComponent(name).appendingPathExtension("nx")
                    
                    NSFileCoordinator().coordinate(writingItemAt: url, options: [], writingItemAt: destUrl, options: .forReplacing, error: &coordError) { (fromUrl, toUrl) in
                        do {
                            print("local - move:", toUrl.path)
                            try FileManager.default.moveItem(at: fromUrl, to: toUrl)
                        } catch {
                            print(error.localizedDescription)
                        }
                    }
                    if let error = coordError {
                        print(error.localizedDescription)
                    }
                    
                    // image file
                    let imageUrl = url.deletingPathExtension().appendingPathExtension("png")

                    if FileManager.default.fileExists(atPath: imageUrl.path) {
                        let imageDestUrl = ubiquitousDocumentsUrl.appendingPathComponent(name).appendingPathExtension("png")
                        
                        NSFileCoordinator().coordinate(writingItemAt: imageUrl, options: [], writingItemAt: imageDestUrl, options: .forReplacing, error: &coordError) { (fromUrl, toUrl) in
                            do {
                                print("local - move:", toUrl.path)
                                try FileManager.default.moveItem(at: fromUrl, to: toUrl)
                            } catch {
                                print(error.localizedDescription)
                            }
                        }
                        if let error = coordError {
                            print(error.localizedDescription)
                        }
                    }
                }
            }
        } catch {
            print(error.localizedDescription)
        }
    }
    
    //MARK: - Cloud Icons
    
    private var iconsMetadataQuery: NSMetadataQuery?
    private var iconsQueryDidFinishGatheringObserver: Any?
    private var iconsQueryDidUpdateObserver: Any?
    
    private func setupCloudIcons() {
        let query = NSMetadataQuery()
        iconsMetadataQuery = query
        query.searchScopes = [NSMetadataQueryUbiquitousDocumentsScope]
        query.predicate = NSPredicate(format: "%K LIKE '*.png'", NSMetadataItemFSNameKey)
        
        iconsQueryDidFinishGatheringObserver = NotificationCenter.default.addObserver(
            forName: NSNotification.Name.NSMetadataQueryDidFinishGathering,
            object: query,
            queue: nil,
            using: { [weak self] (notification) in
                self?.cloudIconListReceived(query: notification.object as! NSMetadataQuery)
            }
        )
        
        iconsQueryDidUpdateObserver = NotificationCenter.default.addObserver(
            forName: NSNotification.Name.NSMetadataQueryDidUpdate,
            object: query,
            queue: nil,
            using: { [weak self] (notification) in
                self?.cloudIconListReceived(query: notification.object as! NSMetadataQuery)
            }
        )
        
        query.start()
    }
    
    private func removeCloudObservers() {
        iconsMetadataQuery?.stop()
        if iconsQueryDidFinishGatheringObserver != nil {
            NotificationCenter.default.removeObserver(iconsQueryDidFinishGatheringObserver!)
            iconsQueryDidFinishGatheringObserver = nil
        }
        if iconsQueryDidUpdateObserver != nil {
            NotificationCenter.default.removeObserver(iconsQueryDidUpdateObserver!)
            iconsQueryDidUpdateObserver = nil
        }
    }
    
    private func cloudIconListReceived(query: NSMetadataQuery) {
        query.disableUpdates()
        for result in query.results as! [NSMetadataItem] {
            let status = result.value(forAttribute: NSMetadataUbiquitousItemDownloadingStatusKey) as! String
            let isDownloading = result.value(forAttribute: NSMetadataUbiquitousItemIsDownloadingKey) as! Bool
            if status == NSMetadataUbiquitousItemDownloadingStatusNotDownloaded && !isDownloading {
                let url = result.value(forAttribute: NSMetadataItemURLKey) as! URL
                do {
                    print("startDownloadingUbiquitousItem:", url)
                    try FileManager.default.startDownloadingUbiquitousItem(at: url)
                } catch {
                    print("startDownloadingUbiquitousItem:", error.localizedDescription)
                }
            }
        }
        query.enableUpdates()
    }
    
    // MARK: - Test
    
    private func list(_ dirUrl: URL) {
        do {
            print(dirUrl.path)
            let urls = try FileManager.default.contentsOfDirectory(at: dirUrl, includingPropertiesForKeys: nil, options: [.skipsHiddenFiles])
            for url in urls {
                print(url.lastPathComponent)
            }
        } catch {
            print(error.localizedDescription)
        }
    }
    
}
