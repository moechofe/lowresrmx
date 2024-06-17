import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;

import 'package:provider/provider.dart';

import 'package:markdown_widget/markdown_widget.dart';

import 'package:lowresrmx/page/manual_page.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/preference.dart';

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

// TODO: should rename from here

class MyEditDrawer extends StatefulWidget {
	final MyProgramPreference programPreference;
  const MyEditDrawer({required this.programPreference, super.key});

  @override
  State<MyEditDrawer> createState() => _MyEditDrawerState();
}

class _MyEditDrawerState extends State<MyEditDrawer> {
  @override
  Widget build(BuildContext context) {
    return Drawer(
      child: Column(
        children: [
          AppBar(
            leading: IconButton(
              icon: const Icon(Icons.arrow_back_rounded),
              onPressed: () => Navigator.of(context)
                ..pop()
                ..pop(),
            ),
          ),
          Expanded(child: _buildDrawer())
        ],
      ),
    );
  }

  Widget _buildDrawer() {
    const int reservedItems = 6;
    return FutureBuilder(
        future: MyPreference.listToolProgram(),
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            return ListView.builder(
              itemCount: 3 + snapshot.data!.length,
              itemBuilder: (context, index) {
                if (index == 0) {
                  return _buildTraceItem(context);
                } else if (index == 1) {
                  return const Divider();
                } else if (index == 2) {
                  return const MyManualTile();
                } else if (index == 3) {
                  return _buildSettingItem(context);
                } else if (index == 4) {
                  return const Divider();
                } else if (index == 5) {
                  return const Padding(
                    padding: EdgeInsets.only(
                        left: 52.0, right: 8.0, top: 8.0, bottom: 4),
                    child: Text("Open with a tool program:",
                        style: TextStyle(fontWeight: FontWeight.bold)),
                  );
                }
                return ListTile(
                  leading: Icon(
                      _toolIcon[(index - reservedItems) % _toolIcon.length]),
                  title: Text(snapshot.data![index - reservedItems]),
                  onTap: () {},
                );
              },
            );
          } else if (snapshot.hasError) {
            return const Text("Error listing tool programs");
          } else {
            return const Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  CircularProgressIndicator(),
                ],
              ),
            );
          }
        });
  }

  Widget _buildSettingItem(BuildContext context) {
    return ListTile(
      leading: const Icon(Icons.settings),
      title: const Text("Editor settings"),
      onTap: () {
        // Navigator.push(
        // 	context,
        // 	MaterialPageRoute(builder: (context) => MySettingPage()),
        // );
      },
    );
  }

  Widget _buildTraceItem(BuildContext context) {
		final currentTrace = widget.programPreference.withTrace;
    return SwitchListTile(
      value: currentTrace,
      title: const Text("Show trace"),
      onChanged: (value) {
				// Store the change
        widget.programPreference.setWithTrace(!currentTrace);
				// Force a rebuild
        setState((){});
      },
    );
  }
}
