import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/widget/library_item.dart';

class MyLibraryGridDelegate extends SliverGridDelegate {
	final int countPerRow;
	final int titleHeight;

	MyLibraryGridDelegate(MyLibraryGrid grid)
			: countPerRow = switch (grid) {
					MyLibraryGrid.two => 2,
					MyLibraryGrid.three => 3,
				}, titleHeight = switch(grid){
					MyLibraryGrid.two => 40,
					MyLibraryGrid.three => 50,
				};

  @override
  SliverGridLayout getLayout(SliverConstraints constraints) {
    final double cross = constraints.crossAxisExtent / countPerRow;
    final double main = cross + titleHeight;
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
    return oldDelegate.countPerRow != countPerRow;
  }
}

class MyCatalogGrid extends StatelessWidget {
  final MyLibrarySort sort;
	final MyLibraryGrid grid;
  const MyCatalogGrid({required this.sort, required this.grid, super.key});

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
                gridDelegate: MyLibraryGridDelegate(grid),
                itemCount: snapshot.data!.length,
                itemBuilder: (context, index) =>
                    _buildGridItem(snapshot.data![index], index));
          } else {
            // TODO: report error
            // TODO: Do it better
            return const SizedBox();
          }
        });
  }

  Widget _buildGridItem(String programName, int index) {
    // Used to make sure to rebuild the item when a preference changes.
    return ChangeNotifierProvider(
      create: (_) => MyProgramPreference(programName),
      child: MyLibraryItem(
          programName: programName,
					grid: grid,
          // The key is used to identify the item in the list when program are added or removed.
          key: ValueKey(programName)),
    );
  }
}
