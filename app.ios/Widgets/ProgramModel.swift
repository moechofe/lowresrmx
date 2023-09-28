//
//  ProgramModel.swift
//  WidgetsExtension
//
//  Created by Timo Kloss on 02/10/2020.
//  Copyright © 2020 Inutilis Software. All rights reserved.
//

import Foundation

struct ProgramModel: Decodable {
    let title: String
    let name: String
    let program: String
    let image: String
    let topicId: Int
    
    var appUrl: URL? {
        var url = URLComponents(string: "lowresrmx:")!
        url.queryItems = [
            URLQueryItem(name: "name", value: name),
            URLQueryItem(name: "program", value: program),
            URLQueryItem(name: "image", value: image),
            URLQueryItem(name: "topic_id", value: String(topicId))
        ]
        return url.url
    }
    
}
