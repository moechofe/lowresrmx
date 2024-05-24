import 'package:flutter/material.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/widget/search_panel.dart';

class MyCodeEditor extends StatefulWidget {
	final CodeLineEditingController controller;
  const MyCodeEditor({required this.controller, super.key});

  @override
  State<MyCodeEditor> createState() => _MyCodeEditorState();
}

class _MyCodeEditorState extends State<MyCodeEditor> {
  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    return CodeEditor(
      controller: widget.controller,
      style: CodeEditorStyle(
          fontFamily: 'RecursiveLinear',
          fontSize: 16,
          fontHeight: 1.1,
          chunkIndicatorColor: Colors.red,
          highlightColor: Colors.purple,
          selectionColor: Colors.pink,
          cursorWidth: 4,
          cursorColor: colorScheme.onSurface),
      indicatorBuilder:
          (context, controller, chunkController, notifier) {
        return Row(
          children: [
            DefaultCodeLineNumber(
                notifier: notifier,
                controller: controller,
                textStyle: const TextStyle(
                  fontFamily: 'RecursiveLinear',
                  fontSize: 16,
                )),
            DefaultCodeChunkIndicator(
                width: 10, controller: chunkController, notifier: notifier),
          ],
        );
      },
			findBuilder: (context, controller, readonly) {
			  return MySearchPanel(controller: controller);
			},
    );
  }
}
