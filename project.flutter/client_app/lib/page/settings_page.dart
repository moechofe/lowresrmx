import 'package:flutter/material.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/sync_manager.dart';
import 'package:lowresrmx/style.dart';
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
            )
          ])),
					Padding(
            padding: const EdgeInsets.fromLTRB(20.0, 8.0, 20.0, 8.0),
            child: Text("Programs in Google Drive",
                style:
                    settingHeader.copyWith(color: Theme.of(context).hintColor),
                textAlign: TextAlign.left),
          ),
					Card(
						child: Consumer<SyncManager>(
							builder: (context, sync, child) {
								final photoUrl = sync.currentUser?.photoUrl;
								return Column(children: [
									ListTile(
										leading: sync.isLoggedIn
											? CircleAvatar(
													backgroundImage: photoUrl != null ? NetworkImage(photoUrl) : null,
													radius: 12,
													child: photoUrl == null ? const Icon(Icons.person) : null,
												)
											: const Icon(Icons.account_circle),
										title: Text(sync.isLoggedIn
											? (sync.currentUser?.displayName ?? sync.currentUser?.email ?? "Connected")
											: "Not connected"),
										subtitle: sync.isLoggedIn && !sync.isAuthorized
											? const Text("Permission required", style: TextStyle(color: Colors.red))
											: null,
										trailing: TextButton(
											onPressed: () => sync.isLoggedIn ? sync.logout() : sync.login(),
											child: Text(sync.isLoggedIn ? "Sign Out" : "Sign In"),
										),
									),
									if (sync.isLoggedIn && !sync.isAuthorized)
										Padding(
											padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
											child: ElevatedButton(
												onPressed: () => sync.requestDrivePermissions(),
												child: const Text("Grant Drive Permission"),
											),
										),
								]);
							},
						)
					)
        ],
      )),
    );
  }
}
