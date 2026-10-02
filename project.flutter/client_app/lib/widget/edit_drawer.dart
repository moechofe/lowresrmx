import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/rom_entries.dart';
import 'package:lowresrmx/data/sync_conflict.dart';
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
    if (MyConflictService().isBlocked(widget.editedProgramName)) {
      Navigator.of(context).pop(); // Drawer only: the choice is still open.
      MyConflictService().present();
      return;
    }
    Navigator.of(context)
      ..pop() // Drawer
      ..pop(); // MyEditPage
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
            return MultiProvider(
                providers: [
                  ChangeNotifierProvider<MyProgramPreference>(
                      create: (context) =>
                          snapshot.data as MyProgramPreference),
                ],
                child: Drawer(
                    child: ScaffoldMessenger(child: buildScaffold(context))));
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
        // appBar: AppBar(
        //   // automaticallyImplyLeading: false,
        //   // title: const Text("Back to Programs"),
        // 	// title: Text(widget.editedProgramName),
        //   // leading: IconButton(
        // 	// 		icon: const Icon(Icons.arrow_back_rounded),
        //   //     tooltip: "Program library",
        //   //     onPressed: () => gotoLibrary(context)),
        // ),
        body: buildListView());
  }

  Widget buildListView() {
    const int reservedItemsBeforeList = 8;
    const int reservedItemsAfterList = 4;
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
                    return buildBackItem(context);
                  } else if (index == 1) {
                    return const MyManualTile();
                  } else if (index == 2) {
                    return buildSettingItem(context);
                  } else if (index == 3) {
                    return const Divider();
                  } else if (index == 4) {
                    return buildTraceItem(context);
                  } else if (index == 5) {
                    return buildToolItem(context);
                  } else if (index == 6) {
                    return const Divider();
                  } else if (index == 7) {
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
                  } else if (index ==
                      reservedItemsBeforeList + snapshot.data!.length + 2) {
                    return const Divider();
                  } else if (index ==
                      reservedItemsBeforeList + snapshot.data!.length + 3) {
                    return _MyRomCopySection(
                        editingController: widget.editingController);
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

  Widget buildBackItem(BuildContext context) {
    return ListTile(
        leading: const Icon(Icons.library_books_rounded),
        title: const Text("Back to Programs"),
        onTap: () => gotoLibrary(context));
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
        subtitle: const Text("Allows edit other programs"),
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

class _MyRomCopySection extends StatefulWidget {
  final CodeLineEditingController editingController;
  const _MyRomCopySection({required this.editingController});

  @override
  State<_MyRomCopySection> createState() => _MyRomCopySectionState();
}

class _MyRomCopySectionState extends State<_MyRomCopySection> {
  late Map<int, String> entries;
  final Set<int> selected = {};

  @override
  void initState() {
    super.initState();
    entries = parseRomEntries(widget.editingController.text);
    selected.addAll(romClipboard.keys.where((id) => !entries.containsKey(id)));
  }

  @override
  Widget build(BuildContext context) {
    return Column(crossAxisAlignment: CrossAxisAlignment.stretch, children: [
      const Padding(
        padding: EdgeInsets.only(left: 52.0, right: 8.0, top: 8.0, bottom: 16.0),
        child: Text("ROM entries:",
            style: TextStyle(fontWeight: FontWeight.bold)),
      ),
      Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16.0),
        child: GridView.count(
            crossAxisCount: 4,
            shrinkWrap: true,
            physics: const NeverScrollableScrollPhysics(),
            padding: EdgeInsets.zero,
            mainAxisSpacing: 6.0,
            crossAxisSpacing: 6.0,
            childAspectRatio: 1.8,
            children: List.generate(16, buildToggle)),
      ),
      Padding(
        padding: const EdgeInsets.fromLTRB(16.0, 8.0, 16.0, 16.0),
        child: Row(children: [
          Expanded(
              child: FilledButton.icon(
                  icon: const Icon(Icons.copy_rounded),
                  label: const Text("Copy"),
                  onPressed: selected.any(entries.containsKey)
                      ? () => copy(context)
                      : null)),
          const SizedBox(width: 8.0),
          Expanded(
              child: FilledButton.icon(
                  icon: const Icon(Icons.paste_rounded),
                  label: const Text("Paste"),
                  onPressed: selected.any(romClipboard.containsKey)
                      ? () => paste(context)
                      : null)),
        ]),
      ),
    ]);
  }

  Widget buildToggle(int id) {
    final bool present =
        entries.containsKey(id) || romClipboard.containsKey(id);
    final bool isOn = selected.contains(id);
    final VoidCallback? onPressed = present
        ? () => setState(() => isOn ? selected.remove(id) : selected.add(id))
        : null;
    final Widget label = Text("$id");
    if (isOn) {
      return FilledButton(
          style: FilledButton.styleFrom(
              padding: EdgeInsets.zero,
              minimumSize: const Size(0, 32),
              tapTargetSize: MaterialTapTargetSize.shrinkWrap),
          onPressed: onPressed,
          child: label);
    }
    return OutlinedButton(
        style: OutlinedButton.styleFrom(
            padding: EdgeInsets.zero,
            minimumSize: const Size(0, 32),
            tapTargetSize: MaterialTapTargetSize.shrinkWrap),
        onPressed: onPressed,
        child: label);
  }

  Future<void> copy(BuildContext context) async {
    final List<int> ids = selected.where(entries.containsKey).toList()..sort();
    setState(() => romClipboard =
        Map.unmodifiable({for (final int id in ids) id: entries[id]!}));
    await Clipboard.setData(ClipboardData(text: romEntriesText(entries, ids)));
    if (!context.mounted) return;
    ScaffoldMessenger.of(context)
      ..hideCurrentSnackBar()
      ..showSnackBar(SnackBar(
          duration: const Duration(seconds: 2),
          content: Text(
              "Copied ROM ${ids.map((id) => "#$id").join(", ")} to clipboard")));
  }

  void paste(BuildContext context) {
    final List<int> ids =
        selected.where(romClipboard.containsKey).toList()..sort();
    widget.editingController.text = pasteRomEntries(
        widget.editingController.text, romClipboard, ids);
    setState(() => entries = parseRomEntries(widget.editingController.text));
    ScaffoldMessenger.of(context)
      ..hideCurrentSnackBar()
      ..showSnackBar(SnackBar(
          duration: const Duration(seconds: 2),
          content: Text("Pasted ROM ${ids.map((id) => "#$id").join(", ")}")));
  }
}
