import 'dart:developer';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:provider/provider.dart';

class MyScreenPainter extends CustomPainter {
  final ValueNotifier<ui.Image?> imageNotifier;

  MyScreenPainter(this.imageNotifier) : super(repaint: imageNotifier);

  @override
  void paint(Canvas canvas, Size size) {
    if (imageNotifier.value != null) {
      canvas.drawImage(imageNotifier.value!, Offset.zero, Paint()..filterQuality = FilterQuality.none);
    }
  }

  @override
  bool shouldRepaint(covariant MyScreenPainter oldDelegate) {
    return true;
  }
}

class MyTextureWidget extends LeafRenderObjectWidget {
  final int textureId;
  final Size size;
  final FilterQuality filterQuality;

  const MyTextureWidget({
    required this.textureId,
    required this.size,
    this.filterQuality = FilterQuality.none,
    super.key,
  });

  @override
  RenderObject createRenderObject(BuildContext context) {
    return MyTextureRenderBox(
      textureId: textureId,
      textureSize: size,
      filterQuality: filterQuality,
      devicePixelRatio: MediaQuery.of(context).devicePixelRatio,
    );
  }

  @override
  void updateRenderObject(BuildContext context, MyTextureRenderBox renderObject) {
    renderObject
      ..textureId = textureId
      ..textureSize = size
      ..filterQuality = filterQuality
      ..devicePixelRatio = MediaQuery.of(context).devicePixelRatio;
  }
}

class MyTextureRenderBox extends RenderBox {
  int _textureId;
  Size _textureSize;
  FilterQuality _filterQuality;
  double _devicePixelRatio;

  MyTextureRenderBox({
    required int textureId,
    required Size textureSize,
    required FilterQuality filterQuality,
    required double devicePixelRatio,
  })  : _textureId = textureId,
        _textureSize = textureSize,
        _filterQuality = filterQuality,
        _devicePixelRatio = devicePixelRatio;

  set textureId(int value) {
    if (_textureId != value) {
      _textureId = value;
      markNeedsPaint();
    }
  }

  set textureSize(Size value) {
    if (_textureSize != value) {
      _textureSize = value;
      markNeedsLayout();
    }
  }

  set filterQuality(FilterQuality value) {
    if (_filterQuality != value) {
      _filterQuality = value;
      markNeedsPaint();
    }
  }

  set devicePixelRatio(double value) {
    if (_devicePixelRatio != value) {
      _devicePixelRatio = value;
      markNeedsPaint();
    }
  }

  @override
  bool get alwaysNeedsCompositing => true;

  @override
  void performLayout() {
    size = constraints.biggest;
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    final double scaleX = size.width / _textureSize.width;
    final double scaleY = size.height / _textureSize.height;
    final double scale = scaleX > scaleY ? scaleX : scaleY;

    final double drawWidth = _textureSize.width * scale;
    final double drawHeight = _textureSize.height * scale;

    // Use integer rounding to align with physical pixels
    final double x = (offset.dx * _devicePixelRatio).roundToDouble() / _devicePixelRatio;
    final double y = (offset.dy * _devicePixelRatio).roundToDouble() / _devicePixelRatio;
    final double w = (drawWidth * _devicePixelRatio).roundToDouble() / _devicePixelRatio;
    final double h = (drawHeight * _devicePixelRatio).roundToDouble() / _devicePixelRatio;

    context.addLayer(TextureLayer(
      rect: Rect.fromLTWH(x, y, w, h),
      textureId: _textureId,
      filterQuality: _filterQuality,
    ));
  }
}

class MyScreenPaint extends StatefulWidget {
  final ValueNotifier<ui.Image?> imageNotifier;

  const MyScreenPaint({
    required this.imageNotifier,
    super.key,
  });

  @override
  State<MyScreenPaint> createState() => _MyScreenPaintState();
}

class _MyScreenPaintState extends State<MyScreenPaint> {
  @override
  Widget build(BuildContext context) {
    log("MyScreenPaint.build()");
    final comport = context.watch<ComPort>();

		// return Placeholder();

		// return Texture(textureId: comport.textureId!, filterQuality: FilterQuality.none);

		// return AspectRatio(
		// 	aspectRatio: 216/384,
		// 	child:
		// 	Texture(textureId: comport.textureId!, filterQuality: FilterQuality.none)
		// );

    return LayoutBuilder(builder: (context, constraints) {
      final double screenWidth = Runtime.screenWidth.toDouble();
      final double screenHeight = Runtime.screenHeight.toDouble();

      if (comport.textureId != null) {
        return MyTextureWidget(
          textureId: comport.textureId!,
          size: Size(screenWidth, screenHeight),
          filterQuality: FilterQuality.none,
        );
      } else {
        return FittedBox(
          fit: BoxFit.cover,
          alignment: Alignment.topLeft,
          child: CustomPaint(
            size: Size(screenWidth, screenHeight),
            painter: MyScreenPainter(widget.imageNotifier),
          ),
        );
      }
    });
  }
}
