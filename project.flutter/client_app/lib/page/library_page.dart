import 'package:flutter/material.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/sync_manager.dart';
import 'package:lowresrmx/page/manual_page.dart';
import 'package:lowresrmx/page/settings_page.dart';
import 'package:lowresrmx/widget/google_account_tile.dart';
import 'package:lowresrmx/widget/library_grid.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:provider/provider.dart';
import 'package:url_launcher/url_launcher.dart';

final Uri retroit = Uri.parse('https://ret.ro.it');

enum MyLibraryMenuOption {
  setting,
  name,
  oldest,
  newest,
	two,
	three,
	list,
}

class MyLibraryPage extends StatelessWidget {
  const MyLibraryPage({super.key});

  Future<String> getVersionInfo() async {
    PackageInfo packageInfo = await PackageInfo.fromPlatform();
    return "${packageInfo.version} (${packageInfo.buildNumber})";
  }

  void gotoSettings(BuildContext context) {
    Navigator.of(context).push(MaterialPageRoute(
        builder: (context) =>
            MySettingsPage(context.read<MyEditorPreference>())));
  }

  Widget buildMorePopupMenu(
      BuildContext context, MyLibraryPreference preference) {
    return PopupMenuButton<MyLibraryMenuOption>(
      itemBuilder: (BuildContext context) =>
          <PopupMenuEntry<MyLibraryMenuOption>>[
        const PopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.setting,
          child: Text('Editor settings'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.name,
          checked: preference.sort == MyLibrarySort.name,
          child: const Text('Sort by name'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.oldest,
          checked: preference.sort == MyLibrarySort.oldest,
          child: const Text('Sort by oldest'),
        ),
        CheckedPopupMenuItem<MyLibraryMenuOption>(
          value: MyLibraryMenuOption.newest,
          checked: preference.sort == MyLibrarySort.newest,
          child: const Text('Sort by newest'),
        ),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.two,
					checked: preference.grid == MyLibraryGrid.two,
					child: const Text('2 Columns'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.three,
					checked: preference.grid == MyLibraryGrid.three,
					child: const Text('3 Columns'),
				),
				CheckedPopupMenuItem<MyLibraryMenuOption>(
					value: MyLibraryMenuOption.list,
					checked: preference.grid == MyLibraryGrid.list,
					child: const Text('List'),
				),
      ],
      onSelected: (MyLibraryMenuOption value) {
        switch (value) {
          case MyLibraryMenuOption.setting:
            // TODO: open settings
            break;
          case MyLibraryMenuOption.name:
            preference.sort = MyLibrarySort.name;
            break;
          case MyLibraryMenuOption.oldest:
            preference.sort = MyLibrarySort.oldest;
            break;
          case MyLibraryMenuOption.newest:
            preference.sort = MyLibrarySort.newest;
            break;
					case MyLibraryMenuOption.two:
						preference.grid = MyLibraryGrid.two;
						break;
					case MyLibraryMenuOption.three:
						preference.grid = MyLibraryGrid.three;
						break;
					case MyLibraryMenuOption.list:
						preference.grid = MyLibraryGrid.list;
						break;
        }
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final MyLibraryPreference preference =
        context.watch<MyLibraryPreference>();
    debugPrint("MyLibraryPage.build()");
    return Scaffold(
        appBar: AppBar(title: const Text("Programs"), actions: [
          Consumer<SyncManager>(
            builder: (context, sync, child) {
              if (sync.accountEmail == null) return const SizedBox.shrink();
              return Icon(
								sync.syncing
										? Icons.cloud_sync_rounded
										:
                sync.isAuthorized
                    ? Icons.cloud_done_rounded
                    : Icons.cloud_off_rounded,
                size: 20,
                color: sync.isAuthorized
                    ? null
                    : Theme.of(context).colorScheme.error,
              );
            },
          ),
          buildMorePopupMenu(context, preference),
          const SizedBox(width: 8.0),
        ]),
        drawer: buildDrawer(context),
        floatingActionButton: FloatingActionButton.small(
          onPressed: () async {
            MyLibrary.createProgram();
          },
          child: const Icon(Icons.add_rounded),
        ),
        body: SafeArea(
            child: MyCatalogGrid(
                sort: preference.sort, grid: preference.grid)));
  }

  Widget buildDrawer(BuildContext context) {
    return Drawer(
      child: ListView(
        children: [
					buildCommunityTile(context),
          const MyManualTile(),
          buildSettingTile(context),
          buildReinstallTile(context),
					const Divider(),
          const MyGoogleAccountTile(),
          const Divider(),
          buildAboutTile(context),
        ],
      ),
    );
  }

	Future<void> openRetroit() async {
		if (!await launchUrl(retroit)) {
			throw Exception('Could not launch $retroit');
		}
	}

	Widget buildCommunityTile(BuildContext context) {
		return ListTile(
			leading: const Icon(Icons.web_rounded),
			title: const Text("Community website"),
			onTap: openRetroit
		);
	}

  Widget buildSettingTile(BuildContext context) {
    return ListTile(
        leading: const Icon(Icons.settings),
        title: const Text("Editor settings"),
        onTap: () {
          Navigator.of(context).pop();
          gotoSettings(context);
        });
  }

  Widget buildReinstallTile(BuildContext context) {
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

  Widget buildAboutTile(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.commit_rounded),
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
    );
  }
}
