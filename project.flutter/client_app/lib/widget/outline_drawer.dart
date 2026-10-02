import 'package:flutter/material.dart';
import 'package:re_editor/re_editor.dart';

import 'package:lowresrmx/core/outline.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/outline_entry.dart';

enum MyOutlineSort {
  position,
  alphabetic,
}

typedef GotoLocationCallBack = void Function(Location location);

class MyOutlineDrawer extends StatefulWidget {
  final CodeLineEditingController editingController;
  final TextEditingController searchController;
  final GotoLocationCallBack gotoLocation;

  const MyOutlineDrawer(
      {required this.editingController,
      required this.searchController,
      required this.gotoLocation,
      super.key});

  @override
  State<MyOutlineDrawer> createState() => _MyOutlineDrawerState();
}

class _MyOutlineDrawerState extends State<MyOutlineDrawer> {
  MyOutlineSort sort = MyOutlineSort.position;

  late final List<OutlineEntry> entries;

  TextEditingController get searchController => widget.searchController;

  @override
  void initState() {
    super.initState();
    entries = scanOutline(widget.editingController.text);
    searchController.addListener(onSearchChanged);
  }

  @override
  void didUpdateWidget(MyOutlineDrawer oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.searchController != widget.searchController) {
      oldWidget.searchController.removeListener(onSearchChanged);
      widget.searchController.addListener(onSearchChanged);
    }
  }

  @override
  void dispose() {
    searchController.removeListener(onSearchChanged);
    super.dispose();
  }

  void onSearchChanged() {
    setState(() {});
  }

  List<OutlineEntry> sortList(List<OutlineEntry> entries) {
    switch (sort) {
      case MyOutlineSort.position:
        // scanOutline already returns them in source order.
        return entries;
      case MyOutlineSort.alphabetic:
        return List.of(entries)
          ..sort((a, b) =>
              a.identifier.toLowerCase().compareTo(b.identifier.toLowerCase()));
    }
  }

  List<OutlineEntry> filterList(List<OutlineEntry> entries) {
    final query = searchController.text.trim().toLowerCase();
    if (query.isEmpty) {
      return entries;
    }
    return entries
        .where((entry) => entry.identifier.toLowerCase().contains(query))
        .toList();
  }

  void gotoEntry(BuildContext context, OutlineEntry entry) {
    Navigator.of(context).pop();
    widget.gotoLocation(entry.location);
  }

  @override
  Widget build(BuildContext context) {
    final List<OutlineEntry> visibleEntries = filterList(sortList(entries));

    return Drawer(
        child: Scaffold(
            appBar: buildAppBar(context),
            body: visibleEntries.isEmpty
                ? Center(
                    child: Text(entries.isEmpty
                        ? "No label, subroutine, mark or ROM entry"
                        : "No match"))
                : ListView.builder(
                    itemCount: visibleEntries.length,
                    itemBuilder: (context, index) {
                      final entry = visibleEntries[index];
                      return ListTile(
                          leading: Icon(switch (entry.kind) {
                            OutlineKind.sub => Icons.functions_rounded,
                            OutlineKind.mark => Icons.bookmark_rounded,
                            OutlineKind.label => Icons.label_outline_rounded,
                            OutlineKind.rom => Icons.memory_rounded,
                          }),
                          title: Text(entry.identifier,
                              style: entry.kind == OutlineKind.mark
                                  ? const TextStyle(fontWeight: FontWeight.bold)
                                  : null),
                          trailing: Text("${entry.location.row + 1}"),
                          dense: true,
                          onTap: () {
                            gotoEntry(context, entry);
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
                      value: MyOutlineSort.position,
                      checked: sort == MyOutlineSort.position,
                      child: const Text("Position")),
                  CheckedPopupMenuItem(
                      value: MyOutlineSort.alphabetic,
                      checked: sort == MyOutlineSort.alphabetic,
                      child: const Text("Alphabetic")),
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
      autofocus: false,
      controller: searchController,
      decoration: InputDecoration(
        hintText: "Search",
        hintStyle: TextStyle(
            color:
                Theme.of(context).colorScheme.onSurface.withValues(alpha: 0.5)),
        border: InputBorder.none,
        icon: const Icon(Icons.search_rounded),
      ),
      onChanged: (value) {},
    );
  }
}
