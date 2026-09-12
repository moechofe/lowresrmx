import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'package:lowresrmx/data/sync_manager.dart';

class MySyncTile extends StatelessWidget {
	const MySyncTile({super.key});

	@override
  Widget build(BuildContext context) {
		final SyncManager sync = context.watch<SyncManager>();
    if (!sync.isAvailable) {
      return const SizedBox.shrink();
    }
		final String? email = sync.accountEmail;
    if (sync.isAuthorized && email != null) {
      return ListTile(
				title: Text("Ready"),
				onTap: () {
					sync.syncAllPrograms();
				},
			);
		}
		return const SizedBox.shrink();
	}
}
