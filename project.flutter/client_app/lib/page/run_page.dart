import 'dart:async';
import 'dart:developer' show log;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/widget/screen_paint.dart';

class MyRunPage extends StatefulWidget {
  final Runtime runtime;

  const MyRunPage(this.runtime, {super.key});

  @override
  State<MyRunPage> createState() => _MyAppPageState();
}

class _MyAppPageState extends State<MyRunPage> {
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
    return LayoutBuilder(
      builder: (context, constraints) {
        // log("constraints: $constraints");
        log("MyAppPage.build()");
        final EdgeInsets safeArea = MediaQuery.of(context).padding;
        widget.runtime.resize(constraints.maxWidth, constraints.maxHeight,
            safeArea.top, safeArea.left, safeArea.bottom, safeArea.right);
        return Focus(
          autofocus: true,
          // canRequestFocus: true,
          onKeyEvent: (node, event) {
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
            child: const FittedBox(
                fit: BoxFit.cover,
                alignment: Alignment.topLeft,
                child: MyScreenPaint()),
          ),
        );
      },
    );
  }
}
