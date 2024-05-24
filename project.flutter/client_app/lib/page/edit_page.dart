import 'dart:async';
import 'dart:developer' show log;

import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:lowresrmx/widget/code_editor.dart';

import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/style.dart';
import 'package:lowresrmx/widget/search_panel.dart';

// TODO: recover the last cursor position

/// The code editor take the program name from the route argument class MyEditArgs.
class MyEditPage extends StatefulWidget {
  final String programName;

  const MyEditPage({required this.programName, super.key});

  @override
  State<MyEditPage> createState() => _MyEditPageState();
}

class _MyEditPageState extends State<MyEditPage> {
  late final CodeLineEditingController editingController; // =
  // CodeLineEditingController();
  final Completer<bool> ready = Completer();

  void initCode() async {
    final String code = await MyLibrary.readCode(widget.programName);
    editingController = CodeLineEditingController.fromText(code);
    // FIXME: sThis value should only be set between frames, e.g. in response to user actions, not during the build, layout, or paint phases.
    // editingController.textAsync = await MyLibrary.readCode(widget.programName);
    ready.complete(true);
    log("Program ${widget.programName} loaded");
  }

  @override
  void initState() {
    log("MyEditPage.initState");
    super.initState();
    initCode();
  }

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    return Scaffold(
      appBar: AppBar(
        title: const Text("hm"),
        actions: [
          IconButton(onPressed: () {}, icon: const Icon(Icons.search_rounded)),
          IconButton(
              onPressed: () {
                Runtime runtime = context.read<Runtime>();
                // Error err = runtime.compileAndStart(controller.text);
                // log("Error: $err");
                // if (err.ok) {
                //   Navigator.of(context).push(MaterialPageRoute(
                //       builder: (context) => MyRunPage(runtime)));
                // }
                // TODO: handle error
              },
              tooltip: "Run project",
              icon: const Icon(Icons.more_vert_rounded)),
          IconButton(onPressed: () {}, icon: const Icon(Icons.toc_rounded)),
          const SizedBox(width: 24.0),
        ],
      ),
      body: PopScope(
          onPopInvoked: (didPop) async {
            await ready.future;
            await MyLibrary.writeCode(
                widget.programName, editingController.text);
            log("Program ${widget.programName} saved");
          },
          child: FutureBuilder(
              future: ready.future,
              builder: (context, snapshot) {
                if (snapshot.hasData) {
                  return Column(
                    children: [
                      Expanded(
                          child: MyCodeEditor(
                        controller: editingController,
                      )),
                    ],
                  );
                } else {
                  // TODO: loading? Maybe after a couple of milliseconds of delay
                  return const SizedBox();
                }
              })),
      floatingActionButton: FloatingActionButton.small(
        onPressed: () {},
        tooltip: "Run program",
        child: const Icon(Icons.play_arrow_rounded),
      ),
    );
  }
}
