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

import XCTest

/// Runs test.suite/ through the shared C harness, and pins the two iOS delegate seams that
/// only exist in this target.
///
/// The bundle is injected into App, so every core_* and harness_* symbol already compiled
/// into the app is available here; backend.core must not be added to this target's Sources.
final class HarnessTests: XCTestCase
{
	/// The test.suite folder reference copied into the test bundle's resources.
	private var suiteURL: URL
	{
		get throws
		{
			let bundle = Bundle(for: HarnessTests.self)
			guard let url = bundle.url(forResource: "test.suite", withExtension: nil)
			else
			{
				throw XCTSkip("test.suite is not in the test bundle resources")
			}
			return url
		}
	}

	func testSuite() throws
	{
		let suite = UnsafeMutablePointer<HarnessSuite>.allocate(capacity: 1)
		let core = UnsafeMutablePointer<Core>.allocate(capacity: 1)
		defer
		{
			harness_suiteDeinit(suite)
			core_deinit(core)
			suite.deallocate()
			core.deallocate()
		}

		let outDirectory = FileManager.default.temporaryDirectory.appendingPathComponent("harness")
		try? FileManager.default.createDirectory(at: outDirectory, withIntermediateDirectories: true)

		let suitePath = try suiteURL.path

		harness_suiteInit(suite, outDirectory.path)
		XCTAssertTrue(harness_suiteAddPath(suite, suitePath, nil), "could not read the suite")
		let count = harness_suiteCount(suite)
		XCTAssertGreaterThan(count, 0, "the suite is empty")

		core_init(core)

		for index in 0 ..< count
		{
			var input = CoreInput()

			if harness_beginCase(suite, core, index)
			{
				while !harness_caseDone(suite)
				{
					harness_beforeUpdate(suite, &input)
					core_update(core, &input)
					harness_afterUpdate(suite, core, &input)
				}
			}
			harness_endCase(suite, core)

			let name = String(cString: harness_caseName(suite, index))
			let verdict = harness_caseVerdict(suite, index)
			let status = String(cString: harness_statusLine(suite))

			XCTContext.runActivity(named: name)
			{ activity in
				for messageIndex in 0 ..< harness_caseMessageCount(suite, index)
				{
					let message = String(cString: harness_caseMessage(suite, index, messageIndex))
					activity.add(XCTAttachment(string: message))

					// A golden mismatch reports the frame it wrote; attach it so it can be eyeballed.
					if message.hasPrefix("wrote ")
					{
						let path = String(message.dropFirst("wrote ".count))
						if FileManager.default.fileExists(atPath: path)
						{
							let attachment = XCTAttachment(contentsOfFile: URL(fileURLWithPath: path))
							attachment.lifetime = .keepAlways
							activity.add(attachment)
						}
					}
				}

				switch verdict
				{
				case HarnessVerdictPass, HarnessVerdictSkip:
					break
				default:
					XCTFail("\(status)")
				}
			}
		}

		XCTAssertEqual(harness_exitCode(suite), 0, "the suite reported failures")
	}

	/// A failing ASSERT must reach the iOS error path: interpreterDidFail crosses the
	/// CoreWrapper bridge and LowResRMXError carries the message and the offending line.
	func testFailingAssertReachesTheCoreWrapperDelegate() throws
	{
		final class Recorder: CoreWrapperDelegate
		{
			var failures: [CoreError] = []

			func coreInterpreterDidFail(coreError: CoreError)
			{
				failures.append(coreError)
			}

			func coreDiskDriveWillAccess(diskDataManager _: UnsafeMutablePointer<DataManager>?) -> Bool
			{
				true
			}

			func coreDiskDriveDidSave(diskDataManager _: UnsafeMutablePointer<DataManager>?) {}
			func coreDiskDriveIsFull(diskDataManager _: UnsafeMutablePointer<DataManager>?) {}
			func coreControlsDidChange(controlsInfo _: ControlsInfo) {}
			func persistentRamWillAccess(destination _: UnsafeMutablePointer<UInt8>?, size _: Int32) {}
			func persistentRamDidChange(_: Data) {}
		}

		let source = "assert 1=1\nassert 1=2\nend\n"
		let recorder = Recorder()
		let wrapper = CoreWrapper()
		wrapper.delegate = recorder

		XCTAssertNil(wrapper.compileProgram(sourceCode: source), "the program must compile")

		wrapper.input.width = 216
		wrapper.input.height = 384
		core_willRunProgram(&wrapper.core, 0)
		var frames = 0
		while frames < 8, recorder.failures.isEmpty
		{
			core_update(&wrapper.core, &wrapper.input)
			frames += 1
		}

		XCTAssertEqual(recorder.failures.count, 1, "no failure reached the delegate")
		let error = try XCTUnwrap(recorder.failures.first)
		XCTAssertEqual(error.code, ErrorAssertionFailed)
		XCTAssertEqual(wrapper.core.interpreter.pointee.numAssertions, 1, "the passing ASSERT was not counted")

		let wrapped = LowResRMXError(error: error, sourceCode: source)
		XCTAssertEqual(wrapped.message, "Assertion Failed")
		XCTAssertEqual(wrapped.line, "assert 1=2")
	}

	/// SYSTEM 9 must reach the host through ControlsInfo, which is the only way the view
	/// controller learns it has to lock the orientation.
	func testSystemNineLocksPortraitThroughControlsInfo()
	{
		final class Recorder: CoreWrapperDelegate
		{
			var controls: [ControlsInfo] = []

			func coreInterpreterDidFail(coreError: CoreError)
			{
				XCTFail("unexpected error \(coreError.code)")
			}

			func coreDiskDriveWillAccess(diskDataManager _: UnsafeMutablePointer<DataManager>?) -> Bool
			{
				true
			}

			func coreDiskDriveDidSave(diskDataManager _: UnsafeMutablePointer<DataManager>?) {}
			func coreDiskDriveIsFull(diskDataManager _: UnsafeMutablePointer<DataManager>?) {}
			func coreControlsDidChange(controlsInfo: ControlsInfo)
			{
				controls.append(controlsInfo)
			}

			func persistentRamWillAccess(destination _: UnsafeMutablePointer<UInt8>?, size _: Int32) {}
			func persistentRamDidChange(_: Data) {}
		}

		let recorder = Recorder()
		let wrapper = CoreWrapper()
		wrapper.delegate = recorder

		XCTAssertNil(wrapper.compileProgram(sourceCode: "system 9,1\nwait 2\nend\n"), "the program must compile")

		wrapper.input.width = 216
		wrapper.input.height = 384
		core_willRunProgram(&wrapper.core, 0)
		for _ in 0 ..< 6
		{
			core_update(&wrapper.core, &wrapper.input)
		}

		XCTAssertTrue(wrapper.core.interpreter.pointee.lockPortrait, "the interpreter did not lock portrait")
		XCTAssertTrue(recorder.controls.contains { $0.isPortraitLocked }, "no ControlsInfo reported the lock")
	}
}
