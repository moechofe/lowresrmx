import 'dart:async';
import 'dart:developer' show log;
import 'dart:isolate';

import 'package:flutter/material.dart';
import 'package:lowresrmx/data/location.dart';

import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/widget/code_editor.dart';
import 'package:lowresrmx/widget/edit_drawer.dart';
import 'package:lowresrmx/widget/search_button.dart';

// TODO: recover the last cursor position
// TODO: save regularly

enum MyEditMenu {
  manual,
}

/// The page to edit and run a program.
class MyEditPage extends StatefulWidget {
  final String programName;
  final MyContinousLocation location = MyContinousLocation();
  late final Error? runningError;

  MyEditPage({required this.programName, super.key}) : runningError = null;
  MyEditPage.withError(
      {required this.programName, required this.runningError, super.key});

  @override
  State<MyEditPage> createState() => _MyEditPageState();
}

class _MyEditPageState extends State<MyEditPage> with WidgetsBindingObserver {
  late final CodeLineEditingController editingController;
  late final CodeFindController findController;
  final CodeScrollController scrollController = CodeScrollController();

  /// Used to give back the focus to the code editor after a error dialog.
  late final FocusNode focusNode;

  /// Used to continuously compile the code and report errors in the gutter.
  Timer? continuouslyCompileTimer;

  /// Used to detecte changes in the code.
  int? lastCodeHash;

  /// Used to block any gesture during compilation.
  bool absorb = false;

  /// Wait for the program code to be loaded.
  final Completer<bool> codeReady = Completer();

  /// Setup the code editor with the code from the library
  void initEditor() async {
    final String code = await MyLibrary.readCode(widget.programName);
    editingController = CodeLineEditingController.fromText(code);
    findController = CodeFindController(editingController);
    codeReady.complete(true);
    lastCodeHash = code.hashCode;

    // Continuously compile the code to report errors in the gutter.
    editingController.addListener(() {
      if (continuouslyCompileTimer != null) {
        continuouslyCompileTimer!.cancel();
      }
      continuouslyCompileTimer = Timer(const Duration(milliseconds: 400), () {
        int currentHash = editingController.text.hashCode;
        if (lastCodeHash != currentHash) {
          lastCodeHash = currentHash;
          context
              .read<ComPort>()
              .compileOnly(editingController.text)
              .then((Error err) {
            widget.location
                .setLocation(err.getLocation(editingController.text));
            // continousLocation
            //     .setLocation(err.getLocation(editingController.text));
            // if (! err.ok) {
            // 		// Show a snackbar with the error message
            // 		ScaffoldMessenger.of(context).showSnackBar(
            // 			SnackBar(
            // 				content: Text(err.msg),

            // 			),
            // 		);

            // }
          });
        }
      });
    });

    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (widget.runningError != null) {
        log("Error ${widget.runningError!.msg}, ${widget.runningError!.getLocation(editingController.text).index}");
        final Location location =
            widget.runningError!.getLocation(editingController.text);
				// Will show the bug icon in the gutter
        widget.location.setLocation(location);

        // Will place the cursor at the error location and scroll to it.
        editingController.selection = CodeLineSelection.collapsed(
            index: location.index, offset: location.offset);
        editingController.makePositionVisible(
            CodeLinePosition(index: location.index, offset: location.offset));
      }
    });
  }

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
    focusNode = FocusNode(); //debugLabel: "EditPage");
    initEditor();
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    super.didChangeAppLifecycleState(state);
    if (state == AppLifecycleState.paused) {
      // Save the code when going to the background
      MyLibrary.writeCode(widget.programName, editingController.text);
    } else if (state == AppLifecycleState.resumed) {
      // Reload the code when coming back, allow external changes
      // TODO: need to test it. I'm not sure if it will work.
      // Do not work, because editingController and findController are marked as final
      // I may not need it anyway.
      // initCode();
    }
  }

  void compileAndRun(BuildContext context, MyProgramPreference preference) {
    setState(() {
      absorb = true;
      context
          .read<ComPort>()
          .compileAndRun(editingController.text)
          .then((Error err) {
        if (err.ok) {
          // Will replace the current page with the run page.
          Navigator.of(context).pushReplacement(MaterialPageRoute(
              builder: (context) => MyRunPage(
                    comPort: context.read<ComPort>(),
                    programName: widget.programName,
                    programPreference: preference,
                  )));
        } else {
          setState(() {
            absorb = false;
            reportError(err);
          });
        }
      });
    });
  }

  void reportError(Error err) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final TextStyle textStyle = TextStyle(color: colorScheme.onErrorContainer);
    final TextStyle btnStyle =
        TextStyle(color: colorScheme.onError, fontWeight: FontWeight.bold);
    final Location location = err.getLocation(editingController.text);
    // Show an error dialog
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          backgroundColor: colorScheme.errorContainer,
          icon: Icon(Icons.error_rounded, color: colorScheme.onErrorContainer),
          title: Text("Error", style: textStyle),
          content: Text(err.msg, style: textStyle),
          actions: [
            FilledButton(
              style: FilledButton.styleFrom(
                backgroundColor: colorScheme.error,
              ),
              onPressed: () {
                Navigator.of(context).pop();
                // Will place the cursor at the error location and scroll to it.
                editingController.selection = CodeLineSelection.collapsed(
                    index: location.index, offset: location.offset);
                editingController.makePositionVisible(CodeLinePosition(
                    index: location.index, offset: location.offset));
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

  @override
  Widget build(BuildContext context) {
    final MyProgramPreference programPreference =
        MyProgramPreference(widget.programName);
    // Wait for the code and the preference to be loaded.
    return FutureBuilder(
        future:
            Future.wait([codeReady.future, programPreference.loadPreference()]),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(providers: [
              // Provide the findController to the code editor because it is not a child of the code editor.
              ChangeNotifierProvider(create: (_) => findController),
              // Used to transport errors from the runtime to the gutter of the code editor.
              // ChangeNotifierProvider(create: (_) => continousLocation),
              ChangeNotifierProvider(create: (_) => widget.location)
            ], child: buildScaffold(context, programPreference));
          } else {
            return const SizedBox();
          }
        });
  }

  Widget buildScaffold(
      BuildContext context, MyProgramPreference programPreference) {
    // AbsorbPointer is used to block any gesture during compilation.
    return AbsorbPointer(
      absorbing: absorb,
      child: Scaffold(
        appBar: AppBar(
          title: Text(widget.programName),
          actions: [
            const MySearchIcon(),
            IconButton(onPressed: () {}, icon: const Icon(Icons.toc_rounded)),
          ],
        ),
        drawer: _buildDrawer(programPreference),
        body: PopScope(
            onPopInvoked: (didPop) async {
              // Save the code at exit
              await MyLibrary.writeCode(
                  widget.programName, editingController.text);
            },
            child: Column(
              children: [
                Expanded(child: _buildCodeEditor()),
              ],
            )),
        floatingActionButton: FloatingActionButton.small(
          onPressed: () {
            // Compile and run the program
            compileAndRun(context, programPreference);
            MyLibrary.writeCode(widget.programName, editingController.text);
          },
          tooltip: "Run program",
          child: const Icon(Icons.play_arrow_rounded),
        ),
      ),
    );
  }

  MyCodeEditor _buildCodeEditor() {
    return MyCodeEditor(
      editingController: editingController,
      findController: findController,
      scrollController: scrollController,
      focusNode: focusNode,
    );
  }

  MyEditDrawer _buildDrawer(MyProgramPreference programPreference) =>
      MyEditDrawer(programPreference: programPreference);
}
