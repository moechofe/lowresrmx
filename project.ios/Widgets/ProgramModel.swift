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
            URLQueryItem(name: "n", value: name),
            URLQueryItem(name: "p", value: program),
            URLQueryItem(name: "i", value: image),
						URLQueryItem(name: "t", value: String(topicId)
						)
        ]
        return url.url
    }

}
