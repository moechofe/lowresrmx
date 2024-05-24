import 'dart:developer' show log;

import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:lowresrmx/data/argument.dart';

import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/program.dart';
import 'package:lowresrmx/page/edit_page.dart';

// TODO: add sort options

class MyLibraryGridDelegate extends SliverGridDelegate {
  @override
  SliverGridLayout getLayout(SliverConstraints constraints) {
    int countPerRow = constraints.crossAxisExtent ~/ 100;
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
  const MyCatalogGrid({super.key});

  @override
  Widget build(BuildContext context) {
		log("MyCatalogGrid.build");
    return FutureBuilder(
        future: MyLibrary.buildList(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return GridView.builder(
              gridDelegate: MyLibraryGridDelegate(),
              itemCount: snapshot.data!.length,
              itemBuilder: (context, index) =>
                  MyLibraryItem(program: snapshot.data![index]),
            );
          } else {
            // TODO: report error
            // TODO: Do it better
            return const SizedBox();
          }
        });
  }
}

class MyLibraryItem extends StatefulWidget {
  final MyProgram program;

  const MyLibraryItem({
    required this.program,
    super.key,
  });

  @override
  State<MyLibraryItem> createState() => _MyLibraryItemState();
}

class _MyLibraryItemState extends State<MyLibraryItem> {
  @override
  Widget build(BuildContext context) {
    return Card(
      child: InkWell(
        onTap: () {
          Navigator.push(
              context,
              MaterialPageRoute(
                  builder: (context) =>
                      MyEditPage(programName: widget.program.name)));
        },
        borderRadius: BorderRadius.circular(12.0),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            ListTile(
              title: Text(widget.program.name),
            )
          ],
        ),
      ),
    );
  }
}
