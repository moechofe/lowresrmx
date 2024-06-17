import 'dart:developer';

import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/data/location.dart';
import 'package:lowresrmx/language.dart';
import 'package:lowresrmx/style.dart';
import 'package:lowresrmx/widget/error_gutter.dart';
import 'package:lowresrmx/widget/search_panel.dart';

class MyCodeEditor extends StatefulWidget {
  final CodeLineEditingController editingController;
  final CodeFindController findController;
  final CodeScrollController scrollController;
  final FocusNode focusNode;
  const MyCodeEditor(
      {required this.editingController,
      required this.findController,
      required this.scrollController,
      required this.focusNode,
      super.key});

  @override
  State<MyCodeEditor> createState() => _MyCodeEditorState();
}

class _MyCodeEditorState extends State<MyCodeEditor> {
  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final highlightStyle = Theme.of(context).brightness == Brightness.dark
        ? styleDark
        : styleLight;
    return CodeEditor(
      focusNode: widget.focusNode,
      autofocus: true,
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
        final ColorScheme colorScheme = Theme.of(context).colorScheme;
        return Row(
          children: [
            _buildErrorGutter(colorScheme, controller, notifier)
          ],
        );
      },
      findBuilder: (context, controller, readonly) {
        return MySearchPanel(controller: controller);
      },
    );
  }

  Widget _buildErrorGutter(
      ColorScheme colorScheme,
      CodeLineEditingController controller,
      ValueNotifier<CodeIndicatorValue?> notifier) {
    final MyContinousLocation continousLocation =
        context.watch<MyContinousLocation>();

    return MyErrorGutter(
        controller: controller,
        notifier: notifier,
				continousLocation: continousLocation,
        textStyle: const TextStyle(
          fontFamily: 'RecursiveLinear',
          fontSize: 16,
          height: 1.2,
        ),
        errorStyle: const TextStyle(
          fontFamily: 'RecursiveLinear',
          fontSize: 16,
          height: 1.2,
        ));
  }
}
