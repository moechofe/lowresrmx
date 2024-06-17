import 'dart:convert' show Base64Codec;
import 'dart:developer';
import 'dart:typed_data' show Uint8List;

import 'package:flutter/material.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/style.dart';
import 'package:provider/provider.dart';

final Uint8List transparentPng = const Base64Codec().decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABAQMAAAAl21bKAAAAA1BMVEUAAACnej3aAAAAAXRSTlMAQObYZgAAAApJREFUCNdjYAAAAAIAAeIhvDMAAAAASUVORK5CYII=");

enum MyItemMenuOption {
  isTool,
  rename,
  share,
  duplicate,
  delete,
}

/// A [Card] with thumbnail, name of a program, a popup menu and a tap action to open the program editor.
class MyLibraryItem extends StatefulWidget {
  final String programName;

  const MyLibraryItem({
    required this.programName,
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
    controller = TextEditingController(text: widget.programName);
  }

  Future<ImageProvider> _loadThumbnail() {
    return MyLibrary.readThumbnail(widget.programName);
  }

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    // Will rebuild the widget when the preference is updated.
    final MyProgramPreference preference = context.read<MyProgramPreference>();
    return LayoutBuilder(builder: (context, constraints) {
      return Card(
        surfaceTintColor: colorScheme.primaryContainer,
        elevation: 3,
        child: FutureBuilder<MyProgramPreference>(
            future: preference.loadPreference(),
            builder: (context, snapshot) {
              if (snapshot.hasData) {
                return _buildContent(
                    context, constraints, snapshot.data as MyProgramPreference);
              } else {
                return const SizedBox();
              }
            }),
      );
    });
  }

  GestureDetector _buildContent(BuildContext context,
      BoxConstraints constraints, MyProgramPreference preference) {
    // Unfortunatly, Inkwell does not support onLongPressStart with details, and I need the position to place the popup menu.
    return GestureDetector(
      onLongPressStart: (details) {
        _showPopupMenu(context, details, preference);
      },
      child: InkWell(
        onTap: () {
          Navigator.push(
              context,
              MaterialPageRoute(
                  builder: (context) =>
                      // Will show the program editor page for this program.
                      _gotoEditPage()));
        },
        borderRadius: BorderRadius.circular(12.0),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [_buildThumbnail(constraints), _buildName()],
        ),
      ),
    );
  }

  Widget _gotoEditPage() {
    return MyEditPage(programName: widget.programName);
  }

  Widget _buildName() {
    return Expanded(
        child: Align(
            alignment: Alignment.centerLeft,
            child: Padding(
              padding: const EdgeInsets.only(
                  left: 12.0, right: 12.0, top: 0.0, bottom: 4.0),
              child: Text(
                style: libraryItemTextStyle,
                widget.programName,
                maxLines: 1,
                overflow: TextOverflow.ellipsis,
              ),
            )));
  }

  SizedBox _buildThumbnail(BoxConstraints constraints) {
    return SizedBox(
      width: constraints.maxWidth,
      height: constraints.maxWidth,
      child: FutureBuilder(
          future: _loadThumbnail(),
          builder: (context, snapshot) {
            if (snapshot.hasData) {
              return Opacity(
                opacity: 0.2,
                child: ClipRRect(
                  borderRadius: BorderRadius.circular(12.0),
                  child: Image(
                      image: snapshot.data as ImageProvider,
                      fit: BoxFit.cover,
                      errorBuilder: (context, error, stackTrace) {
                        return Image(image: MemoryImage(transparentPng));
                      }),
                ),
              );
            } else {
              return const SizedBox();
            }
          }),
    );
  }

  void _showPopupMenu(BuildContext context, LongPressStartDetails details,
      MyProgramPreference preference) {
    showMenu<MyItemMenuOption?>(
      context: context,
      position: RelativeRect.fromLTRB(
        details.globalPosition.dx,
        details.globalPosition.dy,
        details.globalPosition.dx,
        details.globalPosition.dy,
      ),
      items: [
        const PopupMenuItem<MyItemMenuOption>(
          value: MyItemMenuOption.rename,
          child: ListTile(
            leading: Icon(Icons.drive_file_rename_outline_rounded),
            title: Text('Rename'),
          ),
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
          child: ListTile(leading: Icon(Icons.delete), title: Text('Delete')),
        ),
        const PopupMenuDivider(),
        CheckedPopupMenuItem<MyItemMenuOption>(
            value: MyItemMenuOption.isTool,
            checked: preference.isTool,
            child: const Text('Is Tool')),
      ],
    ).then((value) {
      if (value == null) return;
      switch (value) {
        case MyItemMenuOption.isTool:
          preference.setTool(!preference.isTool);
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
      }
    });
  }

  void _showRenameDialog() async {
    String? newName = await showDialog<String?>(
      context: context,
      builder: (BuildContext context) {
        String newName = widget.programName;
        return AlertDialog(
          icon: const Icon(Icons.drive_file_rename_outline_rounded),
          title: const Text("Rename"),
          content: TextField(
            controller: controller,
            autofocus: true,
            onChanged: (value) {
              newName = value;
            },
            decoration: const InputDecoration(
              labelText: "New Name",
            ),
          ),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context),
              child: const Text("Cancel"),
            ),
            TextButton(
              onPressed: () => Navigator.pop(context, newName),
              child: const Text("Rename"),
            ),
          ],
        );
      },
    );
    if (newName != null) {
      MyLibrary.renameProgram(widget.programName, newName);
      MyPreference.renameProgram(widget.programName, newName);
    }
  }

  void _showDeleteDialog() async {
    bool? confirmDelete = await showDialog<bool>(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          icon: const Icon(Icons.delete),
          title: const Text("Delete Program"),
          content: const Text("Are you sure you want to delete this program?"),
          actions: <Widget>[
            TextButton(
              onPressed: () => Navigator.pop(context, false),
              child: const Text("Cancel"),
            ),
            TextButton(
              onPressed: () => Navigator.pop(context, true),
              child: const Text("Delete"),
            ),
          ],
        );
      },
    );
    if (confirmDelete == true) {
      MyLibrary.deleteProgram(widget.programName);
      MyPreference.deleteProgram(widget.programName);
    }
  }
}
