import 'dart:async';
import 'dart:developer' show log;

import 'package:flutter/material.dart';
import 'package:lowresrmx/widget/edit_drawer.dart';

import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/widget/code_editor.dart';
import 'package:lowresrmx/widget/search_button.dart';
import 'package:lowresrmx/data/library.dart';

// TODO: recover the last cursor position
// TODO: save regularly

enum MyEditMenu {
	manual,
}

class MyEditPage extends StatefulWidget {
  final String programName;

  const MyEditPage({required this.programName, super.key});

  @override
  State<MyEditPage> createState() => _MyEditPageState();
}

class _MyEditPageState extends State<MyEditPage> {
  late final CodeLineEditingController editingController;
  late final CodeFindController findController;

  final Completer<bool> ready = Completer();

  void initCode() async {
    final String code = await MyLibrary.readCode(widget.programName);
    editingController = CodeLineEditingController.fromText(code);
    findController = CodeFindController(editingController);
    ready.complete(true);
    log("Program ${widget.programName} loaded");
  }

  @override
  void initState() {
    super.initState();
    initCode();
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
        future: ready.future,
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(providers: [
              ChangeNotifierProvider(create: (_) => findController),
            ], child: buildScaffold(context));
          } else {
            return const SizedBox();
          }
        });
  }

  Widget buildScaffold(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.programName),
        actions: [
          const MySearchIcon(),
          IconButton(onPressed: () {}, icon: const Icon(Icons.toc_rounded)),
          const SizedBox(width: 24.0),
        ],
      ),
			drawer: const MyEditDrawer(),
      body: PopScope(
          onPopInvoked: (didPop) async {
            await MyLibrary.writeCode(
                widget.programName, editingController.text);
          },
          child: Column(
            children: [
              Expanded(
                  child: MyCodeEditor(
                editingController: editingController,
                findController: findController,
              )),
            ],
          )),
      floatingActionButton: FloatingActionButton.small(
        onPressed: () async {
          Runtime runtime = context.read<Runtime>();
          Error err = runtime.compileAndStart(editingController.text);
          log("Error: $err");
          if (err.ok) {
						// TODO: I should replace the current page with the run page and do the opposite when going back
            Navigator.of(context).push(
                MaterialPageRoute(builder: (context) => MyRunPage(runtime: runtime, programName: widget.programName)));
						await MyLibrary.writeCode(widget.programName, editingController.text);
          }
          // TODO: handle error
        },
        tooltip: "Run program",
        child: const Icon(Icons.play_arrow_rounded),
      ),
    );
  }
}
