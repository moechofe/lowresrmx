import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/core/runtime.dart';

// final debugPaint = Paint()
//   ..color = const Color.fromRGBO(0, 255, 0, 1.0)
//   ..strokeWidth = 5
//   ..style = PaintingStyle.stroke;

class MyScreenPainter extends CustomPainter {
  final Runtime runtime;
  final ui.Image? image;

  MyScreenPainter(this.runtime) : image = runtime.image;

  @override
  void paint(Canvas canvas, Size size) {
    if (image != null) {
      canvas.drawImage(image!, Offset.zero, Paint());
    }

    // canvas.drawRect(Rect.fromLTWH(0,0, size.width,size.height), Paint()..color=Color.fromRGBO(255, 255, 255, 0.8));
  }

  @override
  bool shouldRepaint(covariant MyScreenPainter oldDelegate) {
    return image != oldDelegate.image;
  }
}

class MyScreenPaint extends StatefulWidget {
  const MyScreenPaint({
    super.key,
  });

  @override
  State<MyScreenPaint> createState() => _MyScreenPaintState();
}

class _MyScreenPaintState extends State<MyScreenPaint> {
  @override
  initState() {
    super.initState();
    SystemChannels.textInput.invokeMethod("TextInput.show");
  }

  @override
  Widget build(BuildContext context) {
    // log("ScreenPaint.build()");
    return CustomPaint(
        size: Size(Runtime.screenWidth.toDouble(), Runtime.screenHeight.toDouble()),
        painter: MyScreenPainter(context.watch<Runtime>()),
        // child: SizedBox(
        //     width: Runtime.imageWidth.toDouble(),
        //     height: Runtime.imageHeight.toDouble(),
        //     child: Align(
        //         alignment: Alignment.topRight,
        //         child: Padding(
				// 					padding: const EdgeInsets.all(2.0),
				// 					child: SizedBox(

				// 						width: 18,
				// 						height: 18,
				// 						child: IconButton(
				// 								iconSize: 12,
				// 																padding: EdgeInsets.zero,
				// 								icon: const Icon(Icons.more_vert_rounded),
				// 								onPressed: () {}),
				// 					),
				// 				)))

        // // child: Text("test"),
        );
  }
}
