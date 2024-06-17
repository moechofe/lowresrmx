import 'dart:developer';

import 'package:flutter/material.dart';

import 'package:file_picker/file_picker.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/manual_page.dart';
// import 'package:lowresrmx/widget/google_tile.dart';
import 'package:lowresrmx/widget/library_grid.dart';

enum MyLibraryMenuOption {
  setting,
  name,
  oldest,
  newest,
}

class MyLibraryPage extends StatefulWidget {
  const MyLibraryPage({super.key});

  @override
  State<MyLibraryPage> createState() => _MyLibraryPageState();
}

class _MyLibraryPageState extends State<MyLibraryPage> {
  MyLibrarySort sort = MyLibrarySort.name;

  Widget buildMorePopupMenu(BuildContext context) {
    return PopupMenuButton<MyLibraryMenuOption>(
      itemBuilder: (BuildContext context) =>
          <PopupMenuEntry<MyLibraryMenuOption>>[
        const PopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.setting,
          child: Text('Editor settings'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.name,
          checked: sort == MyLibrarySort.name,
          child: const Text('Sort by name'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.oldest,
          checked: sort == MyLibrarySort.oldest,
          child: const Text('Sort by oldest'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.newest,
          checked: sort == MyLibrarySort.newest,
          child: const Text('Sort by newest'),
        ),
      ],
      onSelected: (MyLibraryMenuOption value) {
        switch (value) {
          case MyLibraryMenuOption.setting:
            // TODO: open settings
            break;
          case MyLibraryMenuOption.name:
            setState(() {
              sort = MyLibrarySort.name;
            });
            break;
          case MyLibraryMenuOption.oldest:
            setState(() {
              sort = MyLibrarySort.oldest;
            });
            break;
          case MyLibraryMenuOption.newest:
            setState(() {
              sort = MyLibrarySort.newest;
            });
            break;
        }
        // Handle menu item selection
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(title: const Text("Programs"), actions: [
          buildMorePopupMenu(context),
          const SizedBox(width: 24.0),
        ]),
        drawer: buildDrawer(context),
        floatingActionButton: FloatingActionButton.small(
          onPressed: () async {
            MyLibrary.createProgram();
          },
          child: const Icon(Icons.add_rounded),
        ),
        body: MyCatalogGrid(sort: sort));
  }

  Widget buildDrawer(BuildContext context) {
    return Drawer(
      child: ListView(
        children: [
          // TODO: About tile

          const MyManualTile(),
          const Divider(),

          buildFolder(),

					// const MyGoogleTile(),
        ],
      ),
    );
  }

  Widget buildFolder() {
    return ListTile(
      leading: const Icon(Icons.folder_rounded),
      title: const Text("Change program location"),
      subtitle: FutureBuilder<String>(
        future: MyPreference.getProgramDirectory(),
        builder: (BuildContext context, AsyncSnapshot<String> snapshot) {
          if (snapshot.hasData) {
            return Text(snapshot.data!, overflow: TextOverflow.ellipsis);
          } else {
            return const Text("…");
          }
        },
      ),
      onTap: () async {
        String? selectedDirectory = await FilePicker.platform.getDirectoryPath(
            initialDirectory: await MyPreference.getProgramDirectory());
        if (selectedDirectory != null &&
            selectedDirectory != await MyPreference.getProgramDirectory()) {
          await MyPreference.setProgramDirectory(selectedDirectory);
          setState(() {});
        }
        log("Selected directory: $selectedDirectory");
      },
    );
  }
}
