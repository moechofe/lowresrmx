import 'package:flutter/material.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/style.dart';

class MySettingsPage extends StatefulWidget {
  final MyEditorPreference settings;

  const MySettingsPage(this.settings, {super.key});

  @override
  State<MySettingsPage> createState() => _MySettingsPageState();
}

class _MySettingsPageState extends State<MySettingsPage> {
	final double _fontSize = 16.0;

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
            )
          ])),
        ],
      )),
    );
  }
}
