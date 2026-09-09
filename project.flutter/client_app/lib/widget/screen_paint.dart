import 'dart:developer';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
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

    if (comport.textureId != null) {
      // The surface is already device-resolution and cover-scaled by screenBlitScaled(), so the
      // texture is drawn 1:1 over the page and nothing can filter it.
      return Texture(
          textureId: comport.textureId!, filterQuality: FilterQuality.none);
    }
    return FittedBox(
      fit: BoxFit.cover,
      alignment: Alignment.topLeft,
      child: CustomPaint(
        size: Size(Runtime.screenWidth.toDouble(), Runtime.screenHeight.toDouble()),
        painter: MyScreenPainter(widget.imageNotifier),
      ),
    );
  }
}
