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

class CoreStatsWrapper: NSObject
{
	struct Result
	{
		let error: LowResRMXError?
		let numTokens: Int
		let romSize: Int
	}

	// `stats` (and its embedded tokenizer) is mutable state that must never be
	// touched by two threads at once: stats_update() both fills and frees the
	// tokenizer, so concurrent calls double-free the token strings and corrupt
	// the heap. All access is funneled through this serial queue.
	private var stats = Stats()
	private let queue = DispatchQueue(label: "it.ro.ret.ios.LowResRMX.corestats")

	override init()
	{
		super.init()
		stats_init(&stats)
	}

	deinit
	{
		stats_deinit(&stats)
	}

	/// Runs stats_update on the private serial queue and delivers a value
	/// snapshot back on the main queue. Overlapping calls are serialized.
	func update(sourceCode: String, completion: @escaping (Result) -> Void)
	{
		queue.async
		{
			let cString = sourceCode.cString(using: .utf8)
			let error = stats_update(&self.stats, cString)

			let result = if error.code != ErrorNone
			{
				Result(error: LowResRMXError(error: error, sourceCode: sourceCode), numTokens: 0, romSize: 0)
			}
			else
			{
				Result(error: nil, numTokens: Int(self.stats.numTokens), romSize: Int(self.stats.romSize))
			}

			DispatchQueue.main.async
			{
				completion(result)
			}
		}
	}
}
