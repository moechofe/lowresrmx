//
//  DonationViewController.swift
//  LowResRMX
//
//  Created by Timo Kloss on 15/02/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import UIKit
import StoreKit

class DonationViewController: UITableViewController, SKProductsRequestDelegate, SKPaymentTransactionObserver {
    
    @IBOutlet weak var activityView: UIActivityIndicatorView!
    
    var productsRequest: SKProductsRequest?
    let productIds = ["nx_donation_1", "nx_donation_2", "nx_donation_3"]
    var products = [String: SKProduct]()
    
    var isLoading = false {
        didSet {
            if isLoading {
                activityView.startAnimating()
            } else {
                activityView.stopAnimating()
            }
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        isLoading = true
        let productsRequest = SKProductsRequest(productIdentifiers: Set(productIds))
        productsRequest.delegate = self;
        productsRequest.start()
        self.productsRequest = productsRequest
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        SKPaymentQueue.default().add(self)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        SKPaymentQueue.default().remove(self)
    }

    // MARK: - Table view data source

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return isLoading ? 0 : productIds.count
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "DonateCell", for: indexPath)
        let productId = productIds[indexPath.row]
        
        if let product = products[productId] {
            let numberFormatter = NumberFormatter()
            numberFormatter.formatterBehavior = .behavior10_4
            numberFormatter.numberStyle = .currency
            numberFormatter.locale = product.priceLocale
            
            cell.textLabel!.text = product.localizedTitle
            cell.detailTextLabel!.text = numberFormatter.string(from: product.price)

        } else {
            cell.textLabel!.text = "Error"
            cell.detailTextLabel!.text = ""
        }
        return cell
    }
    
    // MARK: - UITableViewDelegate
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        let productId = productIds[indexPath.row]
        
        if let product = products[productId] {
            let payment = SKMutablePayment(product: product)
            SKPaymentQueue.default().add(payment)
        }
        
        tableView.deselectRow(at: indexPath, animated: true)
    }
    
    // MARK: - SKProductsRequestDelegate
    
    func productsRequest(_ request: SKProductsRequest, didReceive response: SKProductsResponse) {
        for product in response.products {
            products[product.productIdentifier] = product
        }
        DispatchQueue.main.async {
            self.isLoading = false
            self.tableView.reloadData()
        }
    }
    
    // MARK: - SKPaymentTransactionObserver
    
    func paymentQueue(_ queue: SKPaymentQueue, updatedTransactions transactions: [SKPaymentTransaction]) {
        for transaction in transactions {
            switch transaction.transactionState {
            case .purchasing:
                BlockerView.show()
                
            case .purchased:
                SKPaymentQueue.default().finishTransaction(transaction)
                AppController.shared.hasDontated = true
                
                BlockerView.dismiss()
                
                let alert = UIAlertController(title: "Donation Successful", message: "Thank you for your support!", preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { (action) in
                    AppController.shared.requestAppStoreReview()
                }))
                present(alert, animated: true, completion: nil)
                
            case .failed:
                SKPaymentQueue.default().finishTransaction(transaction)
                
                BlockerView.dismiss()
                
                let alert = UIAlertController(title: "Donation Canceled", message: transaction.error?.localizedDescription, preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
                present(alert, animated: true, completion: nil)
                
            case .restored:
                assertionFailure()
                BlockerView.dismiss()
                
            case .deferred:
                assertionFailure()
                BlockerView.dismiss()
                
            @unknown default:
                BlockerView.dismiss()
            }
        }
    }
    
}
