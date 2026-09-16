import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/widget/library_item.dart';

const double libraryBigItemExtent = 176.0;
const double libraryMediumItemExtent = 116.0;

class MyLibraryGridDelegate extends SliverGridDelegate {
	final double targetExtent;
	final double titleHeight;

	const MyLibraryGridDelegate({required this.targetExtent, required this.titleHeight});

  @override
  SliverGridLayout getLayout(SliverConstraints constraints) {
    final int rounded = (constraints.crossAxisExtent / targetExtent).round();
    final int countPerRow = rounded < 1 ? 1 : rounded;
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
    return oldDelegate.targetExtent != targetExtent ||
        oldDelegate.titleHeight != titleHeight;
  }
}

class MyCatalogGrid extends StatelessWidget {
  final MyLibrarySort sort;
	final MyLibraryGrid grid;
  const MyCatalogGrid({required this.sort, required this.grid, super.key});

  static const EdgeInsets _padding =
      EdgeInsets.only(left: 12.0, right: 12.0, bottom: 24.0);

  @override
  Widget build(BuildContext context) {
    // Make sure to rebuild the grid when a program is created, removed or renamed.
    final MyLibrary _ = context.watch<MyLibrary>();
    return FutureBuilder(
        future: MyLibrary.buildList(sort),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            final List<String> names = snapshot.data!;
            return switch (grid) {
              MyLibraryGrid.big => _buildGrid(names,
                  const MyLibraryGridDelegate(
                      targetExtent: libraryBigItemExtent, titleHeight: 40.0)),
              MyLibraryGrid.medium => _buildGrid(names,
                  const MyLibraryGridDelegate(
                      targetExtent: libraryMediumItemExtent, titleHeight: 50.0)),
              MyLibraryGrid.list => _buildList(names),
            };
          } else {
            // TODO: report error
            // TODO: Do it better
            return const SizedBox();
          }
        });
  }

  Widget _buildGrid(List<String> names, MyLibraryGridDelegate delegate) =>
      GridView.builder(
          padding: _padding,
          gridDelegate: delegate,
          itemCount: names.length,
          itemBuilder: (context, index) => _buildItem(names[index]));

  Widget _buildList(List<String> names) => ListView.builder(
      padding: _padding,
      itemCount: names.length,
      itemBuilder: (context, index) => _buildItem(names[index]));

  Widget _buildItem(String programName) {
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
