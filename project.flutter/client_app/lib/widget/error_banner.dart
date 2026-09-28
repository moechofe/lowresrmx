import 'package:flutter/material.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/editor_errors.dart';

/// Shows the pinned error above the code until the code changes or it is dismissed.
class MyErrorBanner extends StatelessWidget {
  final void Function(Location location) onGoto;

  const MyErrorBanner({super.key, required this.onGoto});

  @override
  Widget build(BuildContext context) {
    final MyEditorErrors errors = context.watch<MyEditorErrors>();
    final MyReportedError? error = errors.pinned;
    if (error == null) return const SizedBox.shrink();

    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final int row = error.location.row;
    return Material(
      color: colorScheme.errorContainer,
      child: ListTile(
        dense: true,
        textColor: colorScheme.onErrorContainer,
        iconColor: colorScheme.onErrorContainer,
        leading: const Icon(Icons.error_rounded),
        title: Text(error.message),
        subtitle: row >= 0 ? Text("Line ${row + 1}") : null,
        onTap: row >= 0 ? () => onGoto(error.location) : null,
        trailing: IconButton(
          icon: const Icon(Icons.close_rounded),
          tooltip: "Dismiss",
          onPressed: errors.unpin,
        ),
      ),
    );
  }
}
