import 'package:flutter/material.dart';
import 'package:lowresrmx/language.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/widget/search_panel.dart';
import 'package:lowresrmx/style.dart';

class MyCodeEditor extends StatefulWidget {
  final CodeLineEditingController editingController;
  final CodeFindController findController;
  const MyCodeEditor(
      {required this.editingController,
      required this.findController,
      super.key});

  @override
  State<MyCodeEditor> createState() => _MyCodeEditorState();
}

class _MyCodeEditorState extends State<MyCodeEditor> {
  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
		final highlightStyle = Theme.of(context).brightness == Brightness.dark ? styleDark : styleLight;
    return CodeEditor(
      controller: widget.editingController,
      findController: widget.findController,
      style: CodeEditorStyle(
        fontFamily: 'RecursiveLinear',
        fontSize: 16,
        fontHeight: 1.2,
        //chunkIndicatorColor: Colors.red,
        highlightColor: colorScheme.tertiaryContainer.withOpacity(0.5),
        //selectionColor: Colors.pink,
        cursorWidth: 4,
        cursorColor: colorScheme.onSurface,
        codeTheme: CodeHighlightTheme(
            theme: highlightStyle,
            languages: {'basic': CodeHighlightThemeMode(mode: langLowResRMX)}),
      ),
      indicatorBuilder: (context, controller, chunkController, notifier) {
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
