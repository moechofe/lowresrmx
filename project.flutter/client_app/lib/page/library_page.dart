import 'dart:developer';
import 'package:flutter/material.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/sync_manager.dart';
import 'package:lowresrmx/page/manual_page.dart';
import 'package:lowresrmx/page/settings_page.dart';
import 'package:lowresrmx/widget/library_grid.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:provider/provider.dart';

enum MyLibraryMenuOption {
  setting,
  name,
  oldest,
  newest,
	two,
	three,
}

class MyLibraryPage extends StatefulWidget {
  const MyLibraryPage({super.key});

  @override
  State<MyLibraryPage> createState() => _MyLibraryPageState();
}

class _MyLibraryPageState extends State<MyLibraryPage> {
  MyLibrarySort sort = MyLibrarySort.name;
	MyLibraryGrid grid = MyLibraryGrid.two;

  Future<String> getVersionInfo() async {
    PackageInfo packageInfo = await PackageInfo.fromPlatform();
    return packageInfo.version;
  }

  void gotoSettings(BuildContext context) {
    Navigator.of(context).push(MaterialPageRoute(
        builder: (context) =>
            MySettingsPage(context.read<MyEditorPreference>())));
  }

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
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.two,
					checked: grid == MyLibraryGrid.two,
					child: const Text('2 Columns'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.three,
					checked: grid == MyLibraryGrid.three,
					child: const Text('3 Columns'),
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
					case MyLibraryMenuOption.two:
						setState(() {
							grid = MyLibraryGrid.two;
						});
						break;
					case MyLibraryMenuOption.three:
						setState(() {
							grid = MyLibraryGrid.three;
						});
						break;
        }
        // Handle menu item selection
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    log("MyLibraryPage.build()");
    return Scaffold(
        appBar: AppBar(title: const Text("Programs"), actions: [
          // Consumer<SyncManager>(
          //   builder: (context, sync, child) {
          //     if (!sync.isLoggedIn) return const SizedBox.shrink();
          //     return Icon(
          //       sync.isAuthorized
          //           ? Icons.cloud_done_rounded
          //           : Icons.cloud_off_rounded,
          //       size: 20,
          //       color: sync.isAuthorized
          //           ? null
          //           : Theme.of(context).colorScheme.error,
          //     );
          //   },
          // ),
          buildMorePopupMenu(context),
          const SizedBox(width: 8.0),
        ]),
        drawer: buildDrawer(context),
        floatingActionButton: FloatingActionButton.small(
          onPressed: () async {
            MyLibrary.createProgram();
          },
          child: const Icon(Icons.add_rounded),
        ),
        body: SafeArea(child:MyCatalogGrid(sort: sort, grid: grid)));
  }

  Widget buildDrawer(BuildContext context) {
    return Drawer(
      child: ListView(
        children: [
          const MyManualTile(),
          buildSettingItem(context),
          buildReinstallItem(context),
          const Divider(),
          buildAboutItem(context),
        ],
      ),
    );
  }

  Widget buildSettingItem(BuildContext context) {
    return ListTile(
        leading: const Icon(Icons.settings),
        title: const Text("Editor settings"),
        onTap: () {
          Navigator.of(context).pop();
          gotoSettings(context);
        });
  }

  Widget buildReinstallItem(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.restore_rounded),
      title: const Text("Reinstall default programs"),
      subtitle: const Text("Will erase modification mades on them."),
      titleAlignment: ListTileTitleAlignment.top,
      onTap: () async {
        // TODO: implement me
      },
    );
  }

  Widget buildAboutItem(BuildContext context) {
    return ListTile(
      title: const Text("LowResRMX version"),
      subtitle: FutureBuilder(
        future: getVersionInfo(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return Text(snapshot.data!);
          }
          return const Text("…");
        },
      ),
      titleAlignment: ListTileTitleAlignment.top,
    );
  }
}
