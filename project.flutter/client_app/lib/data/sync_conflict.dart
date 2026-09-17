import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';

import 'package:lowresrmx/app_keys.dart';
import 'package:lowresrmx/data/library.dart';

/// The version of a program's code the app still held when a Drive pull
/// replaced the file on disk.
@immutable
class MyCodeConflict {
  const MyCodeConflict({
    required this.programName,
    required this.localCode,
    required this.localModified,
    required this.remoteModified,
  });

  final String programName;

  /// What was on disk before the pull, or what the holder produced since — a
  /// tool's data-disk save, the editor buffer at dispose.
  final String localCode;

  final DateTime localModified;

  /// When Drive last saw the version that is now on disk.
  final DateTime remoteModified;

  MyCodeConflict withLocal(String code) => MyCodeConflict(
        programName: programName,
        localCode: code,
        localModified: DateTime.now(),
        remoteModified: remoteModified,
      );
}

class MyCodeHolder {
  MyCodeHolder._(this._service, this.programName, this.localCode, this.onReload);

  final MyConflictService _service;
  final String programName;

  /// The live version, or null while the page is not ready yet.
  final String? Function()? localCode;

  /// Receives the Drive version when the user reloads.
  final void Function(String code)? onReload;

  void release() => _service._release(this);
}

/// Tracks the programs a page holds in memory and the conflicts a Drive pull
/// creates by replacing one of them on disk.
class MyConflictService {
  static final MyConflictService _singleton = MyConflictService._internal();

  factory MyConflictService() => _singleton;

  MyConflictService._internal();

  final Map<String, List<MyCodeHolder>> _holds = {};
  final Map<String, MyCodeConflict> _conflicts = {};

  /// The snackbar on screen, or null. Compared by identity when a `closed`
  /// future fires: resolving one conflict hides its snackbar and shows the next
  /// one immediately, so the old future arrives while the new bar is up and
  /// must not report it as gone — a second [present] would then queue a
  /// duplicate behind it, left showing an empty bar once everything resolved.
  ScaffoldFeatureController<SnackBar, SnackBarClosedReason>? _shown;

  /// Set by [SyncManager]: pushes one program after "keep local" rewrote it.
  Future<void> Function(String programName)? pushProgram;

  /// Pending conflicts, oldest first. The editor watches it to refuse pops.
  final ValueNotifier<List<MyCodeConflict>> conflicts = ValueNotifier(const []);

  bool get hasConflicts => _conflicts.isNotEmpty;

  bool isHeld(String programName) => _holds.containsKey(programName);

  /// True while [programName] must not be written by anything but a resolution.
  bool isBlocked(String programName) => _conflicts.containsKey(programName);

  MyCodeConflict? conflictFor(String programName) => _conflicts[programName];

  /// Registers that the caller holds [programName]'s code. Holds stack: the run
  /// page takes one before the editor it replaces releases its own.
  MyCodeHolder hold(String programName,
      {String? Function()? localCode, void Function(String code)? onReload}) {
    final MyCodeHolder hold =
        MyCodeHolder._(this, programName, localCode, onReload);
    _holds.putIfAbsent(programName, () => <MyCodeHolder>[]).add(hold);
    return hold;
  }

  void _release(MyCodeHolder hold) {
    final List<MyCodeHolder>? list = _holds[hold.programName];
    if (list == null) return;
    list.remove(hold);
    if (list.isEmpty) _holds.remove(hold.programName);
  }

  /// Records that a pull is replacing [programName] while a page holds it. A
  /// pull over a file nobody holds is invisible and correct, so it is ignored.
  /// A second pull keeps the first local version and only moves the Drive date.
  void recordPull({
    required String programName,
    required String localCode,
    required DateTime localModified,
    required DateTime remoteModified,
  }) {
    if (!isHeld(programName)) return;
    final MyCodeConflict? existing = _conflicts[programName];
    _conflicts[programName] = MyCodeConflict(
      programName: programName,
      localCode: existing?.localCode ?? localCode,
      localModified: existing?.localModified ?? localModified,
      remoteModified: remoteModified,
    );
    _publish();
    present();
  }

  /// Replaces the local version on offer: a tool's data-disk save or the editor
  /// buffer at dispose, neither of which may reach the disk while unresolved.
  void updateLocal(String programName, String code) {
    final MyCodeConflict? conflict = _conflicts[programName];
    if (conflict == null) return;
    _conflicts[programName] = conflict.withLocal(code);
    _publish();
  }

  /// Writes the held version back over the pulled file and pushes it.
  Future<void> keepLocal(String programName) async {
    final MyCodeConflict? conflict = _conflicts[programName];
    if (conflict == null) return;
    final String code = _liveCode(programName) ?? conflict.localCode;
    _resolve(programName);
    await MyLibrary.writeCode(programName, code);
    MyLibrary().refresh();
    await pushProgram?.call(programName);
  }

  /// Drops the held version; the pulled file on disk wins and any holder that
  /// can show it is told to reload.
  Future<void> reloadFromDrive(String programName) async {
    if (!_conflicts.containsKey(programName)) return;
    final File file = await MyLibrary.getCodeFile(programName);
    final String code = await file.exists() ? await file.readAsString() : "";
    _resolve(programName);
    _notifyReload(programName, code);
  }

  String? _liveCode(String programName) {
    for (final MyCodeHolder hold in (_holds[programName] ?? []).reversed) {
      final String? code = hold.localCode?.call();
      if (code != null) return code;
    }
    return null;
  }

  void _notifyReload(String programName, String code) {
    for (final MyCodeHolder hold in (_holds[programName] ?? []).reversed) {
      if (hold.onReload != null) {
        hold.onReload!(code);
        return;
      }
    }
  }

  void _resolve(String programName) {
    _conflicts.remove(programName);
    _publish();
    appMessengerKey.currentState?.hideCurrentSnackBar();
    _shown = null;
    present();
  }

  void _publish() =>
      conflicts.value = List<MyCodeConflict>.unmodifiable(_conflicts.values);

  /// Shows the pending conflict. The messenger sits above the navigator, so the
  /// snackbar stays up when the editor is replaced by the run page. No-op while
  /// one is already showing or nothing is pending.
  void present() {
    if (_shown != null || _conflicts.isEmpty) return;
    final ScaffoldMessengerState? messenger = appMessengerKey.currentState;
    if (messenger == null) return;
    final ScaffoldFeatureController<SnackBar, SnackBarClosedReason> shown =
        messenger.showSnackBar(SnackBar(
      duration: const Duration(days: 7),
      dismissDirection: DismissDirection.none,
      content: ValueListenableBuilder<List<MyCodeConflict>>(
        valueListenable: conflicts,
        builder: (context, list, _) => list.isEmpty
            ? const SizedBox.shrink()
            : _MyConflictChoice(conflict: list.first),
      ),
    ));
    _shown = shown;
    shown.closed.then((_) {
      if (identical(_shown, shown)) _shown = null;
    });
  }

  @visibleForTesting
  void reset() {
    _holds.clear();
    _conflicts.clear();
    _shown = null;
    conflicts.value = const [];
  }
}

/// "just now", "25 min ago", "3 h ago", "2 days ago". Negative ages (a Drive
/// clock ahead of this device) read "just now".
String formatAge(DateTime when) {
  final Duration age = DateTime.now().difference(when);
  if (age.inMinutes < 1) return "just now";
  if (age.inMinutes < 60) return "${age.inMinutes} min ago";
  if (age.inHours < 24) return "${age.inHours} h ago";
  return "${age.inDays} ${age.inDays == 1 ? "day" : "days"} ago";
}

class _MyConflictChoice extends StatelessWidget {
  const _MyConflictChoice({required this.conflict});

  final MyCodeConflict conflict;

  @override
  Widget build(BuildContext context) {
    final ThemeData theme = Theme.of(context);
    final Color color =
        theme.snackBarTheme.actionTextColor ?? theme.colorScheme.inversePrimary;
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text("Synchronization replaced ${conflict.programName}. \n"
            "Which version do you keep?"),
        Row(children: [
          Expanded(
              child: _choice(
                  context,
                  color,
                  "Reload from Cloud",
                  formatAge(conflict.remoteModified),
                  () => unawaited(MyConflictService()
                      .reloadFromDrive(conflict.programName)))),
          const SizedBox(width: 8.0),
          Expanded(
              child: _choice(
                  context,
                  color,
                  "Keep local",
                  formatAge(conflict.localModified),
                  () => unawaited(
                      MyConflictService().keepLocal(conflict.programName)))),
        ]),
      ],
    );
  }

  Widget _choice(BuildContext context, Color color, String label, String age,
      VoidCallback onPressed) {
    return TextButton(
      style: TextButton.styleFrom(foregroundColor: color),
      onPressed: onPressed,
      child: Column(mainAxisSize: MainAxisSize.min, children: [
        Text(label, textAlign: TextAlign.center),
        Text(age,
            style:
                Theme.of(context).textTheme.labelSmall?.copyWith(color: color)),
      ]),
    );
  }
}
