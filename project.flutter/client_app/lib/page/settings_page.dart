import 'package:flutter/material.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/sync_manager.dart';
import 'package:lowresrmx/style.dart';
import 'package:lowresrmx/widget/code_sample.dart';
import 'package:provider/provider.dart';

class MySettingsPage extends StatefulWidget {
  final MyEditorPreference settings;

  const MySettingsPage(this.settings, {super.key});

  @override
  State<MySettingsPage> createState() => _MySettingsPageState();
}

class _MySettingsPageState extends State<MySettingsPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Editor Setting'),
      ),
      body: SingleChildScrollView(
          child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(20.0, 8.0, 20.0, 8.0),
            child: Text("Editor appearance",
                style:
                    settingHeader.copyWith(color: Theme.of(context).hintColor),
                textAlign: TextAlign.left),
          ),
          Card(
              child: Column(children: [
            ListTile(
              title: const Text("Font size"),
              subtitle: Slider(
								min: 10,
								max: 24,
								divisions: 24-10,
								value: widget.settings.fontSize,
								label: widget.settings.fontSize.truncate().toString(),
								onChanged: (value) {
									setState(() {
										widget.settings.fontSize=value;
									});

								}),
            ),
            Padding(
              padding: const EdgeInsets.fromLTRB(16.0, 0.0, 16.0, 16.0),
              child: MyCodeSample(fontSize: widget.settings.fontSize),
            ),
          ])),
        ],
      )),
    );
  }
}
