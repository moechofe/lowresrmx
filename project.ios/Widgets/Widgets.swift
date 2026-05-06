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
import SwiftUI
import WidgetKit

struct ProgramEntry: TimelineEntry
{
	let date: Date
	let model: ProgramModel?
	let image: UIImage?
}

var currentEntry: ProgramEntry?

struct Provider: TimelineProvider
{
	func placeholder(in _: Context) -> ProgramEntry
	{
		currentEntry ?? ProgramEntry(date: Date(), model: nil, image: nil)
	}

	func getSnapshot(in context: Context, completion: @escaping (ProgramEntry) -> Void)
	{
		if context.isPreview, let currentEntry
		{
			completion(currentEntry)
		}
		else
		{
			APIClient.shared.fetchProgramOfTheDay
			{ result in
				switch result
				{
				case let .success(model):
					let image = loadImage(urlString: model.image)
					currentEntry = ProgramEntry(date: Date(), model: model, image: image)
				case let .failure(error):
					print(error.localizedDescription)
				}
				completion(currentEntry ?? ProgramEntry(date: Date(), model: nil, image: nil))
			}
		}
	}

	func getTimeline(in _: Context, completion: @escaping (Timeline<ProgramEntry>) -> Void)
	{
		APIClient.shared.fetchProgramOfTheDay
		{ result in
			let updateAfter: Date
			switch result
			{
			case let .success(model):
				let image = loadImage(urlString: model.image)
				currentEntry = ProgramEntry(date: Date(), model: model, image: image)
				updateAfter = Calendar.current.startOfDay(for: Date() + 24 * 60 * 60)
			case let .failure(error):
				print(error.localizedDescription)
				updateAfter = Date() + 60 * 60
			}
			let timeline = Timeline(
				entries: [currentEntry ?? ProgramEntry(date: Date(), model: nil, image: nil)],
				policy: .after(updateAfter)
			)
			completion(timeline)
		}
	}

	private func loadImage(urlString: String) -> UIImage?
	{
		guard let url = URL(string: urlString) else { return nil }
		do
		{
			let data = try Data(contentsOf: url)
			return UIImage(data: data)
		}
		catch
		{
			return nil
		}
	}
}

struct WidgetsEntryView: View
{
	var entry: Provider.Entry

	var body: some View
	{
		ZStack(alignment: .bottom)
		{
			Image(uiImage: entry.image ?? UIImage())
				.resizable()
				.scaledToFill()
				.background(Color(.sRGB, red: 0, green: 0.75, blue: 0.75, opacity: 1.0))
				.frame(minWidth: 0, maxWidth: .infinity, minHeight: 0, maxHeight: .infinity, alignment: .center)
			LinearGradient(gradient: Gradient(colors: [Color(white: 0, opacity: 0), Color(white: 0, opacity: 0.5)]), startPoint: .top, endPoint: .bottom)
				.frame(height: 50.0)
			VStack
			{
				Text("Program of the Day")
					.font(.caption)
				Text(entry.model?.title ?? "Unknown")
					.font(.headline)
					.multilineTextAlignment(.center)
					.lineLimit(1)
					.padding([.leading, .bottom, .trailing], 8.0)
			}
			.foregroundColor(Color.white)
			.shadow(color: .black, radius: 2, y: 1)
		}
		.widgetURL(entry.model?.appUrl ?? URL(string: "lowresrmx:")!)
	}
}

@main
struct Widgets: Widget
{
	let kind: String = "Widgets"

	var body: some WidgetConfiguration
	{
		StaticConfiguration(kind: kind, provider: Provider())
		{ entry in
			WidgetsEntryView(entry: entry)
		}
		.configurationDisplayName("Program of the Day")
		.description("Discover every day a program from the community.")
		.supportedFamilies([.systemSmall, .systemMedium])
	}
}

struct Widgets_Previews: PreviewProvider
{
	static var previews: some View
	{
		WidgetsEntryView(entry: ProgramEntry(date: Date(), model: nil, image: UIImage(named: "LowRes Galaxy 2")))
			.previewContext(WidgetPreviewContext(family: .systemSmall))
		WidgetsEntryView(entry: ProgramEntry(date: Date(), model: nil, image: UIImage(named: "LowRes Galaxy 2")))
			.previewContext(WidgetPreviewContext(family: .systemMedium))
	}
}
