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

enum APIClientError: Error
{
	case invalidData
}

class APIClient: NSObject
{
	static let shared: APIClient = .init()

	static var baseURL: String
	{
		let key = "USE_SHARE_HOST"
		guard ProcessInfo.processInfo.environment.contains(where: { $0.key == key })
		else
		{
			return "https://ret.ro.it"
		}
		return ProcessInfo.processInfo.environment[key]!
	}

	func fetchProgramOfTheDay(completion: @escaping (Result<ProgramModel, Error>) -> Void)
	{
		let date = ISO8601DateFormatter.string(
			from: Date(), timeZone: TimeZone.current, formatOptions: .withFullDate
		)
		let url = URL(string: "\(APIClient.baseURL)/potd/\(date)")!
		let request = URLRequest(url: url, cachePolicy: .returnCacheDataElseLoad)
		let task = URLSession.shared.dataTask(with: request)
		{ data, _, error in
			guard error == nil
			else
			{
				completion(.failure(error!))
				return
			}
			guard let data
			else
			{
				completion(.failure(APIClientError.invalidData))
				return
			}

			do
			{
				let decoder = JSONDecoder()
				let programModel = try decoder.decode(ProgramModel.self, from: data)
				completion(.success(programModel))
			}
			catch
			{
				completion(.failure(APIClientError.invalidData))
			}
		}
		task.resume()
	}
}
