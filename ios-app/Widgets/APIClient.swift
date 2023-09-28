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
    
    static let baseUrl = URL(string: "https://lowresnx.inutilis.com/")!
    static let shared: APIClient = APIClient()
    
    func fetchProgramOfTheDay(completion: @escaping (Result<ProgramModel, Error>) -> Void) {
        let date = ISO8601DateFormatter.string(from: Date(), timeZone: TimeZone.current, formatOptions: .withFullDate)
        let url = URL(string: "ajax/program_of_the_day.php?date=\(date)", relativeTo: APIClient.baseUrl)!
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
