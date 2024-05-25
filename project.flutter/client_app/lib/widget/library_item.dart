import 'package:flutter/material.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';

enum MyItemMenuOption {
  isTool,
  rename,
  share,
  duplicate,
  delete,
}

class MyLibraryItem extends StatefulWidget {
  final String program;

  const MyLibraryItem({
    required this.program,
    super.key,
  });

  @override
  State<MyLibraryItem> createState() => _MyLibraryItemState();
}

class _MyLibraryItemState extends State<MyLibraryItem> {
  late final TextEditingController controller;

  @override
  void initState() {
    super.initState();
    controller = TextEditingController(text: widget.program);
  }

  void _showRenameDialog() async {
    String? newName = await showDialog<String?>(
      context: context,
      builder: (BuildContext context) {
        String newName = widget.program;
        return AlertDialog(
          icon: const Icon(Icons.drive_file_rename_outline_rounded),
          title: const Text('Rename'),
          content: TextField(
            controller: controller,
            autofocus: true,
            onChanged: (value) {
              newName = value;
            },
            decoration: const InputDecoration(
              labelText: 'New Name',
            ),
          ),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text('Cancel'),
            ),
            TextButton(
              onPressed: () => Navigator.pop(context, newName),
              child: const Text('Rename'),
            ),
          ],
        );
      },
    );
    if (newName != null) {
      MyLibrary.renameProgram(widget.program, newName);
    }
  }

  void _showDeleteDialog() async {
    bool? confirmDelete = await showDialog<bool>(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          icon: const Icon(Icons.delete),
          title: const Text('Delete Program'),
          content: const Text('Are you sure you want to delete this program?'),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context, false),
              child: const Text('Cancel'),
            ),
            TextButton(
              onPressed: () => Navigator.pop(context, true),
              child: const Text('Delete'),
            ),
          ],
        );
      },
    );
    if (confirmDelete == true) {
      MyLibrary.deleteProgram(widget.program);
    }
  }

  void _showPopupMenu(LongPressStartDetails details) async {
		bool isTool = await MyPreference.programIsTool(widget.program);
    switch (await showMenu<MyItemMenuOption?>(
      context: context,
      position: RelativeRect.fromLTRB(
        details.globalPosition.dx,
        details.globalPosition.dy,
        details.globalPosition.dx,
        details.globalPosition.dy,
      ),
      items: [
        CheckedPopupMenuItem<MyItemMenuOption>(
          value: MyItemMenuOption.isTool,
					checked: isTool,
					child: const Text('Is Tool')),
        const PopupMenuItem<MyItemMenuOption>(
          value: MyItemMenuOption.rename,
          child: Text('Rename'),
        ),
        // const PopupMenuItem<MyItemMenuOption>(
        //   value: MyItemMenuOption.share,
        //   child: Text('Share'),
        // ),
        // const PopupMenuItem<MyItemMenuOption>(
        //   value: MyItemMenuOption.duplicate,
        //   child: Text('Duplicate'),
        // ),
        const PopupMenuItem<MyItemMenuOption>(
          value: MyItemMenuOption.delete,
          child: Text('Delete'),
        ),
      ],
    )) {
			case MyItemMenuOption.isTool:
				MyPreference.setProgramIsTool(widget.program, !isTool);
				break;
      case MyItemMenuOption.rename:
        _showRenameDialog();
        break;
      case MyItemMenuOption.share:
        break;
      case MyItemMenuOption.duplicate:
        break;
      case MyItemMenuOption.delete:
        _showDeleteDialog();
        break;
      default:
        break;
    }
  }

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    return Card(
      surfaceTintColor: colorScheme.primaryContainer,
      elevation: 3,
      child: GestureDetector(
        onLongPressStart: _showPopupMenu,
        child: InkWell(
          onTap: () {
            Navigator.push(
                context,
                MaterialPageRoute(
                    builder: (context) =>
                        MyEditPage(programName: widget.program)));
          },
          borderRadius: BorderRadius.circular(12.0),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              ListTile(
                title: Text(widget.program),
              )
            ],
          ),
        ),
      ),
    );
  }
}
