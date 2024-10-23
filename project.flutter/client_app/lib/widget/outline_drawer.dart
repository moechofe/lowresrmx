import 'package:flutter/material.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/outline_entry.dart';

enum MyOutlineSort {
  position,
  alphabetic,
}

typedef GotoLocationCallBack = void Function(Location location);

class MyOutlineDrawer extends StatefulWidget {
  final String program;
  final List<OutlineEntry> entries;
  final GotoLocationCallBack gotoLocation;

  const MyOutlineDrawer(
      {required this.program,
      required this.entries,
      required this.gotoLocation,
      super.key});

  @override
  State<MyOutlineDrawer> createState() => _MyOutlineDrawerState();
}

class _MyOutlineDrawerState extends State<MyOutlineDrawer> {
  MyOutlineSort sort = MyOutlineSort.position;

  bool searchMode = false;

  final TextEditingController searchController = TextEditingController();

  @override
  void initState() {
    super.initState();
    searchController.addListener(() {
      setState(() {});
    });
  }

  @override
  void dispose() {
    searchController.dispose();
    super.dispose();
  }

  List<OutlineEntry> sortList(List<OutlineEntry> entries) {
    switch (sort) {
      case MyOutlineSort.position:
        return List.from(entries)
          ..sort((a, b) => a.position.compareTo(b.position));
      case MyOutlineSort.alphabetic:
        return List.from(entries)
          ..sort((a, b) => a.identifier.compareTo(b.identifier));
    }
  }

  List<OutlineEntry> filterList(List<OutlineEntry> entries) {
    final query = searchController.text;
    if (query.isEmpty) {
      return entries;
    }
    return entries
        .where((element) =>
            element.identifier.contains(RegExp(query, caseSensitive: false)))
        .toList();
  }

  void gotoPosition(BuildContext context, int position) {
    final location = Location.fromCode(widget.program, position);
    Navigator.of(context).pop();
    widget.gotoLocation(location);
  }

  @override
  Widget build(BuildContext context) {
    final sortedEntries = sortList(widget.entries);
    final filteredEntries = filterList(sortedEntries);

    return Drawer(
        child: Scaffold(
            appBar: buildAppBar(context),
            body: ListView.builder(
                itemCount: filteredEntries.length,
                itemBuilder: (context, index) {
                  final entry = filteredEntries[index];
                  return ListTile(
                      title: Text(entry.identifier),
                      dense: true,
                      onTap: () {
                        gotoPosition(context, entry.position);
                      });
                })));
  }

  PreferredSizeWidget buildAppBar(BuildContext context) {
    return AppBar(
      actions: [
				PopupMenuButton<MyOutlineSort>(
					icon: const Icon(Icons.sort_rounded),
					itemBuilder: (BuildContext context) => [
            CheckedPopupMenuItem(
								value: MyOutlineSort.position, checked: sort == MyOutlineSort.position, child: const Text("Position")),
						CheckedPopupMenuItem(
								value: MyOutlineSort.alphabetic, checked: sort == MyOutlineSort.alphabetic, child: const Text("Alphabetic")),
					],
					onSelected: (value) {
						setState(() {
							sort = value;
						});
					}),
        const SizedBox(width: 8.0),
      ],
      title: buildSearchBar(context),
    );
  }

  Widget buildSearchBar(BuildContext context) {
    return TextField(
			autofocus: true,
      controller: searchController,
      decoration: InputDecoration(
        hintText: "Search",
        hintStyle: TextStyle(
            color: Theme.of(context).colorScheme.onSurface.withOpacity(0.5)),
        border: InputBorder.none,
				icon: const Icon(Icons.search_outlined),
      ),
      onChanged: (value) {},
    );
  }
}
