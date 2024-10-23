import 'dart:async';
import 'dart:developer';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/manual_page.dart';
import 'package:lowresrmx/page/settings_page.dart';
import 'package:lowresrmx/widget/library_grid.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:path/path.dart' as p;
import 'package:provider/provider.dart';
import 'package:share_handler/share_handler.dart';

enum MyLibraryMenuOption {
  setting,
  name,
  oldest,
  newest,
}

// enum MyLibraryFolderOption {
// 	reset
// }

class MyLibraryPage extends StatefulWidget {
  const MyLibraryPage({super.key});

  @override
  State<MyLibraryPage> createState() => _MyLibraryPageState();
}

class _MyLibraryPageState extends State<MyLibraryPage> {
  MyLibrarySort sort = MyLibrarySort.name;
  StreamSubscription<SharedMedia>? streamSubscription;
  SharedMedia? sharedMedia;
  TextEditingController? nameController;

  @override
  void initState() {
    super.initState();
		if (Platform.isAndroid || Platform.isIOS) {
    	initSharedMedia();
		}
  }

  @override
  void dispose() {
    streamSubscription?.cancel();
    super.dispose();
  }

  bool mediaIsAProgram(SharedMedia media) {
    if (media.attachments == null) {
      return false;
    }
    if (media.attachments!.length != 1) {
      return false;
    }
    final SharedAttachment attachment = media.attachments![0]!;
    if (p.extension(attachment.path) != MyLibrary.extension) {
      return false;
    }
    return true;
  }

  Future<void> initSharedMedia() async {
    final handler = ShareHandlerPlatform.instance;
    final media = await handler.getInitialSharedMedia();
    if (media != null && mediaIsAProgram(media)) {
      sharedMedia = media;
    }
    streamSubscription = handler.sharedMediaStream.listen((media) {
      if (!mounted) return;
      if (mediaIsAProgram(media)) {
        setState(() {
          sharedMedia = media;
        });
      }
    });
  }

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
        }
        // Handle menu item selection
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    log("MyLibraryPage.build()");
		if (sharedMedia != null) {
			WidgetsBinding.instance.addPostFrameCallback((_) {
				showSaveSharedMedia(sharedMedia!.attachments![0]!.path);
			});
		}
    return Scaffold(
        appBar: AppBar(title: const Text("Programs"), actions: [
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
        body: MyCatalogGrid(sort: sort));
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
			) , titleAlignment: ListTileTitleAlignment.top,
		);
	}

  void showSaveSharedMedia(String inputFile) async {
    // Retrieve media name and content
    final String mediaName = p.basenameWithoutExtension(inputFile);
    final File mediaFile = File(inputFile);
    if (!await mediaFile.exists()) {
      log("File not found: $inputFile");
      return;
    }
    final String mediaCode = await mediaFile.readAsString();
    nameController = TextEditingController(text: mediaName);
    if (!mounted) return;
    String? newName = await showDialog<String?>(
        context: context,
        builder: (BuildContext context) {
          String newName = p.basenameWithoutExtension(inputFile);
          return AlertDialog(
              icon: const Icon(Icons.drive_file_rename_outline_rounded),
              title: const Text("Name"),
              content: TextField(
                controller: nameController,
                autofocus: true,
                onChanged: (value) => newName = value,
                decoration: const InputDecoration(
                  labelText: "Name",
                ),
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(context),
                  child: const Text("Cancel"),
                ),
                TextButton(
                  onPressed: () => Navigator.pop(context, newName),
                  child: const Text("Import"),
                )
              ]);
        });
    if (newName != null) {
      final File newFile = await MyLibrary.createProgram();
      final String automaticName = p.basenameWithoutExtension(newFile.path);
      await MyLibrary.writeCode(automaticName, mediaCode);
      await MyLibrary.renameProgram(automaticName, newName);
			setState(() {
				sharedMedia = null;
				nameController = null;
			});
    }
  }
}
