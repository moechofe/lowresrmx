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

const double listThumbnailExtent = 40.0;
const Key libraryItemThumbnailKey = Key('library-item-thumbnail');

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
	final MyLibraryGrid grid;

  const MyLibraryItem({
    required this.programName,
		required this.grid,
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

  Future<void> gotoEdit(BuildContext context) async {
    await Navigator.of(context).pushNamed(MyEditPage.routeName,
        arguments: {"programName": widget.programName});
    MyLibrary().refresh();
  }

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    // Will rebuild the widget when the preference is updated.
    final MyProgramPreference preference = context.read<MyProgramPreference>();
    return LayoutBuilder(builder: (context, constraints) {
      return Card(
        surfaceTintColor: colorScheme.surfaceBright,
        elevation: switch(widget.grid) {
					MyLibraryGrid.two => 2,
					MyLibraryGrid.three => 1,
					MyLibraryGrid.list => 0,
				},
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
        child: switch (widget.grid) {
          MyLibraryGrid.two || MyLibraryGrid.three => Column(
              mainAxisSize: MainAxisSize.min,
              children: [buildThumbnail(constraints), buildName()],
            ),
          MyLibraryGrid.list => Row(
              children: [
                Padding(
                    padding: const EdgeInsets.all(0.0),
                    child: buildThumbnail(constraints)),
                buildName(),
              ],
            ),
        },
      ),
    );
  }

  Widget buildName() {
    return Expanded(
        child: Align(
            alignment: Alignment.centerLeft,
            child: Padding(
              padding: switch(widget.grid) {
								MyLibraryGrid.two => EdgeInsets.only(
									left: 12.0, right: 12.0, top: 4.0, bottom: 4.0),
								MyLibraryGrid.three => EdgeInsets.only(
									left: 6.0, right: 6.0, top: 0.0, bottom: 0.0),
								MyLibraryGrid.list => const EdgeInsets.only(
									left: 8.0, right: 12.0)
							},
              child: Text(
                style: switch(widget.grid) {
									MyLibraryGrid.two => libraryItemTextStyle,
									MyLibraryGrid.three => libraryItemSmallTextStyle,
									MyLibraryGrid.list => libraryItemTextStyle
								},
                widget.programName,
                maxLines: switch(widget.grid) {
									MyLibraryGrid.two => 1,
									MyLibraryGrid.three => 2,
									MyLibraryGrid.list => 2
								},
                overflow: TextOverflow.ellipsis,
              ),
            )));
  }

  SizedBox buildThumbnail(BoxConstraints constraints) {
    final double extent = switch (widget.grid) {
      MyLibraryGrid.two || MyLibraryGrid.three => constraints.maxWidth - 8,
      MyLibraryGrid.list => listThumbnailExtent,
    };
    final double radius = switch (widget.grid) {
      MyLibraryGrid.two || MyLibraryGrid.three => 12.0,
      MyLibraryGrid.list => 8.0,
    };
    return SizedBox(
      key: libraryItemThumbnailKey,
      width: extent,
      height: extent,
      child: FutureBuilder(
          future: loadThumbnail(),
          builder: (context, snapshot) {
            if (snapshot.hasData) {
              return Opacity(
                opacity: Platform.isLinux ? 0.2 : 1.0,
                child: ClipRRect(
                  borderRadius: BorderRadius.circular(radius),
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
