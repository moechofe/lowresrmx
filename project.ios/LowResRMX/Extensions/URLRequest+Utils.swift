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

import Foundation

struct MultipartFile
{
	let filename: String
	let data: Data
	let mime: String
}

extension URLRequest
{
	mutating func setMultipartBody(parameters: [String: Any])
	{
		let boundary = "1238476192857619283764981256498327645adsflkuhzvbjha"
		setValue("multipart/form-data; boundary=\(boundary)", forHTTPHeaderField: "Content-Type")

		var body = Data()

		for (key, value) in parameters
		{
			if let string = value as? String
			{
				body.append("--\(boundary)\r\n".data(using: .utf8)!)
				body.append("Content-Disposition: form-data; name=\"\(key)\"\r\n\r\n".data(using: .utf8)!)
				body.append("\(string)\r\n".data(using: .utf8)!)
			}
			else if let file = value as? MultipartFile
			{
				body.append("--\(boundary)\r\n".data(using: .utf8)!)
				body.append("Content-Disposition: form-data; name=\"\(key)\"; filename=\"\(file.filename)\"\r\n".data(using: .utf8)!)
				body.append("Content-Type: \(file.mime)\r\n\r\n".data(using: .utf8)!)
				body.append(file.data)
				body.append("\r\n".data(using: .utf8)!)
			}
			else
			{
				assertionFailure("Unsupported type")
			}
		}

		body.append("--\(boundary)--\r\n".data(using: .utf8)!)

		httpBody = body
	}
}
