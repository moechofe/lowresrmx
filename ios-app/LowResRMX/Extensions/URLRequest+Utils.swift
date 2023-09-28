//
//  URLRequest+Utils.swift
//  LowResRMX
//
//  Created by Timo Kloss on 15/03/2019.
//  Copyright © 2019 Inutilis Software. All rights reserved.
//

import Foundation

struct MultipartFile {
    let filename: String
    let data: Data
    let mime: String
}

extension URLRequest {
    
    mutating func setMultipartBody(parameters: [String: Any]) {
        let boundary = "1238476192857619283764981256498327645adsflkuhzvbjha"
        setValue("multipart/form-data; boundary=\(boundary)", forHTTPHeaderField: "Content-Type")
        
        var body = Data()
        
        for (key, value) in parameters {
            if let string = value as? String {
                body.append("--\(boundary)\r\n".data(using: .utf8)!)
                body.append("Content-Disposition: form-data; name=\"\(key)\"\r\n\r\n".data(using: .utf8)!)
                body.append("\(string)\r\n".data(using: .utf8)!)
                
            } else if let file = value as? MultipartFile {
                body.append("--\(boundary)\r\n".data(using: .utf8)!)
                body.append("Content-Disposition: form-data; name=\"\(key)\"; filename=\"\(file.filename)\"\r\n".data(using: .utf8)!)
                body.append("Content-Type: \(file.mime)\r\n\r\n".data(using: .utf8)!)
                body.append(file.data)
                body.append("\r\n".data(using: .utf8)!)
                
            } else {
                assertionFailure("Unsupported type")
            }
        }
        
        body.append("--\(boundary)--\r\n".data(using: .utf8)!)
        
        httpBody = body
    }
    
}
