import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'package:re_editor/re_editor.dart';

/// A search icon that changes its appearance based on the state of the [CodeFindController].
class MySearchIcon extends StatelessWidget {
  static const Widget searchIcon = Icon(Icons.search_rounded);
  static const Widget replaceIcon = Icon(Icons.find_replace_rounded);
  static const Widget closeIcon = Icon(Icons.search_off_rounded);

  const MySearchIcon({super.key});

  @override
  Widget build(BuildContext context) {
    final CodeFindController controller = context.watch<CodeFindController>();

    bool isOpen = controller.value != null;
    bool isReplaceMode = controller.value?.replaceMode ?? false;

    late final Widget icon;
    if (isReplaceMode) {
      icon = closeIcon;
    } else if (isOpen) {
      icon = replaceIcon;
    } else {
      icon = searchIcon;
    }

    return IconButton(
        onPressed: () {
          if (isReplaceMode) {
            controller.close();
          } else if (isOpen) {
            controller.replaceMode();
          } else {
            controller.findMode();
          }
        },
				tooltip: "Search and replace",
        icon: icon);
  }
}
