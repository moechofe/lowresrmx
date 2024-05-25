import 'package:flutter/material.dart';

import 'package:lowresrmx/data/library.dart';
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

	Widget _buildMorePopupMenu(BuildContext context) {
		return PopupMenuButton<MyLibraryMenuOption>(
			itemBuilder: (BuildContext context) => <PopupMenuEntry<MyLibraryMenuOption>>[
				const PopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.setting,
					child: Text('Editor settings'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.name,
					checked: sort==MyLibrarySort.name,
					child: const Text('Sort by name'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.oldest,
					checked: sort==MyLibrarySort.oldest,
					child: const Text('Sort by oldest'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.newest,
					checked: sort==MyLibrarySort.newest,
					child: const Text('Sort by newest'),
				),
			],
			onSelected: (MyLibraryMenuOption value) {
				switch(value) {
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
        _buildMorePopupMenu(context),
        const SizedBox(width: 24.0),
      ]),
      floatingActionButton: FloatingActionButton.small(
        onPressed: () async {
					MyLibrary.createProgram();
				},
        child: const Icon(Icons.add_rounded),
      ),
			body: MyCatalogGrid(sort: sort)
    );
  }
}
