import 'package:flutter/material.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/library_page.dart';
import 'package:lowresrmx/page/manual_page.dart';
import 'package:lowresrmx/page/settings_page.dart';
import 'package:provider/provider.dart';
import 'package:re_editor/re_editor.dart';

final List<IconData> _toolIcon = [
  Icons.build_rounded,
  Icons.construction_rounded,
  Icons.handshake_rounded,
  Icons.brush_rounded,
  Icons.build_circle_rounded,
  Icons.auto_fix_high_rounded,
  Icons.home_repair_service_rounded,
  Icons.square_foot_rounded,
  Icons.architecture_rounded,
  Icons.colorize_rounded,
  Icons.hardware_rounded,
  Icons.plumbing_rounded,
  Icons.carpenter_rounded
];

class MyEditDrawer extends StatefulWidget {
  final String editedProgramName;
  final CodeLineEditingController editingController;
  final void Function(String) onStartTool;
  const MyEditDrawer(
      {required this.editedProgramName,
      required this.editingController,
      required this.onStartTool,
      super.key});

  @override
  State<MyEditDrawer> createState() => _MyEditDrawerState();
}

class _MyEditDrawerState extends State<MyEditDrawer> {
  late final MyProgramPreference programPreference;

  void gotoLibrary(BuildContext context) {
    Navigator.of(context).pushReplacement(
        MaterialPageRoute(builder: (context) => const MyLibraryPage()));
  }

  void gotoSettings(BuildContext context) {
    Navigator.of(context).push(MaterialPageRoute(
        builder: (context) =>
            MySettingsPage(context.read<MyEditorPreference>())));
  }

  @override
  void initState() {
    super.initState();
    programPreference = MyProgramPreference(widget.editedProgramName);
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
        future: programPreference.loadPreference(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return MultiProvider(providers: [
              ChangeNotifierProvider<MyProgramPreference>(
                  create: (context) => snapshot.data as MyProgramPreference),
            ], child: Drawer(child: buildScaffold(context)));
          } else if (snapshot.hasError) {
            return const Center(
                child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [Text("Error loading program preferences")]));
          } else {
            return const SizedBox();
          }
        });
  }

  Widget buildScaffold(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          // automaticallyImplyLeading: false,
          title: const Text("Back to Programs"),
          leading: IconButton(
              icon: const Icon(Icons.library_books_rounded),
              tooltip: "Program library",
              onPressed: () {
                Navigator.of(context).pop();
                gotoLibrary(context);
              }),
        ),
        body: buildListView());
  }

  Widget buildListView() {
    const int reservedItemsBeforeList = 7;
    const int reservedItemsAfterList = 2;
    return FutureBuilder(
        future: MyPreference.listToolProgram(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return ListView.builder(
                itemCount: reservedItemsBeforeList +
                    snapshot.data!.length +
                    reservedItemsAfterList,
                itemBuilder: (context, index) {
                  if (index == 0) {
                    return buildTraceItem(context);
                  } else if (index == 1) {
                    return buildToolItem(context);
                  } else if (index == 2) {
                    return const Divider();
                  } else if (index == 3) {
                    return const MyManualTile();
                  } else if (index == 4) {
                    return buildSettingItem(context);
                  } else if (index == 5) {
                    return const Divider();
                  } else if (index == 6) {
                    return const Padding(
                      padding: EdgeInsets.only(
                          left: 52.0, right: 8.0, top: 8.0, bottom: 4),
                      child: Text("Open with a tool:",
                          style: TextStyle(fontWeight: FontWeight.bold)),
                    );
                  } else if (index ==
                      reservedItemsBeforeList + snapshot.data!.length + 0) {
                    return const Divider();
                  } else if (index ==
                      reservedItemsBeforeList + snapshot.data!.length + 1) {
                    return buildInfoItem(context);
                  }
                  return ListTile(
                    leading: Icon(_toolIcon[
                        (index - reservedItemsBeforeList) % _toolIcon.length]),
                    title:
                        Text(snapshot.data![index - reservedItemsBeforeList]),
                    onTap: () {
                      Navigator.of(context).pop();
                      widget.onStartTool(
                          snapshot.data![index - reservedItemsBeforeList]);
                    },
                  );
                });
          } else if (snapshot.hasError) {
            return const Text("Error listing tool programs");
          } else {
            return const Center(
                child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                  CircularProgressIndicator(),
                ]));
          }
        });
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

  Widget buildTraceItem(BuildContext context) {
    final programPreference = context.watch<MyProgramPreference>();
    final currentTrace = programPreference.withTrace;
    return SwitchListTile(
        value: currentTrace,
        title: const Text("Show trace"),
        onChanged: (value) {
          // Store the change
          programPreference.setWithTrace(!currentTrace);
          // Force a rebuild
          setState(() {});
        });
  }

  Widget buildToolItem(BuildContext context) {
    final programPreference = context.watch<MyProgramPreference>();
    final currentTool = programPreference.isTool;
    return SwitchListTile(
        value: currentTool,
        title: const Text("Is a Tool"),
        onChanged: (value) {
          // Store the change
          programPreference.setTool(!currentTool);
          // Force a rebuild
          setState(() {});
        });
  }

  Widget buildInfoItem(BuildContext context) {
    return ListTile(
        leading: const Icon(Icons.info_rounded),
        titleAlignment: ListTileTitleAlignment.top,
        subtitle: Text(
          "Line count: ${widget.editingController.lineCount}\nToken count: ?\nCartridge size: ?",
        ));
  }
}
