//
//  APIClient.swift
//  WidgetsExtension
//
//  Created by Timo Kloss on 02/10/2020.
//  Copyright © 2020 Inutilis Software. All rights reserved.
//

import UIKit

enum APIClientError: Error {
  case invalidData
}

class APIClient: NSObject {

  static let shared: APIClient = APIClient()

  static var baseURL: String {
    let key = "USE_SHARE_HOST"
    guard ProcessInfo.processInfo.environment.contains(where: { $0.key == key }) else {
      return "https://ret.ro.it"
    }
    return ProcessInfo.processInfo.environment[key]!
  }

  func fetchProgramOfTheDay(completion: @escaping (Result<ProgramModel, Error>) -> Void) {
    let date = ISO8601DateFormatter.string(
      from: Date(), timeZone: TimeZone.current, formatOptions: .withFullDate)
		let url = URL(string: "\(APIClient.baseURL)/potd/\(date)")!
		let request = URLRequest(url: url, cachePolicy: .returnCacheDataElseLoad)
    let task = URLSession.shared.dataTask(with: request) { (data, response, error) in
      guard error == nil else {
        completion(.failure(error!))
        return
      }
      guard let data = data else {
        completion(.failure(APIClientError.invalidData))
        return
      }

      do {
        let decoder = JSONDecoder()
        let programModel = try decoder.decode(ProgramModel.self, from: data)
        completion(.success(programModel))
      } catch {
        completion(.failure(APIClientError.invalidData))
      }
    }
    task.resume()
  }

}
