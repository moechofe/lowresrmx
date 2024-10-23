import 'package:flutter/material.dart';
import 'package:lowresrmx/data/preference.dart';

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
	final MyEditorPreference editorSettings;
  const MyCodeEditor(
      {required this.editingController,
      required this.findController,
      required this.scrollController,
      required this.focusNode,
			required this.editorSettings,
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
			scrollController: widget.scrollController,
      style: CodeEditorStyle(
        fontFamily: 'RecursiveLinear',
        fontSize: widget.editorSettings.fontSize,
        fontHeight: 1.2,
        highlightColor: colorScheme.tertiaryContainer.withOpacity(0.5),
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
            buildErrorGutter(colorScheme, controller, notifier)
          ],
        );
      },
      findBuilder: (context, controller, readonly) {
        return MySearchPanel(controller: controller);
      },
    );
  }

  Widget buildErrorGutter(
      ColorScheme colorScheme,
      CodeLineEditingController controller,
      ValueNotifier<CodeIndicatorValue?> notifier) {
    final MyContinousLocation continousLocation =
        context.watch<MyContinousLocation>();

    return MyErrorGutter(
        controller: controller,
        notifier: notifier,
				continousLocation: continousLocation,
        textStyle: TextStyle(
          fontFamily: 'RecursiveLinear',
          fontSize: widget.editorSettings.fontSize,
          height: 1.2,
        ),
        errorStyle: TextStyle(
          fontFamily: 'RecursiveLinear',
          fontSize: widget.editorSettings.fontSize,
          height: 1.2,
        ));
  }
}
