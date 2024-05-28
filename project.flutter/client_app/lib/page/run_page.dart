import 'dart:async';
import 'dart:developer' show log;

import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:lowresrmx/data/library.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/widget/screen_paint.dart';

enum MyRunMenuOption {
  thumbnail,
}

class MyRunPage extends StatefulWidget {
  final Runtime runtime;

  final String _programName;
  String get programName => _programName;

  const MyRunPage(
      {required this.runtime, required String programName, super.key})
      : _programName = programName;

  @override
  State<MyRunPage> createState() => _MyRunPageState();
}

class _MyRunPageState extends State<MyRunPage> {
  late Timer updater;

  tickUpdate() {
    widget.runtime.update();
    widget.runtime.render();
  }

  @override
  void initState() {
    super.initState();
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.immersive);
    updater = Timer.periodic(
        const Duration(microseconds: 1e6 ~/ 60), (_) => tickUpdate());
  }

  @override
  void dispose() {
    super.dispose();
    updater.cancel();
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual,
        overlays: SystemUiOverlay.values);
  }

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      // log("constraints: $constraints");
      log("MyAppPage.build()");
      final EdgeInsets safeArea = MediaQuery.of(context).padding;
      widget.runtime.resize(constraints.maxWidth, constraints.maxHeight,
          safeArea.top, safeArea.left, safeArea.bottom, safeArea.right);
      // Capture simple touch events
      return Stack(
        children: [
          RawGestureDetector(
            gestures: {
              TapAndPanGestureRecognizer: GestureRecognizerFactoryWithHandlers<
                  TapAndPanGestureRecognizer>(
                () => TapAndPanGestureRecognizer(),
                (instance) {
                  instance
                    ..onTapDown = (details) {
                      context.read<Runtime>().touchOn(details.localPosition);
                    }
                    ..onTapUp = (details) {
                      context.read<Runtime>().touchOff();
                    }
                    ..onDragStart = (details) {
                      context.read<Runtime>().touchOn(details.localPosition);
                    }
                    ..onDragUpdate = (details) {
                      context.read<Runtime>().touchOn(details.localPosition);
                    }
                    ..onDragEnd = (details) {
                      context.read<Runtime>().touchOff();
                    };
                },
              ),
            },
            // Capture keyboard events
            child: Focus(
              autofocus: true,
              onKeyEvent: (node, event) {
                log("RunPage.KeyEvent: $event");
                if (event is KeyDownEvent &&
                    event.logicalKey == LogicalKeyboardKey.escape) {
                  Navigator.of(context).pop();
                  return KeyEventResult.handled;
                } else {
                  return KeyEventResult.ignored;
                }
              },
              child: SizedBox(
                width: constraints.maxWidth,
                height: constraints.maxHeight,
                // Make the screen scaled and aligned to the top left.
                child: const FittedBox(
                    fit: BoxFit.cover,
                    alignment: Alignment.topLeft,
                    child: MyScreenPaint()),
              ),
            ),
          ),
          Align(
            alignment: Alignment.topRight,
            child: Padding(
              padding: const EdgeInsets.fromLTRB(0.0, 8.0, 32.0, 0.0),
              child: PopupMenuButton<MyRunMenuOption>(
                itemBuilder: (BuildContext context) => [
                  const PopupMenuItem(
                    value: MyRunMenuOption.thumbnail,
                    child: Text("Save thumbnail"),
                  ),
                ],
                onSelected: (value) async {
                  final Runtime runtime = context.read<Runtime>();
                  switch (value) {
                    case MyRunMenuOption.thumbnail:
											runtime.renderThumbnail(widget.programName);
											break;
                  }
                  // Handle selected option
                },
              ),
            ),
          )
        ],
      );
    });
  }
}
