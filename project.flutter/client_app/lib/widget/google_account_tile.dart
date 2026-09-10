import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'package:lowresrmx/data/sync_manager.dart';

class MyGoogleAccountTile extends StatelessWidget {
  const MyGoogleAccountTile({super.key});

  @override
  Widget build(BuildContext context) {
    final SyncManager sync = context.watch<SyncManager>();
    if (!sync.isAvailable) {
      return const SizedBox.shrink();
    }
    final String? email = sync.accountEmail;
    if (sync.isAuthorized && email != null) {
      return ListTile(
        leading: const Icon(Icons.cloud_done_rounded),
        title: Text(sync.accountName ?? "Google Drive"),
        subtitle: Text(email),
        trailing: IconButton(
          icon: const Icon(Icons.logout_rounded),
          tooltip: "Disconnect",
          onPressed: () => sync.disconnect(),
        ),
      );
    }
    return ListTile(
      leading: Icon(Icons.cloud_off_rounded,
          color: email == null ? null : Theme.of(context).colorScheme.error),
      title: Text(
          email == null ? "Connect Google Drive" : "Reconnect Google Drive"),
      subtitle: email == null ? null : Text(email),
      onTap: () async {
        final bool connected = await sync.connect();
        if (!connected && context.mounted) {
          ScaffoldMessenger.of(context).showSnackBar(const SnackBar(
              content: Text("Google Drive access was not granted.")));
        }
      },
    );
  }
}
