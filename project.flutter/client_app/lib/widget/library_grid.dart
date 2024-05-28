import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/widget/library_item.dart';

// TODO: add sort options

class MyLibraryGridDelegate extends SliverGridDelegate {
  @override
  SliverGridLayout getLayout(SliverConstraints constraints) {
    int countPerRow = constraints.crossAxisExtent ~/ 150;
    if (countPerRow < 1) countPerRow = 1;
    final double cross = constraints.crossAxisExtent / countPerRow;
    final double main = cross + 48; // 48 is the height of the ListTile
    return SliverGridRegularTileLayout(
      childMainAxisExtent: main,
      childCrossAxisExtent: cross,
      crossAxisCount: countPerRow,
      mainAxisStride: main,
      crossAxisStride: cross,
      reverseCrossAxis: false,
    );
  }

  @override
  bool shouldRelayout(covariant MyLibraryGridDelegate oldDelegate) {
    return false;
  }
}

class MyCatalogGrid extends StatelessWidget {
  final MyLibrarySort sort;
  const MyCatalogGrid({required this.sort, super.key});

  @override
  Widget build(BuildContext context) {
		// Make sure to rebuild the grid when a program is created, removed or renamed.
    final MyLibrary _ = context.watch<MyLibrary>();
    return FutureBuilder(
        future: MyLibrary.buildList(sort),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return GridView.builder(
                padding: const EdgeInsets.only(
                    left: 12.0, right: 12.0, bottom: 24.0),
                gridDelegate: MyLibraryGridDelegate(),
                itemCount: snapshot.data!.length,
                itemBuilder: (context, index) => ChangeNotifierProvider(
									// Used to make sure to rebuild the item when a preference changes.
									create: (_) => MyProgramPreference(snapshot.data![index]),
									child: MyLibraryItem(
											program: snapshot.data![index],
											// The key is used to identify the item in the list when program are added and removed.
											key: ValueKey(snapshot.data![index])),
								));
          } else {
            // TODO: report error
            // TODO: Do it better
            return const SizedBox();
          }
        });
  }
}
