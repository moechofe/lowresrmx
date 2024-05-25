import 'dart:async';
import 'dart:developer' show log;

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/widget/code_editor.dart';
import 'package:lowresrmx/widget/search_button.dart';
import 'package:lowresrmx/data/library.dart';

// TODO: recover the last cursor position
// TODO: save regularly

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
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    return FutureBuilder(
        future: ready.future,
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(
                providers: [
                  ChangeNotifierProvider(create: (_) => findController),
                ],
                child: Scaffold(
                  appBar: AppBar(
                    title: const Text("hm"),
                    actions: [
                      const MySearchIcon(),
                      IconButton(
                          onPressed: () {},
                          tooltip: "Run project",
                          icon: const Icon(Icons.more_vert_rounded)),
                      IconButton(
                          onPressed: () {},
                          icon: const Icon(Icons.toc_rounded)),
                      const SizedBox(width: 24.0),
                    ],
                  ),
                  body: PopScope(
                      onPopInvoked: (didPop) async {
                        await MyLibrary.writeCode(
                            widget.programName, editingController.text);
                      },
                      child: FutureBuilder(
                          future: ready.future,
                          builder: (context, snapshot) {
                            if (snapshot.hasData) {
                              return Column(
                                children: [
                                  Expanded(
                                      child: MyCodeEditor(
                                    editingController: editingController,
                                    findController: findController,
                                  )),
                                ],
                              );
                            } else {
                              // TODO: loading? Maybe after a couple of milliseconds of delay
                              return const SizedBox();
                            }
                          })),
                  floatingActionButton: FloatingActionButton.small(
                    onPressed: () async {
                      await MyLibrary.writeCode(
                          widget.programName, editingController.text);
                      Runtime runtime = context.read<Runtime>();
                      Error err =
                          runtime.compileAndStart(editingController.text);
                      log("Error: $err");
                      if (err.ok) {
                        Navigator.of(context).push(MaterialPageRoute(
                            builder: (context) => MyRunPage(runtime)));
                      }
                      // TODO: handle error
                    },
                    tooltip: "Run program",
                    child: const Icon(Icons.play_arrow_rounded),
                  ),
                ));
          } else {
            return const SizedBox();
          }
        });
  }
}
