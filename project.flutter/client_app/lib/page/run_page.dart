import 'dart:developer' show log;
import 'dart:convert';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_keyboard_visibility/flutter_keyboard_visibility.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/widget/screen_paint.dart';
import 'package:provider/provider.dart';

enum MyRunMenuOption {
  trace,
  thumbnail,
  returnEditor,
}

/// A page that show a running program.
/// It does not compile the program, it only runs it.
class MyRunPage extends StatefulWidget {
  final ComPort comPort;

  /// The program that was edited in the MyEditPage.
  final String editingName;

  /// The data disk used to save on it.
  final String dataDiskName;

  /// The name of the executed program used for retrieve the trace flag.
  final String executedName;

  const MyRunPage(
      {required this.comPort,
      required this.editingName,
      required this.dataDiskName,
      required this.executedName,
      super.key});

  @override
  State<MyRunPage> createState() => _MyRunPageState();
}

class _MyRunPageState extends State<MyRunPage> {
  late final MyProgramPreference programPreference;

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

    programPreference = MyProgramPreference(widget.executedName);

    // TODO: move this into a Completer/Future
    // Actually, nothing required async here, and no future are needed.
    widget.comPort.onRunningError = (error) {
      widget.comPort.onRunningError = null;
      widget.comPort.stop();
      reportError(error);
    };
    widget.comPort.onThumbnail = (image) {
      MyLibrary.writeThumbnail(widget.editingName, image);
    };
    widget.comPort.onSaveDataDisk = (dataDisk) {
      MyLibrary.writeCode(widget.dataDiskName, dataDisk);
    };
    widget.comPort.onKeyboardVisible = (visible) {
      if (visible) {
        SystemChannels.textInput.invokeMethod("TextInput.show");
      } else {
        SystemChannels.textInput.invokeMethod("TextInput.hide");
      }
    };
    WidgetsBinding.instance.addPostFrameCallback((_) {
      widget.comPort.start();
    });
  }

  @override
  void dispose() {
    widget.comPort.stop();
    widget.comPort.onSaveDataDisk = null;
    widget.comPort.onThumbnail = null;
    widget.comPort.onRunningError = null;
    SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual,
        overlays: SystemUiOverlay.values);

    super.dispose();
  }

  void reportError(Error err) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final TextStyle btnStyle =
        TextStyle(color: colorScheme.onError, fontWeight: FontWeight.bold);
    showDialog(
        context: context,
        builder: (context) {
          return AlertDialog(
              icon: const Icon(Icons.error_rounded),
              title: const Text("Error"),
              content: Text(err.msg),
              actions: [
                FilledButton(
                    style: FilledButton.styleFrom(
                      backgroundColor: colorScheme.error,
                    ),
                    child: Text("Go to", style: btnStyle),
                    onPressed: () {
                      Navigator.of(context).pop();
                      gotoError(context, err);
                    })
              ]);
        });
  }

  /// Will go back to the previous page.
  void gotoEditor(BuildContext context) {
    Navigator.of(context).restorablePushReplacementNamed(MyEditPage.routeName,
        arguments: {"programName": widget.editingName});
  }

  void gotoError(BuildContext context, Error error) {
    Navigator.of(context).restorablePushReplacementNamed(MyEditPage.routeName,
        arguments: {
          "programName": widget.editingName,
          "runningError": error.toMap()
        });
  }

  @override
  Widget build(BuildContext context) {
// TODO: must include Provider for MyProgramPreference

    log("RunPageState.build()");
    return FutureBuilder(
        future: programPreference.loadPreference(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(
              providers: [
                ChangeNotifierProvider<MyProgramPreference>(
                    create: (context) => snapshot.data as MyProgramPreference),
              ],
              builder: (context, _) => buildScope(context),
            );
          } else if (snapshot.hasError) {
            return const Center(
                child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [Text("Error loading program preferences")]));
          } else {
            return const SizedBox();
          }
        });
  }

  Widget buildScope(BuildContext context) {
    final MyProgramPreference programPreference =
        context.read<MyProgramPreference>();
    widget.comPort.trace(programPreference.withTrace);

    return PopScope(
      canPop: false,
      onPopInvoked: (_) => gotoEditor(context),
      child: buildLayout(),
    );
  }

  Widget buildLayout() {
    return LayoutBuilder(builder: (context, constraints) {
      // Send the safe area to the runtime.
      final EdgeInsets safeArea = MediaQuery.of(context).padding;
      final EdgeInsets viewInsets = MediaQuery.of(context).viewInsets;
      log("LayoutBuilder: ${viewInsets.toString()} ${safeArea.toString()}");
      widget.comPort.resize(
          constraints.maxWidth,
          constraints.maxHeight,
          safeArea.top + viewInsets.top,
          safeArea.left + viewInsets.left,
          safeArea.bottom + viewInsets.bottom,
          safeArea.right + viewInsets.right);
      // Capture simple touch events
      return Scaffold(
          resizeToAvoidBottomInset: false,
          body: Stack(children: [
            buildTapGesture(context, constraints),
            buildMenu(context)
          ]));
    });
  }

  Widget buildTapGesture(BuildContext context, BoxConstraints constraints) {
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
        child: buildKeyboardEvent(context, constraints));
  }

  Widget buildKeyboardEvent(BuildContext context, BoxConstraints constraints) {
    return Focus(
        autofocus: true,
        onKeyEvent: (node, event) {
					if (event is! KeyDownEvent) {
						return KeyEventResult.ignored;
					}
          else if (event.logicalKey == LogicalKeyboardKey.escape) {
            gotoEditor(context);
            return KeyEventResult.handled;
					} else if (event.logicalKey == LogicalKeyboardKey.enter) {
						widget.comPort.keyDown(10);
						return KeyEventResult.handled;
					} else if (event.logicalKey == LogicalKeyboardKey.backspace) {
						widget.comPort.keyDown(8);
						return KeyEventResult.handled;
          } else {
						String? char = (event as KeyEvent).character;
						log("Char: $char");
						if (char != null) {
							Uint8List list = ascii.encode(char);
							if (list.length == 1) {
								int code = list[0];
								log("Code: $code");
								if (code >= 32 && code <= 95)
								{
									widget.comPort.keyDown(code);
									return KeyEventResult.handled;
								}
								else if (code >= 97 && code <= 122)
								{
									widget.comPort.keyDown(code - 32);
									return KeyEventResult.handled;
								}
							}
						}
						return KeyEventResult.ignored;
          }
        },
        child: IgnoreKeyboardDismiss(child: buildScreen(constraints, context)));
  }

  Widget buildScreen(BoxConstraints constraints, BuildContext context) {
    return SizedBox(
        width: constraints.maxWidth,
        height: constraints.maxHeight,
        // Make the screen scaled and aligned to the top left.
        child: FittedBox(
            fit: BoxFit.cover,
            alignment: Alignment.topLeft,
            child: MyScreenPaint(
              comPort: widget.comPort,
            )));
  }

  Widget buildMenu(BuildContext context) {
    return Align(
        alignment: Alignment.topRight,
        child: Padding(
            padding: const EdgeInsets.fromLTRB(0.0, 12.0, 12.0, 0.0),
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
                      const PopupMenuItem(
                        value: MyRunMenuOption.returnEditor,
                        child: Row(
                          children: [
                            Icon(Icons.arrow_back_rounded),
                            SizedBox(width: 12.0),
                            Text("Return to editor")
                          ],
                        ),
                      ),
                      CheckedPopupMenuItem(
                        value: MyRunMenuOption.trace,
                        checked: programPreference.withTrace,
                        child: const Text("Show trace"),
                      ),
                      const PopupMenuItem(
                          value: MyRunMenuOption.thumbnail,
                          child: Row(children: [
                            Icon(Icons.image_rounded),
                            SizedBox(width: 12.0),
                            Text("Save thumbnail")
                          ]))
                    ],
                onSelected: (value) async {
                  final currentTrace = programPreference.withTrace;
                  switch (value) {
                    case MyRunMenuOption.trace:
                      // Apply the change
                      widget.comPort.trace(!currentTrace);
                      // Store the change
                      programPreference.setWithTrace(!currentTrace);
                      break;
                    case MyRunMenuOption.thumbnail:
                      widget.comPort.thumbnail();
                      break;
                    case MyRunMenuOption.returnEditor:
                      gotoEditor(context);
                      break;
                  }
                })));
  }
}
