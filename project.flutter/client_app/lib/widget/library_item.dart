import 'dart:convert' show Base64Codec;
import 'dart:io';
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
  // isTool,
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
  late final TextEditingController renameController;

  @override
  void initState() {
    super.initState();
    renameController = TextEditingController(text: widget.programName);
  }

  Future<ImageProvider> loadThumbnail() {
    return MyLibrary.readThumbnail(widget.programName);
  }

  void gotoEdit(BuildContext context) {
    Navigator.of(context).pushReplacementNamed(MyEditPage.routeName,
        arguments: {"programName": widget.programName});
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
                return buildContent(
                    context, constraints, snapshot.data as MyProgramPreference);
              } else {
                return const SizedBox();
              }
            }),
      );
    });
  }

  Widget buildContent(BuildContext context, BoxConstraints constraints,
      MyProgramPreference preference) {
    // Unfortunatly, Inkwell does not support onLongPressStart with details, and I need the position to place the popup menu.
    return GestureDetector(
      onLongPressStart: (details) {
        showPopupMenu(context, details, preference);
      },
      child: InkWell(
        onTap: () {
          gotoEdit(context);
        },
        borderRadius: BorderRadius.circular(12.0),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [buildThumbnail(constraints), buildName()],
        ),
      ),
    );
  }

  Widget buildName() {
    return Expanded(
        child: Align(
            alignment: Alignment.centerLeft,
            child: Padding(
              padding: const EdgeInsets.only(
                  left: 12.0, right: 12.0, top: 4.0, bottom: 4.0),
              child: Text(
                style: libraryItemTextStyle,
                widget.programName,
                maxLines: 1,
                overflow: TextOverflow.ellipsis,
              ),
            )));
  }

  SizedBox buildThumbnail(BoxConstraints constraints) {
    return SizedBox(
      width: constraints.maxWidth - 8,
      height: constraints.maxWidth - 8,
      child: FutureBuilder(
          future: loadThumbnail(),
          builder: (context, snapshot) {
            if (snapshot.hasData) {
              return Opacity(
                opacity: Platform.isLinux ? 0.2 : 1.0,
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

  void showPopupMenu(BuildContext context, LongPressStartDetails details,
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
        // const PopupMenuDivider(),
        // CheckedPopupMenuItem<MyItemMenuOption>(
        //     value: MyItemMenuOption.isTool,
        //     checked: preference.isTool,
        //     child: const Text('Is Tool')),
      ],
    ).then((value) {
      if (value == null) return;
      switch (value) {
        // case MyItemMenuOption.isTool:
        //   preference.setTool(!preference.isTool);
        //   break;
        case MyItemMenuOption.rename:
          showRenameDialog();
          break;
        case MyItemMenuOption.share:
          break;
        case MyItemMenuOption.duplicate:
          break;
        case MyItemMenuOption.delete:
          showDeleteDialog();
          break;
      }
    });
  }

  void showRenameDialog() async {
    String? newName = await showDialog<String?>(
      context: context,
      builder: (BuildContext context) {
        String newName = widget.programName;
        return AlertDialog(
            icon: const Icon(Icons.drive_file_rename_outline_rounded),
            title: const Text("Rename"),
            content: TextField(
              controller: renameController,
              autofocus: true,
              onChanged: (value) => newName = value,
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
              )
            ]);
      },
    );
    if (newName != null) {
      MyLibrary.renameProgram(widget.programName, newName);
      MyPreference.renameProgram(widget.programName, newName);
    }
  }

  void showDeleteDialog() async {
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
