import 'dart:ui' show PointerDeviceKind;

import 'package:flutter/material.dart';
import 'package:re_editor/re_editor.dart';

const List<String> _symbolKeys = [ '_', '"', '(', ')', '=', ':', '\$', "'", '+', '-', '*', '/', '<', '>', ';', '%', '^', '\\', '#' ];

class MyKeyboardKey extends StatelessWidget {
  final bool enabled;
  final Widget label;
  final VoidCallback onPressed;

  const MyKeyboardKey(
      {required this.label,
      required this.onPressed,
      required this.enabled,
      super.key});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
        width: 38,
        height: 38,
        child: ActionChip(
            onPressed: enabled ? onPressed : null,
            label: label,
            labelPadding: EdgeInsets.zero,
            side: const BorderSide(color: Colors.transparent),
            color: WidgetStateProperty.all(
                Theme.of(context).colorScheme.brightness == Brightness.light
                    ? const Color(0xFFF6EAE7)
                    : const Color(0xFF2b1d1a))));
  }
}

class MyKeyboardBar extends StatefulWidget {
  final CodeLineEditingController controller;
  final FocusNode focusNode;

  const MyKeyboardBar(
      {required this.controller, required this.focusNode, super.key});

  @override
  State<StatefulWidget> createState() => _MyKeyboardBarState();
}

class _MyKeyboardBarState extends State<MyKeyboardBar> {
  @override
  void initState() {
    super.initState();
    widget.controller.addListener(controllerHasChanged);
  }

  @override
  void dispose() {
    widget.controller.removeListener(controllerHasChanged);
    super.dispose();
  }

  void controllerHasChanged() {
    setState(() {});
  }

  Widget _symbolKey(String symbol) {
    return MyKeyboardKey(
        label: Text(symbol,
            style: const TextStyle(fontFamily: 'RecursiveLinear', fontSize: 20)),
        onPressed: () => widget.controller.replaceSelection(symbol),
        enabled: true);
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      width: double.infinity,
      color: Theme.of(context).colorScheme.brightness == Brightness.light
          ? const Color(0xFFF6EAE7)
          : const Color(0xFF2b1d1a),
      child: CodeEditorTapRegion(
          child: ScrollConfiguration(
        behavior: ScrollConfiguration.of(context).copyWith(dragDevices: {
          PointerDeviceKind.touch,
          PointerDeviceKind.stylus,
          PointerDeviceKind.trackpad,
          PointerDeviceKind.mouse
        }),
        child: SingleChildScrollView(
          scrollDirection: Axis.horizontal,
          child: Padding(
            padding: const EdgeInsets.all(4.0),
            child: Row(children: [
              MyKeyboardKey(
                  label: const Icon(Icons.undo_rounded, size: 22),
                  onPressed: () => widget.controller.undo(),
                  enabled: widget.controller.canUndo),
              MyKeyboardKey(
                label: const Icon(Icons.redo_rounded, size: 22),
                onPressed: () => widget.controller.redo(),
                enabled: widget.controller.canRedo,
              ),
              MyKeyboardKey(
                  label: const Icon(Icons.content_cut_rounded, size: 22),
                  onPressed: () => widget.controller.cut(),
                  enabled: !widget.controller.selection.isCollapsed),
              MyKeyboardKey(
                  label: const Icon(Icons.content_copy_rounded, size: 22),
                  onPressed: () => widget.controller.copy(),
                  enabled: !widget.controller.selection.isCollapsed),
              MyKeyboardKey(
                  label: const Icon(Icons.content_paste_rounded, size: 22),
                  onPressed: () => widget.controller.paste(),
                  enabled: true),
              const SizedBox(width: 8),
              for (final symbol in _symbolKeys) _symbolKey(symbol),
            ]),
          ),
        ),
      )),
    );
  }
}
