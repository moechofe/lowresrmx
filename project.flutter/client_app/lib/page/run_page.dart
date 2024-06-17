import 'dart:developer' show log;

import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/widget/screen_paint.dart';

enum MyRunMenuOption {
  trace,
  thumbnail,
}

class MyRunPage extends StatefulWidget {
  final ComPort comPort;
  final MyProgramPreference programPreference;

  final String _programName;
  String get programName => _programName;

  const MyRunPage(
      {required this.comPort,
      required String programName,
      required this.programPreference,
      super.key})
      : _programName = programName;

  @override
  State<MyRunPage> createState() => _MyRunPageState();
}

class _MyRunPageState extends State<MyRunPage> {
  @override
  void initState() {
    super.initState();
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.edgeToEdge);
		SystemChrome.setSystemUIOverlayStyle(const SystemUiOverlayStyle(
			statusBarColor: Colors.transparent,
			statusBarIconBrightness: Brightness.dark,
			systemNavigationBarColor: Colors.transparent,
			systemNavigationBarIconBrightness: Brightness.dark,
		));
		widget.comPort.onRunningError = (error) {
			widget.comPort.onRunningError = null;
			widget.comPort.stop();
			log("RunPage.Error: $error");
			// _gotoPrevious(context);
			_reportError(error);
		};
		widget.comPort.onThumbnail = (image) {
			MyLibrary.writeThumbnail(widget.programName, image);
		};
    WidgetsBinding.instance.addPostFrameCallback((_) {
      widget.comPort.start();
    });
  }

  @override
  void dispose() {
    widget.comPort.stop();
		widget.comPort.onThumbnail = null;
		widget.comPort.onRunningError = null;
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual,
        overlays: SystemUiOverlay.values);
    super.dispose();
  }

	void _reportError(Error err)
	{
		showDialog(
			context: context,
			builder: (context) {
				return AlertDialog(
					icon: const Icon(Icons.error_rounded),
					title: const Text("Error"),
					content: Text(err.msg),
					actions: [
						TextButton(
							child: const Text("Go to"),
							onPressed: () {
								Navigator.of(context).pop();
								_gotoError(context, err);
							}
						)
					]
				);
			}
		);
	}

  /// Will go back to the previous page.
  void _gotoPrevious(BuildContext context) {
    Navigator.of(context).pushReplacement(MaterialPageRoute(
        builder: (_) => MyEditPage(programName: widget.programName)));
  }

	void _gotoError(BuildContext context, Error error) {
		Navigator.of(context).pushReplacement(MaterialPageRoute(
			builder: (_) => MyEditPage.withError(programName: widget.programName, runningError: error)
		));
	}

  @override
  Widget build(BuildContext context) {
    context.read<ComPort>().trace(widget.programPreference.withTrace);
    return LayoutBuilder(builder: (context, constraints) {
      // Send the safe area to the runtime.
      final EdgeInsets safeArea = MediaQuery.of(context).padding;
      widget.comPort.resize(constraints.maxWidth, constraints.maxHeight,
          safeArea.top, safeArea.left, safeArea.bottom, safeArea.right);
      // Capture simple touch events
      return Scaffold(
          body: Stack(children: [
        _buildTapGesture(context, constraints),
        _buildMenu(context)
      ]));
    });
  }

  Widget _buildTapGesture(BuildContext context, BoxConstraints constraints) {
    return RawGestureDetector(
        gestures: {
          TapAndPanGestureRecognizer:
              GestureRecognizerFactoryWithHandlers<TapAndPanGestureRecognizer>(
                  () => TapAndPanGestureRecognizer(), (instance) {
            instance
              ..onTapDown = (details) {
                widget.comPort.touchOn(details.localPosition);
              }
              ..onTapUp = (details) {
                widget.comPort.touchOff();
              }
              ..onDragStart = (details) {
                widget.comPort.touchOn(details.localPosition);
              }
              ..onDragUpdate = (details) {
                widget.comPort.touchOn(details.localPosition);
              }
              ..onDragEnd = (details) {
                widget.comPort.touchOff();
              };
          })
        },
        // Capture keyboard events
        child: _buildKeyboardEvent(context, constraints));
  }

  Widget _buildKeyboardEvent(BuildContext context, BoxConstraints constraints) {
    return Focus(
        autofocus: true,
        onKeyEvent: (node, event) {
          log("RunPage.KeyEvent: $event");
          if (event is KeyDownEvent &&
              event.logicalKey == LogicalKeyboardKey.escape) {
            _gotoPrevious(context);
            return KeyEventResult.handled;
          } else {
            return KeyEventResult.ignored;
          }
        },
        child: _buildScreen(constraints, context));
  }

  Widget _buildScreen(BoxConstraints constraints, BuildContext context) {
    return SizedBox(
        width: constraints.maxWidth,
        height: constraints.maxHeight,
        // Make the screen scaled and aligned to the top left.
        child: FittedBox(
            fit: BoxFit.cover,
            alignment: Alignment.topLeft,
            child: MyScreenPaint(
              comPort: context.read<ComPort>(),
            )));
  }

  Widget _buildMenu(BuildContext context) {
    return Align(
        alignment: Alignment.topRight,
        child: Padding(
            padding: const EdgeInsets.fromLTRB(0.0, 8.0, 32.0, 0.0),
            child: PopupMenuButton<MyRunMenuOption>(
                child: ClipRRect(
                    borderRadius: BorderRadius.circular(100),
                    child: Container(
                      width: 32,
                      height: 32,
                      color: Colors.black45,
                      child: Icon(Icons.adaptive.more),
                    )),
                itemBuilder: (BuildContext context) => [
                      CheckedPopupMenuItem(
                        value: MyRunMenuOption.trace,
                        checked: widget.programPreference.withTrace,
                        child: const Text("Show trace"),
                      ),
                      const PopupMenuItem(
                        value: MyRunMenuOption.thumbnail,
                        child: Text("Save thumbnail"),
                      )
                    ],
                onSelected: (value) async {
                  final currentTrace = widget.programPreference.withTrace;
                  switch (value) {
                    case MyRunMenuOption.trace:
                      // Apply the change
                      context.read<ComPort>().trace(!currentTrace);
                      // Store the change
                      widget.programPreference.setWithTrace(!currentTrace);
                      break;
                    case MyRunMenuOption.thumbnail:
											context.read<ComPort>().thumbnail();
                      break;
                  }
                })));
  }
}
