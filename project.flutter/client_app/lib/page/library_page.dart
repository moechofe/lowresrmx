import 'package:flutter/material.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/widget/library_grid.dart';

class MyLibraryPage extends StatefulWidget {
  const MyLibraryPage({super.key});

  @override
  State<MyLibraryPage> createState() => _MyLibraryPageState();
}

class _MyLibraryPageState extends State<MyLibraryPage> {
	Key refreshKey = UniqueKey();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("prg"), actions: [
        IconButton(
          onPressed: () {},
          tooltip: "Settings",
          icon: const Icon(Icons.more_vert_rounded),
        ),
        const SizedBox(width: 24.0),
      ]),
      floatingActionButton: FloatingActionButton.small(
        onPressed: () {
					MyLibrary.createProgram();
					setState((){
						refreshKey = UniqueKey();
					});
				},
        child: const Icon(Icons.add_rounded),
      ),
			body: MyCatalogGrid(key: refreshKey),
    );
  }
}
