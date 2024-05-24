import 'dart:developer' show log;

import 'package:flutter/material.dart';
import 'package:re_editor/re_editor.dart';

class MySearchPanel extends StatelessWidget implements PreferredSizeWidget {
  final CodeFindController controller;

  bool get isOpen => controller.value != null;
  bool get isReplaceMode => controller.value?.replaceMode ?? false;

  const MySearchPanel({required this.controller, super.key});

  @override
  Size get preferredSize {
    log("MySearchPanel.preferredSize ${controller.value}");
		return Size(double.infinity, (isOpen && isReplaceMode) ? 124.0 : isOpen ? 64.0 : 0.0);
  }

  @override
  Widget build(BuildContext context) {
    log("MySearchPanel.build");
    if (!isOpen) {
      return const SizedBox(width: 0.0, height: 0.0);
    } else {
      return Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(
          children: [
            buildSearchRow(),
            ... buildReplaceRow()
          ],
        ),
      );
    }
  }

  List<Widget> buildReplaceRow() {
		if (!isReplaceMode) {
			return [];
		} else {
    return [
			const SizedBox(height: 12.0),
			Row(
      children: [
        Expanded(
            child: TextField(
          decoration: InputDecoration(
              border: OutlineInputBorder(), label: Text("Replace")),
        )),
        const SizedBox(width: 8.0),
        IconButton(
            onPressed: () {}, icon: const Icon(Icons.find_replace_rounded))
      ],
    )];
		}
  }

  Widget buildSearchRow() {
    return Row(children: [
      Expanded(
          child: TextField(
        decoration: InputDecoration(
          border: OutlineInputBorder(),
          label: Text("Search"),
          // prefixIcon: Padding(
          //     padding: EdgeInsets.fromLTRB(12, 10, 0, 5),
          //     child: Text("/")),
        ),
      )),
      const SizedBox(width: 8.0),
      IconButton(
          onPressed: () {}, icon: const Icon(Icons.navigate_next_rounded))
    ]);
  }

}
