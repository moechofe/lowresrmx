import 'package:flutter/material.dart';
import 'package:re_editor/re_editor.dart';

class MySearchPanel extends StatelessWidget implements PreferredSizeWidget {
  final CodeFindController controller;

  bool get isOpen => controller.value != null;
  bool get isReplaceMode => controller.value?.replaceMode ?? false;

  const MySearchPanel({required this.controller, super.key});

  @override
  Size get preferredSize => Size(
      double.infinity,
      (isReplaceMode)
          ? 124.0
          : isOpen
              ? 64.0
              : 0.0);

  @override
  Widget build(BuildContext context) {
    if (!isOpen) {
      return const SizedBox(width: 0.0, height: 0.0);
    } else {
      return Padding(
      	padding: const EdgeInsets.all(12.0),
      	child: Column(
      		children: [buildSearchRow(context), ...buildReplaceRow(context)],
      	),
      );
    }
  }

  Widget buildSearchRow(BuildContext context) {
    return Container(
			color: Theme.of(context).colorScheme.surfaceContainer,
			child: Row(children: [
				Expanded(
						child: TextField(
					controller: controller.findInputController,
					focusNode: controller.findInputFocusNode,
					decoration: const InputDecoration(
						border: OutlineInputBorder(),
						label: Text("Search"),
					),
				)),
				const SizedBox(width: 8.0),
				IconButton(
						onPressed: () {
							controller.nextMatch();
						},
						icon: const Icon(Icons.navigate_next_rounded))
			]),
		);
  }

  List<Widget> buildReplaceRow(BuildContext context) {
    if (!isReplaceMode) {
      return [];
    } else {
      return [
        const SizedBox(height: 12.0),
        Container(
					color: Theme.of(context).colorScheme.surfaceContainer,
					child: Row(
						children: [
							Expanded(
									child: TextField(
								controller: controller.replaceInputController,
												focusNode: controller.replaceInputFocusNode,
								decoration: const InputDecoration(
										border: OutlineInputBorder(), label: Text("Replace")),
							)),
							const SizedBox(width: 8.0),
							IconButton(
									onPressed: () {
														controller.replaceMatch();
													}, icon: const Icon(Icons.find_replace_rounded))
						],
					),
				)
      ];
    }
  }
}
