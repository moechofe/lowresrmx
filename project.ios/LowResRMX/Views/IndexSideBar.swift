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

class IndexMarker: NSObject
{
	let label: String
	let line: Int
	let range: NSRange
	var currentBarY: CGFloat = 0.0

	init(label: String, line: Int, range: NSRange)
	{
		self.label = label
		self.line = line
		self.range = range
	}
}

class IndexSideBar: UIControl
{
	static let margin: CGFloat = 3.0

	weak var textView: UITextView!
	var shouldUpdateOnTouch = false

	private var numLines: Int = 0
	private var markers: [IndexMarker]?
	private var oldMarker: IndexMarker?
	private var highlight: UIView!
	private var labels: [GORLabel]?
	private var startTouchY: CGFloat = 0.0

	override func awakeFromNib()
	{
		super.awakeFromNib()
		alpha = 0.5
		highlight = UIView()
		highlight.alpha = 0.25
	}

	override func draw(_: CGRect)
	{
		let context = UIGraphicsGetCurrentContext()

		let width = bounds.size.width

		var markRect = CGRect(x: IndexSideBar.margin, y: 0.0, width: width - 2 * IndexSideBar.margin, height: 2.0)

		if let markers
		{
			for marker in markers
			{
				markRect.origin.y = floor(marker.currentBarY)
				context?.fill(markRect)
			}
		}
	}

	override func layoutSubviews()
	{
		super.layoutSubviews()
		updateBarPositions()
		if labels != nil
		{
			hideLabels()
			showLabels()
		}
	}

	private func updateBarPositions()
	{
		if let markers
		{
			let height = bounds.size.height - 2.0 - 2 * IndexSideBar.margin
			for marker in markers
			{
				marker.currentBarY = IndexSideBar.margin + CGFloat(marker.line) * height / CGFloat(numLines)
			}
		}
		setNeedsDisplay()
	}

	func update()
	{
		guard let text = textView.text
		else
		{
			return
		}

		let labelMode = AppController.shared.editorLabelIndexMode
		let procedureMode = AppController.shared.editorProcedureIndexMode
		let markerMode = AppController.shared.editorManualMarkerIndexMode

		if labelMode == .noLabels, procedureMode == .noProcedures, markerMode == .noMarkers
		{
			numLines = 0
			self.markers = []
			shouldUpdateOnTouch = false
			updateBarPositions()
			return
		}

		let nsText = text as NSString

		// Line pass. Separate from the span pass on purpose: the tokenizer stops
		// at the first "#", but the bar positions divide by the line count of the
		// *whole* document, ROM entries included.
		var lineRanges = [NSRange]()
		nsText.enumerateSubstrings(
			in: NSRange(location: 0, length: nsText.length),
			options: [.byLines, .substringNotRequired]
		)
		{ _, _, enclosingRange, _ in
			lineRanges.append(enclosingRange)
		}

		guard !lineRanges.isEmpty
		else
		{
			numLines = 0
			self.markers = []
			shouldUpdateOnTouch = false
			updateBarPositions()
			return
		}

		let spans = SyntaxWrapper.shared.synchronousSpans(for: text)

		var markers = [IndexMarker]()
		var line = 0

		for i in 0 ..< spans.count
		{
			let span = spans.span(at: i)

			guard let label = markerLabel(
				for: span, in: nsText,
				labelMode: labelMode, procedureMode: procedureMode, markerMode: markerMode
			)
			else
			{
				continue
			}

			// Spans and lines are both in ascending order, so this walks forward
			// rather than searching.
			while line < lineRanges.count - 1, NSMaxRange(lineRanges[line]) <= span.range.location
			{
				line += 1
			}

			markers.append(IndexMarker(label: label, line: line, range: lineRanges[line]))
		}

		numLines = lineRanges.count
		self.markers = markers
		shouldUpdateOnTouch = false
		updateBarPositions()
	}

	/// The text to show for a span, or nil if it does not belong in the index.
	private func markerLabel(
		for span: SourceSpan, in nsText: NSString,
		labelMode: LabelIndexMode, procedureMode: ProcedureIndexMode, markerMode: MarkerIndexMode
	) -> String?
	{
		// `enum SyntaxKind` is a plain C enum, so Swift imports it as a struct
		// rather than a Swift enum — compared with ==, not matched with `case`.
		if span.kind == SyntaxLabel
		{
			// References (GOTO/GOSUB/RESTORE targets) are not declarations.
			guard span.isDeclaration else { return nil }
			let name = nsText.substring(with: span.range)
			switch labelMode
			{
			case .allLabels:
				return name
			case .labelsWithoutUnderscore:
				return name.contains("_") ? nil : name
			default:
				return nil
			}
		}

		if span.kind == SyntaxSub
		{
			// CALL targets are not declarations.
			guard span.isDeclaration, procedureMode == .allProcedures else { return nil }
			return nsText.substring(with: span.range)
		}

		if span.kind == SyntaxComment
		{
			// Manual marker: '''NAME, labelled with the first word after the apostrophes ("'''TLB_0 backed first" indexes as "TLB_0").
			guard markerMode == .manualMarkers else { return nil }
			let comment = nsText.substring(with: span.range)
			guard comment.hasPrefix("'''") else { return nil }
			let name = comment.dropFirst(3).prefix { !$0.isWhitespace }
			return name.isEmpty ? nil : String(name)
		}

		return nil
	}

	override func beginTracking(_ touch: UITouch, with _: UIEvent?) -> Bool
	{
		textView.resignFirstResponder()
		// Force layout update if keyboard resignation caused changes
		superview?.layoutIfNeeded()

		if shouldUpdateOnTouch
		{
			update()
		}
		showLabels()
		let point = touch.location(in: self)
		startTouchY = point.y
		return true
	}

	override func continueTracking(_ touch: UITouch, with _: UIEvent?) -> Bool
	{
		let point = touch.location(in: self)
		touchedAt(y: point.y)
		return true
	}

	override func endTracking(_: UITouch?, with _: UIEvent?)
	{
		if let marker = oldMarker
		{
			textView.selectedRange = NSMakeRange(marker.range.location, 0)
		}
		oldMarker = nil
		unhighlight()
		hideLabels()
	}

	override func cancelTracking(with _: UIEvent?)
	{
		oldMarker = nil
		unhighlight()
		hideLabels()
	}

	private func touchedAt(y touchY: CGFloat)
	{
		if abs(touchY - startTouchY) < 10.0
		{
			// not moved enough
			return
		}
		else
		{
			// unblock scrolling
			startTouchY = -100.0
		}

		var bestMarker: IndexMarker?
		var bestDist = bounds.size.height
		var dist: CGFloat = 0.0
		if let markers
		{
			for marker in markers
			{
				dist = abs(marker.currentBarY - touchY)
				if dist < 22.0, bestMarker == nil || dist < bestDist
				{
					bestMarker = marker
					bestDist = dist
				}
			}
		}

		let visibleHeight = textView.bounds.size.height - textView.contentInset.bottom
		let maxOffset = max(0.0, textView.contentSize.height - visibleHeight)

		var scrollCenterY: CGFloat = -1.0
		if let bestMarker
		{
			if bestMarker != oldMarker
			{
				var rect = textView.layoutManager.boundingRect(forGlyphRange: bestMarker.range, in: textView.textContainer)
				rect.origin.y += textView.textContainerInset.top
				scrollCenterY = rect.origin.y + rect.size.height * 0.5

				rect.size.width -= 22.0
				highlight.frame = rect
				if highlight.superview == nil
				{
					textView.addSubview(highlight)
				}

				oldMarker = bestMarker
			}
		}
		else
		{
			unhighlight()
			oldMarker = nil

			var factor = (touchY - 22.0) / (bounds.size.height - 44.0)
			if factor < 0.0 { factor = 0.0 }
			if factor > 1.0 { factor = 1.0 }

			scrollCenterY = factor * textView.contentSize.height
		}

		if scrollCenterY != -1.0
		{
			textView.setContentOffset(CGPoint(x: 0, y: max(min(floor(scrollCenterY - visibleHeight * 0.5), maxOffset), 0.0)), animated: false)
		}
	}

	private func unhighlight()
	{
		if highlight.superview != nil
		{
			highlight.removeFromSuperview()
		}
	}

	private func showLabels()
	{
		labels = []

		guard let markers, let superview
		else
		{
			return
		}

		var lastBottom: CGFloat = 2.0
		var lastX: CGFloat = 0.0
		var lastY: CGFloat = 0.0
		for marker in markers
		{
			let label = GORLabel()
			label.isUserInteractionEnabled = false
			label.backgroundColor = UIColor.black
			label.insets = UIEdgeInsets(top: 0, left: -4.0, bottom: 0, right: -4.0)
			label.layer.cornerRadius = 4.0
			label.clipsToBounds = true
			label.textColor = UIColor.white
			label.font = UIFont.systemFont(ofSize: AppController.shared.editorFontSize * 0.7)
			label.textAlignment = .center
			label.text = marker.label
			label.sizeToFit()

			var frame = label.frame
			var isFirstInLine = false
			if marker.currentBarY > lastBottom
			{
				frame.origin.x = ceil(-frame.size.width - 24.0)
				frame.origin.y = round(max(marker.currentBarY - frame.size.height * 0.5, lastBottom))
				isFirstInLine = true
			}
			else
			{
				frame.origin.x = floor(lastX - frame.size.width)
				frame.origin.y = round(lastY)
			}

			label.frame = superview.convert(frame, from: self)
			lastX = frame.origin.x - 1.0
			lastY = frame.origin.y
			lastBottom = frame.origin.y + frame.size.height + 1.0

			if isFirstInLine || label.frame.origin.x >= 100.0
			{
				superview.addSubview(label)
				labels!.append(label)
			}
		}
	}

	private func hideLabels()
	{
		if let labels
		{
			for label in labels
			{
				label.removeFromSuperview()
			}
		}
		labels = nil
	}
}
