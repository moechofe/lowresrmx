import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:lowresrmx/data/editor_errors.dart';

import 'package:re_editor/re_editor.dart';

class MyErrorGutterRenderObject extends RenderBox {
  CodeLineEditingController _controller;
  CodeIndicatorValueNotifier _notifier;
  MyEditorErrors _errors;
  TextStyle _textStyle;
  TextStyle _errorStyle;

  final TextPainter _textPainter;

  MyErrorGutterRenderObject({
    required CodeLineEditingController controller,
    required CodeIndicatorValueNotifier notifier,
    required MyEditorErrors errors,
    required TextStyle textStyle,
    required TextStyle errorStyle,
  })  : _controller = controller,
        _notifier = notifier,
        _errors = errors,
        _textStyle = textStyle,
        _errorStyle = errorStyle,
        _textPainter = TextPainter(
          textDirection: TextDirection.ltr,
        );

  set controller(CodeLineEditingController value) {
    if (_controller == value) {
      return;
    }
    if (attached) {
      _controller.removeListener(markNeedsPaint);
    }
    _controller = value;
    if (attached) {
      _controller.addListener(markNeedsPaint);
    }
    markNeedsPaint();
  }

  set notifier(CodeIndicatorValueNotifier value) {
    if (_notifier == value) {
      return;
    }
    if (attached) {
      _notifier.removeListener(markNeedsPaint);
    }
    _notifier = value;
    if (attached) {
      _notifier.addListener(markNeedsPaint);
    }
    markNeedsPaint();
  }

  set errors(MyEditorErrors value) {
    if (_errors == value) {
      return;
    }
    if (attached) {
      _errors.removeListener(markNeedsPaint);
    }
    _errors = value;
    if (attached) {
      _errors.addListener(markNeedsPaint);
    }
    markNeedsPaint();
  }

  set textStyle(TextStyle value) {
    if (_textStyle == value) {
      return;
    }
    _textStyle = value;
    markNeedsLayout();
  }

  set errorStyle(TextStyle value) {
    if (_errorStyle == value) {
      return;
    }
    _errorStyle = value;
    markNeedsLayout();
  }

  @override
  bool hitTestSelf(Offset position) => true;

  @override
  void handleEvent(PointerEvent event, BoxHitTestEntry entry) {
    if (event is PointerDownEvent) {
      final Offset position = globalToLocal(event.position);
      final CodeLineRenderParagraph? paragraph =
          _findParagraphByPosition(position);
      if (paragraph != null) {
        _controller.selectLine(paragraph.index);
        if (paragraph.index == _errors.markedRow) {
          _errors.pinMarked(_controller.text.hashCode);
        }
      }
    }
    super.handleEvent(event, entry);
  }

  @override
  void attach(covariant PipelineOwner owner) {
    _controller.addListener(markNeedsPaint);
    _notifier.addListener(markNeedsPaint);
    _errors.addListener(markNeedsPaint);
    super.attach(owner);
  }

  @override
  void detach() {
    _controller.removeListener(markNeedsLayout);
    _notifier.removeListener(markNeedsPaint);
    _errors.removeListener(markNeedsPaint);
    super.detach();
  }

  @override
  void performLayout() {
    assert(
        constraints.maxHeight > 0 && constraints.maxHeight != double.infinity,
        'CodeLineNumber should have an explicit height.');
    _textPainter.text = TextSpan(
      text: '⚠',
      style: _textStyle,
    );
    _textPainter.layout();
    size = Size(_textPainter.width, constraints.maxHeight);
    debugPrint("MyErrorGutterRenderObject size: $size");
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    final Canvas canvas = context.canvas;
    final CodeIndicatorValue? value = _notifier.value;
    if (value == null || value.paragraphs.isEmpty) {
      return;
    }
    canvas.save();
    canvas
        .clipRect(Rect.fromLTWH(offset.dx, offset.dy, size.width, size.height));
    for (final CodeLineRenderParagraph paragraph in value.paragraphs) {
      if (paragraph.index == _errors.markedRow) {
        _textPainter.text = TextSpan(text: "🐞", style: _errorStyle);
      } else {
        _textPainter.text = TextSpan(text: "󠁪", style: _textStyle);
      }
      _textPainter.layout();
      _textPainter.paint(
          canvas,
          Offset(offset.dx + size.width - _textPainter.width,
              offset.dy + paragraph.offset.dy));
    }
    canvas.restore();
  }

  CodeLineRenderParagraph? _findParagraphByPosition(Offset position) {
    final int? index = _notifier.value?.paragraphs
        .indexWhere((e) => position.dy > e.top && position.dy < e.bottom);
    if (index == null || index < 0) {
      return null;
    }
    return _notifier.value?.paragraphs[index];
  }
}

class MyErrorGutter extends LeafRenderObjectWidget {
  final CodeLineEditingController controller;
  final CodeIndicatorValueNotifier notifier;
  final MyEditorErrors errors;
  final TextStyle textStyle;
  final TextStyle errorStyle;

  const MyErrorGutter({
    super.key,
    required this.controller,
    required this.notifier,
    required this.errors,
    required this.textStyle,
    required this.errorStyle,
  });

  @override
  RenderObject createRenderObject(BuildContext context) =>
      MyErrorGutterRenderObject(
          controller: controller,
          notifier: notifier,
          errors: errors,
          textStyle: textStyle,
          errorStyle: errorStyle);

  @override
  void updateRenderObject(
      BuildContext context, covariant MyErrorGutterRenderObject renderObject) {
    renderObject
      ..controller = controller
      ..notifier = notifier
      ..errors = errors
      ..textStyle = textStyle
      ..errorStyle = errorStyle;
    super.updateRenderObject(context, renderObject);
  }
}
