import 'dart:developer';

import 'package:flutter/material.dart';
import 'package:re_editor/re_editor.dart';

class MyKeyboardKey extends StatelessWidget {
  final bool enabled;
  final IconData icon;
  final VoidCallback onPressed;

  const MyKeyboardKey(
      {required this.icon,
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
          label: Icon(icon, size: 22),
          labelPadding: EdgeInsets.zero,
          side: const BorderSide(color: Colors.transparent),
    	color: WidgetStateProperty.all(
    		Theme.of(context).colorScheme.brightness == Brightness.light ? const Color(0xFFF6EAE7) : const Color(0xFF2b1d1a)
    	)
        ));
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

  @override
  Widget build(BuildContext context) {
    return Container(
      width: double.infinity,
			color: Theme.of(context).colorScheme.brightness == Brightness.light ? const Color(0xFFF6EAE7) : const Color(0xFF2b1d1a),
      child: CodeEditorTapRegion(
          child: SingleChildScrollView(
        scrollDirection: Axis.horizontal,
        child: Padding(
          padding: const EdgeInsets.all(4.0),
          child: Row(
          		children: [
          								MyKeyboardKey(
          									icon: Icons.undo_rounded,
          									onPressed: () => widget.controller.undo(),
          									enabled: widget.controller.canUndo),
          								MyKeyboardKey(
          									icon: Icons.redo_rounded,
          									onPressed: () => widget.controller.redo(),
          									enabled: widget.controller.canRedo,
          								),
          			MyKeyboardKey(
          					icon: Icons.content_cut_rounded,
          					onPressed: () => widget.controller.cut(),
          					enabled: !widget.controller.selection.isCollapsed),
          								MyKeyboardKey(
          										icon: Icons.content_copy_rounded,
          										onPressed: () => widget.controller.copy(),
          										enabled: !widget.controller.selection.isCollapsed),
          								MyKeyboardKey(
          										icon: Icons.content_paste_rounded,
          										onPressed: () => widget.controller.paste(),
          										enabled: true),
          		]),
        ),
      )),
    );
  }
}
