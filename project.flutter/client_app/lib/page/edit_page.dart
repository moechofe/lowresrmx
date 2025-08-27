import 'dart:async';
import 'dart:developer' show log;
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_keyboard_visibility/flutter_keyboard_visibility.dart';
import 'package:lowresrmx/widget/keyboard_bar.dart';

import 'package:provider/provider.dart';
import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/location.dart';
import 'package:lowresrmx/data/outline_entry.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/widget/code_editor.dart';
import 'package:lowresrmx/widget/edit_drawer.dart';
import 'package:lowresrmx/widget/outline_drawer.dart';
import 'package:lowresrmx/widget/search_button.dart';

// TODO: flutter client_app: Store the last cursor position in the code editor and restore it when the code is reloaded.

enum MyEditMenu {
  manual,
}

class MyContextMenuItemWidget extends PopupMenuItem<void>
    implements PreferredSizeWidget {
  MyContextMenuItemWidget({
    super.key,
    required String text,
    required VoidCallback super.onTap,
  }) : super(child: Text(text));

  @override
  Size get preferredSize => const Size(100, 25);
}

class MyEditPageToolbarController implements SelectionToolbarController {
  const MyEditPageToolbarController();

  @override
  void hide(BuildContext context) {}

  @override
  void show(
      {required BuildContext context,
      required CodeLineEditingController controller,
      required TextSelectionToolbarAnchors anchors,
      Rect? renderRect,
      required LayerLink layerLink,
      required ValueNotifier<bool> visibility}) {
    showMenu(
        context: context,
        useRootNavigator: true,
        position: RelativeRect.fromSize(
            anchors.primaryAnchor & const Size(100, double.infinity),
            MediaQuery.of(context).size),
        items: [
          MyContextMenuItemWidget(
              text: "Cut",
              onTap: () {
                controller.cut();
              }),
          MyContextMenuItemWidget(
              text: "Copy",
              onTap: () {
                controller.copy();
              }),
          MyContextMenuItemWidget(
              text: "Paste",
              onTap: () {
                controller.paste();
              }),
        ]);
  }
}

class MyEditPage extends StatefulWidget {
  static const routeName = '/edit';

  const MyEditPage({super.key});

  @override
  State<MyEditPage> createState() => _MyEditPageState();
}

class _MyEditPageState extends State<MyEditPage> with WidgetsBindingObserver {
  // @override
  // String? get restorationId => 'edit_page';

  // @override
  // void restoreState(RestorationBucket? oldBucket, bool initialRestore) {
  // 	registerForRestoration(property, restorationId)
  // 	// TODO: implement restoreState
  // }

  /// Used to give back the focus to the code editor after a error dialog.
  final FocusNode focusNode = FocusNode();

  /// Used to block any gesture during compilation.
  bool absorb = false;

  /// Wait for the editor to be ready.
  final Completer<bool> codeReady = Completer();

  /// Used to access the scaffold from the actions bar button and open the drawer.
  final GlobalKey<ScaffoldState> scaffoldKey = GlobalKey<ScaffoldState>();

  // For re_editor widgets.
  late final CodeLineEditingController editingController;
  late final CodeFindController findController;
  late final CodeScrollController scrollController;
  late final SelectionToolbarController selectionToolbarController;

  /// Used to continuously compile the code and report errors in the gutter.
  Timer? continuouslyCompileTimer;

  /// Used to detect changes in the code. Avoiding compiling the same code twice.
  int? lastCodeHash;

  /// Used to gather location of errors and warnings caused by the continuous compilation.
  final MyContinousLocation location = MyContinousLocation();

  /// Used to store the outline entries to feed the outline drawer.
  final MyOutlineEntries outline = MyOutlineEntries();

  /// The program name to execute.
  String? editedProgramName;

  /// The data disk name to use.
  String? dataDiskProgramName;

  /// The program name to use as data disk.
  String? executedProgramName;

  /// Used to store tool and trace flags.
  late final MyProgramPreference editedProgramPreference;

  @override
  // Calling this multiple times will break the editor.
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    final programName = (ModalRoute.of(context)!.settings.arguments
        as Map)["programName"]! as String;
    super.didChangeAppLifecycleState(state);
    if (state == AppLifecycleState.paused) {
      // The editor may not be ready here, if app is home before the editor is ready.
      if (codeReady.isCompleted) {
        MyLibrary.writeCode(programName, editingController.text);
      }
    }
  }

  Future<bool> initEditor(BuildContext context) async {
    if (codeReady.isCompleted) return true;

    final ComPort comPort = context.read<ComPort>();

    log("initEditor()");

    final programName = (ModalRoute.of(context)!.settings.arguments
        as Map)["programName"]! as String;
    final Error? runningError = Error.fromMap(
        (ModalRoute.of(context)!.settings.arguments as Map)["runningError"]
            as Map<String, dynamic>?);

    // Load the program code.
    final String code = await MyLibrary.readCode(programName);

    editingController = CodeLineEditingController.fromText(code);
    findController = CodeFindController(editingController);
    scrollController = CodeScrollController();
    selectionToolbarController = const MyEditPageToolbarController();

    // When the code change, it will compile it and report errors in the gutter.
    editingController.addListener(() {
      if (continuouslyCompileTimer != null) {
        continuouslyCompileTimer!.cancel();
      }
      continuouslyCompileTimer =
          Timer(const Duration(milliseconds: 400), compileOnly);
    });

    // When the code is ready, it will place the cursor at the error location and scroll to it.
    if (runningError != null) {
      log("Error ${runningError.msg}, ${runningError.getLocation(editingController.text).row}");
      WidgetsBinding.instance.addPostFrameCallback((_) {
        log("addPostFrameCallback");
        location.setLocation(runningError.getLocation(editingController.text));
        gotoLocation(location.location);
      });
    }

    // Compilation will update the outline entries.
    comPort.onOutline = (entries) {
      outline.entries = entries;
    };

    compileOnly();

    editedProgramPreference = MyProgramPreference(programName);
    editedProgramPreference.loadPreference();

    codeReady.complete(true);
    return true;
  }

  /// Compile the current program and report errors in the gutter and outline in the drawer.
  void compileOnly() {
    int currentHash = editingController.text.hashCode;
    if (lastCodeHash != currentHash) {
      lastCodeHash = currentHash;
      context
          .read<ComPort>()
          .compileOnly(editingController.text)
          .then((Error err) {
        location.setLocation(err.getLocation(editingController.text));
      });
    }
  }

  Future<Error> compileAndRun(ComPort comPort,{required String programSource, required String dataDisk}) async {
    	return await comPort.compileAndRun(programSource, dataDisk);
  }

  void runEditedProgramWithLibraryDataDisk(String executedProgramName) {
    final ComPort comPort = context.read<ComPort>();
    setState(() {
      absorb = true;
      MyLibrary.writeCode(executedProgramName, editingController.text)
          .then((_) {
        MyLibrary.readCode(".dataDisk").then((dataDisk) {
          compileAndRun(comPort,
                  programSource: editingController.text, dataDisk: dataDisk)
              .then((Error err) {
            if (err.ok) {
              setState(() {
                this.executedProgramName = executedProgramName;
                editedProgramName = executedProgramName;
                dataDiskProgramName = ".dataDisk";
              });
            } else {
              setState(() {
                absorb = false;
                reportCompileError(err);
              });
            }
          });
        });
      });
    });
  }

  void runToolProgramWithEditedDataDisk(String toolProgramName) {
    final ComPort comPort = context.read<ComPort>();
    final programName = (ModalRoute.of(context)!.settings.arguments
        as Map)["programName"]! as String;
    setState(() {
      absorb = true;
      MyLibrary.writeCode(programName, editingController.text).then((_) {
        MyLibrary.readCode(toolProgramName).then((toolProgramSource) {
          final String dataDisk = editingController.text;
          compileAndRun(comPort,
                  programSource: toolProgramSource, dataDisk: dataDisk)
              .then((err) {
            if (err.ok) {
              setState(() {
                executedProgramName = toolProgramName;
                editedProgramName = programName;
                dataDiskProgramName = programName;
              });
            } else {
              setState(() {
                absorb = false;
                reportCompileError(err);
              });
            }
          });
        });
      });
    });
  }

  /// Will switch to the run page. Program should be compiled and run before.
  void gotoRun(BuildContext context,
      {required String editedName,
      required String dataDiskName,
      required String executedName}) {
    Navigator.of(context).pushReplacement(MaterialPageRoute(
        builder: (context) => MyRunPage(
            comPort: context.read<ComPort>(),
            editingName: editedName,
            dataDiskName: dataDiskName,
            executedName: executedName)));
  }

  void reportCompileError(Error err) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    // final TextStyle textStyle = TextStyle(color: colorScheme.onErrorContainer);
    final TextStyle btnStyle =
        TextStyle(color: colorScheme.onError, fontWeight: FontWeight.bold);
    location.setLocation(err.getLocation(editingController.text));
    // Show an error dialog
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          icon: const Icon(Icons.error_rounded),
          title: const Text("Error"), //, style: textStyle),
          content: Text(err.msg), // style: textStyle),
          actions: [
            FilledButton(
              style: FilledButton.styleFrom(
                backgroundColor: colorScheme.error,
              ),
              onPressed: () {
                Navigator.of(context).pop();
                // Will place the cursor at the error location and scroll to it.
                gotoLocation(location.location);
              },
              child: Text("Go to", style: btnStyle),
            ),
          ],
        );
      },
    ).then((_) {
      focusNode.requestFocus();
    });
  }

  void gotoLocation(Location location) {
    focusNode.requestFocus();
    editingController.selection = CodeLineSelection.collapsed(
        index: location.row, offset: location.column);
    editingController.makePositionVisible(
        CodeLinePosition(index: location.row, offset: location.column));
  }

  @override
  Widget build(BuildContext context) {
    if (executedProgramName != null && editedProgramName != null) {
      assert(absorb, "Should be absorbing");
      WidgetsBinding.instance.addPostFrameCallback((_) {
        // Need to reset the values here. A rebuild may be requested and I don't wont to gotoRun twice.
        String programName = editedProgramName!;
        String dataDiskName = dataDiskProgramName!;
        String executedName = executedProgramName!;
        editedProgramName = null;
        dataDiskProgramName = null;
        executedProgramName = null;
        gotoRun(context,
            editedName: programName,
            dataDiskName: dataDiskName,
            executedName: executedName);
      });
    }
    return FutureBuilder(
        future: initEditor(context),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(
                providers: [
                  ChangeNotifierProvider(create: (_) => findController),
                  ChangeNotifierProvider(create: (_) => location),
                  ChangeNotifierProvider(create: (_) => outline),
                ],
                builder: (providerContext, _) {
                  return buildScaffold(providerContext);
                });
          } else if (snapshot.hasError) {
            return const Scaffold(
              body: Center(child: Text("Error loading editor")),
            );
          } else {
            return const Scaffold();
          }
        });
  }

  Widget buildScaffold(BuildContext providerContext) {
    return AbsorbPointer(
      absorbing: absorb,
      child: Scaffold(
        key: scaffoldKey,
        appBar: buildAppBar(providerContext),
        drawer: buildDrawer(providerContext),
        endDrawer: buildOutlineDrawer(providerContext),
        body: buildBody(providerContext),
        floatingActionButton: FloatingActionButton.small(
          onPressed: () {
            final programName = (ModalRoute.of(context)!.settings.arguments
                as Map)["programName"]! as String;
            runEditedProgramWithLibraryDataDisk(programName);
          },
          tooltip: "Run program",
          child: const Icon(Icons.play_arrow_rounded),
        ),
        onDrawerChanged: (isOpened) {
          if (!isOpened) {
            final programName = (ModalRoute.of(context)!.settings.arguments
                as Map)["programName"]! as String;
            MyLibrary.writeCode(programName, editingController.text);
          }
        },
      ),
    );
  }

  PreferredSizeWidget buildAppBar(BuildContext providerContext) {
    final programName = (ModalRoute.of(context)!.settings.arguments
        as Map)["programName"]! as String;
    return AppBar(title: Text(programName), actions: [
      const MySearchIcon(),
      Padding(
          padding: const EdgeInsets.only(right: 8.0),
          child: IconButton(
            onPressed: () => scaffoldKey.currentState!.openEndDrawer(),
            icon: const Icon(Icons.toc_rounded),
          ))
    ]);
  }

  Widget buildDrawer(BuildContext providerContext) {
    final programName = (ModalRoute.of(context)!.settings.arguments
        as Map)["programName"]! as String;
    return MyEditDrawer(
        editedProgramName: programName,
        editingController: editingController,
        onStartTool: runToolProgramWithEditedDataDisk);
  }

  Widget buildOutlineDrawer(BuildContext providerContext) {
    final MyOutlineEntries outline = providerContext.watch<MyOutlineEntries>();
    return MyOutlineDrawer(
        program: editingController.text,
        entries: outline.entries,
        gotoLocation: (location) {
          gotoLocation(location);
        });
  }

  Widget buildBody(BuildContext providerContext) {
    return KeyboardVisibilityBuilder(builder: (context, visible) {
      return Column(children: [
        Expanded(child: buildCodeEditor(providerContext)),
        if (visible || Platform.isLinux)
          MyKeyboardBar(controller: editingController, focusNode: focusNode)
      ]);
    });
  }

  Widget buildCodeEditor(BuildContext providerContext) {
    final MyEditorPreference settings =
        providerContext.watch<MyEditorPreference>();
    return MyCodeEditor(
        editingController: editingController,
        findController: findController,
        scrollController: scrollController,
        focusNode: focusNode,
        editorSettings: settings);
  }
}
