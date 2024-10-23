import 'dart:ui' as ui;

import 'package:flutter/material.dart';

import 'package:lowresrmx/core/runtime.dart';

// final debugPaint = Paint()
//   ..color = const Color.fromRGBO(0, 255, 0, 1.0)
//   ..strokeWidth = 5
//   ..style = PaintingStyle.stroke;

class MyScreenPainter extends CustomPainter {
  ui.Image? image;

  MyScreenPainter(this.image);

  @override
  void paint(Canvas canvas, Size size) {
    if (image != null) {
      canvas.drawImage(image!, Offset.zero, Paint());
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
	final ComPort comPort;

  const MyScreenPaint({
		required this.comPort,
    super.key,
  });

  @override
  State<MyScreenPaint> createState() => _MyScreenPaintState();
}

class _MyScreenPaintState extends State<MyScreenPaint> {
  ui.Image? image;

  @override
  initState() {
    super.initState();
		// Start updating the image.
		widget.comPort.onImage = (image) {
			if (mounted) {
				setState(() {
					this.image = image;
				});
			}
		};

    // TODO: allow to show the keyboard, put it somewhere else
    // SystemChannels.textInput.invokeMethod("TextInput.show");
  }

	@override
	void dispose() {
		// Stop updating the image.
		widget.comPort.onImage = null;
		super.dispose();
	}

  @override
  Widget build(BuildContext context) {
    return CustomPaint(
      size:
          Size(Runtime.screenWidth.toDouble(), Runtime.screenHeight.toDouble()),
      painter: MyScreenPainter(image), //context.watch<Runtime>()),
    );
  }
}
