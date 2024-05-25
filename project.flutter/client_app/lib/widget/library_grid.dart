import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/widget/library_item.dart';

// TODO: add sort options

class MyLibraryGridDelegate extends SliverGridDelegate {
  @override
  SliverGridLayout getLayout(SliverConstraints constraints) {
    int countPerRow = constraints.crossAxisExtent ~/ 150;
    if (countPerRow < 1) countPerRow = 1;
    final double cross = constraints.crossAxisExtent / countPerRow;
    final double main = cross;
    return SliverGridRegularTileLayout(
      childMainAxisExtent: main,
      childCrossAxisExtent: cross,
      crossAxisCount: countPerRow,
      mainAxisStride: main + 8,
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
                itemBuilder: (context, index) => MyLibraryItem(
                    program: snapshot.data![index],
                    key: ValueKey(snapshot.data![index])));
          } else {
            // TODO: report error
            // TODO: Do it better
            return const SizedBox();
          }
        });
  }
}
