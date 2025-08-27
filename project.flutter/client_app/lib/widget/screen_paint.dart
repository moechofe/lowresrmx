import 'dart:developer';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';

import 'package:lowresrmx/core/runtime.dart';

// final debugPaint = Paint()
//   ..color = const Color.fromRGBO(0, 255, 0, 1.0)
//   ..strokeWidth = 5
//   ..style = PaintingStyle.stroke;

class MyScreenPainter extends CustomPainter {
	final ValueNotifier<ui.Image?> imageNotifier;
  // ui.Image? image;

  MyScreenPainter(this.imageNotifier) : super(repaint: imageNotifier);

  @override
  void paint(Canvas canvas, Size size) {
		if (imageNotifier.value != null) {
    // if (image != null) {
      canvas.drawImage(imageNotifier.value!, Offset.zero, Paint());
    }

    // canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height),
    //     Paint()..color = const Color.fromRGBO(255, 255, 255, 0.8));
  }

  @override
  bool shouldRepaint(covariant MyScreenPainter oldDelegate) {
		return true;
  }
}

class MyScreenPaint extends StatefulWidget {
	// final ComPort comPort;
	final ValueNotifier<ui.Image?> imageNotifier;

  const MyScreenPaint({
		required this.imageNotifier,
		// required this.comPort,
    super.key,
  });

  @override
  State<MyScreenPaint> createState() => _MyScreenPaintState();
}

class _MyScreenPaintState extends State<MyScreenPaint> {
  ui.Image? image;

  @override
  Widget build(BuildContext context) {
		log("MyScreenPaint.build() Not good if called multiple times.");
    return CustomPaint(
      size:
          Size(Runtime.screenWidth.toDouble(), Runtime.screenHeight.toDouble()),
      painter: MyScreenPainter(widget.imageNotifier), //context.watch<Runtime>()),
    );
  }
}
