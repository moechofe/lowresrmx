//
//  IndexSideBar.swift
//  LowRes NX Coder
//
//  Created by Timo Kloss on 17/3/18.
//  Copyright © 2018 Inutilis Software. All rights reserved.
//

import UIKit

class IndexMarker: NSObject {
    let label: String
    let line: Int
    let range: NSRange
    var currentBarY: CGFloat = 0.0
    
    init(label: String, line: Int, range: NSRange) {
        self.label = label
        self.line = line
        self.range = range
    }
}

class IndexSideBar: UIControl {
    
    static let margin: CGFloat = 3.0
    
    weak var textView: UITextView!
    var shouldUpdateOnTouch = false
    
    private var numLines: Int = 0
    private var markers: [IndexMarker]?
    private var oldMarker: IndexMarker?
    private var highlight: UIView!
    private var labels: [GORLabel]?
    private var startTouchY: CGFloat = 0.0
    
    override func awakeFromNib() {
        super.awakeFromNib()
        
//        backgroundColor = AppStyle.mediumGrayColor()
        alpha = 0.5;
        
        highlight = UIView()
//        highlight.backgroundColor = AppStyle.brightTintColor()
        highlight.alpha = 0.25
    }
    
    override func draw(_ rect: CGRect) {
        let context = UIGraphicsGetCurrentContext()
        
        let width = bounds.size.width
//        let rectColor = AppStyle.brightTintColor()
        
        var markRect = CGRect(x: IndexSideBar.margin, y: 0.0, width: width - 2 * IndexSideBar.margin, height: 2.0)
//        context?.setFillColor(rectColor.cgColor)
        
        if let markers = markers {
            for marker in markers {
                markRect.origin.y = floor(marker.currentBarY)
                context?.fill(markRect)
            }
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        updateBarPositions()
    }
    
    private func updateBarPositions() {
        if let markers = markers {
            let height = bounds.size.height - 2.0 - 2 * IndexSideBar.margin
            for marker in markers {
                marker.currentBarY = IndexSideBar.margin + CGFloat(marker.line) * height / CGFloat(numLines)
            }
        }
        setNeedsDisplay()
    }
    
    func update() {
        guard let text = textView.text else {
            return
        }
        
        let regex = try! NSRegularExpression(pattern: "\\A\\s*(\\S+?:\\s*|#\\d+?:.*?|SUB\\s.+?)\\Z", options: .caseInsensitive)
        
        var markers = [IndexMarker]()
        var numLines = 0
        
        text.enumerateSubstrings(in: text.startIndex..<text.endIndex, options: .byLines) { (string, substringRange, enclosingRange, stop) in
            if let string = string {
                let numMatches = regex.numberOfMatches(in: string, options: [], range: NSRange(location: 0, length: string.utf16.count))
                if numMatches > 0 {
                    let range = NSRange(enclosingRange, in: text)
                    markers.append(IndexMarker(label: string, line: numLines, range: range))
                }
            }
            numLines += 1
        }
        
        self.numLines = numLines
        self.markers = markers
        self.shouldUpdateOnTouch = false
        updateBarPositions()
    }
    
    override func beginTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        if shouldUpdateOnTouch {
            update()
        }
        showLabels()
        let point = touch.location(in: self)
        startTouchY = point.y
        return true
    }
    
    override func continueTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        let point = touch.location(in: self)
        touchedAt(y: point.y)
        return true
    }
    
    override func endTracking(_ touch: UITouch?, with event: UIEvent?) {
        oldMarker = nil
        unhighlight()
        hideLabels()
    }
    
    override func cancelTracking(with event: UIEvent?) {
        oldMarker = nil
        unhighlight()
        hideLabels()
    }
    
    private func touchedAt(y touchY: CGFloat) {
        if abs(touchY - startTouchY) < 10.0 {
            // not moved enough
            return
        } else {
            // unblock scrolling
            startTouchY = -100.0
        }
        
        var bestMarker: IndexMarker?
        var bestDist = bounds.size.height
        var dist: CGFloat = 0.0
        if let markers = markers {
            for marker in markers {
                dist = abs(marker.currentBarY - touchY)
                if dist < 22.0 && (bestMarker == nil || dist < bestDist) {
                    bestMarker = marker
                    bestDist = dist
                }
            }
        }
        
        let visibleHeight = textView.bounds.size.height - textView.contentInset.bottom
        let maxOffset = max(0.0, textView.contentSize.height - visibleHeight)
        
        var scrollCenterY: CGFloat = -1.0
        if let bestMarker = bestMarker {
            if bestMarker != oldMarker {
                var rect = textView.layoutManager.boundingRect(forGlyphRange: bestMarker.range, in: textView.textContainer)
                rect.origin.y += textView.textContainerInset.top
                scrollCenterY = rect.origin.y + rect.size.height * 0.5
                
                rect.size.width -= 22.0
                highlight.frame = rect
                if highlight.superview == nil {
                    textView.addSubview(highlight)
                }
                
                oldMarker = bestMarker
            }
        } else {
            unhighlight()
            oldMarker = nil
            
            var factor = (touchY - 22.0) / (bounds.size.height - 44.0)
            if factor < 0.0 { factor = 0.0 }
            if factor > 1.0 { factor = 1.0 }
            
            scrollCenterY = factor * textView.contentSize.height
        }
        
        if scrollCenterY != -1.0 {
            textView.setContentOffset(CGPoint(x: 0, y: max(min(floor(scrollCenterY - visibleHeight * 0.5), maxOffset), 0.0)), animated: false)
        }
    }
    
    private func unhighlight() {
        if highlight.superview != nil {
            highlight.removeFromSuperview()
        }
    }
    
    private func showLabels() {
        labels = []
        
        guard let markers = markers, let superview = superview else {
            return
        }
        
        var lastBottom: CGFloat = 2.0
        var lastX: CGFloat = 0.0
        var lastY: CGFloat = 0.0
        for marker in markers {
            let label = GORLabel()
            label.isUserInteractionEnabled = false
//            label.backgroundColor = AppStyle.whiteColor()
            label.insets = UIEdgeInsets(top: 0, left: -4.0, bottom: 0, right: -4.0)
            label.layer.cornerRadius = 4.0
            label.clipsToBounds = true
//            label.textColor = AppStyle.darkGrayColor()
            label.font = UIFont.systemFont(ofSize: 11)
            label.textAlignment = .center
            label.text = marker.label
            label.sizeToFit()
            
            var frame = label.frame
            var isFirstInLine = false
            if marker.currentBarY > lastBottom {
                frame.origin.x = ceil(-frame.size.width - 24.0)
                frame.origin.y = round(max(marker.currentBarY - frame.size.height * 0.5, lastBottom))
                isFirstInLine = true
            } else {
                frame.origin.x = floor(lastX - frame.size.width)
                frame.origin.y = round(lastY)
            }
            
            label.frame = superview.convert(frame, from: self)
            lastX = frame.origin.x - 1.0
            lastY = frame.origin.y
            lastBottom = frame.origin.y + frame.size.height + 1.0
            
            if isFirstInLine || label.frame.origin.x >= 100.0 {
                superview.addSubview(label)
                labels!.append(label)
            }
        }
    }
    
    private func hideLabels() {
        if let labels = labels {
            for label in labels {
                label.removeFromSuperview()
            }
        }
        labels = nil
    }
    
}
