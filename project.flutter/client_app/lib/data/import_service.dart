import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:lowresrmx/app_keys.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/page/edit_page.dart';

enum MyImportPhase { importing, done, failed }

class MyImportStatus {
  const MyImportStatus(this.phase, this.name);

  final MyImportPhase phase;

  final String name;
}

/// Imports programs shared into the app or opened through a `lowresrmx:`
class MyImportService {
  static final MyImportService _singleton = MyImportService._internal();

  factory MyImportService() {
    return _singleton;
  }

  MyImportService._internal();

  static const MethodChannel _channel = MethodChannel("com.lowresrmx/import");

  bool _started = false;
  final ValueNotifier<MyImportStatus> _status =
      ValueNotifier(const MyImportStatus(MyImportPhase.importing, ""));
  bool _visible = false;
  Timer? _dismissTimer;

  Future<void> start() async {
    if (_started) return;
    _started = true;

    _channel.setMethodCallHandler(_onCall);
    try {
      final List<Object?> queued =
          await _channel.invokeMethod<List<Object?>>("drainPendingImports") ??
              const [];
      for (final Object? payload in queued) {
        await _dispatch(Map<String, Object?>.from(payload as Map));
      }
    } on PlatformException catch (error) {
      debugPrint("Import channel unavailable: $error");
    } on MissingPluginException catch (error) {
      debugPrint("Import channel unavailable: $error");
    }
  }

  Future<Object?> _onCall(MethodCall call) async {
    if (call.method == "import") {
      await _dispatch(Map<String, Object?>.from(call.arguments as Map));
    }
    return null;
  }

  Future<void> _dispatch(Map<String, Object?> payload) async {
    switch (payload["kind"]) {
      case "file":
        await importFile(payload["path"] as String, payload["name"] as String);
      case "uri":
        await importUri(Uri.parse(payload["uri"] as String));
      default:
        debugPrint("Unknown import payload: $payload");
    }
  }

  /// Imports the program at [path].
  Future<void> importFile(String path, String name) async {
    showImporting(name);
    final File file = File(path);
    try {
      if (!await file.exists()) {
        debugPrint("Shared file not found: $path");
        showFailed(name);
        return;
      }
      showImported(await MyLibrary.importCode(name, await file.readAsString()));
    } catch (error) {
      debugPrint("Import failed: $error");
      showFailed(name);
    } finally {
      try {
        await file.delete();
      } catch (_) {}
    }
  }

  Future<void> importUri(Uri uri) async {
    if (uri.scheme != "lowresrmx") return;
    final String? pid = uri.queryParameters['i'];
    final String? name = uri.queryParameters['n'];
    if (pid == null || name == null) return;
    showImporting(name);
    try {
      final String? imported = await MyLibrary.importFromRetroit(pid, name);
      if (imported == null) {
        showFailed(name);
        return;
      }
      showImported(imported);
    } catch (error) {
      debugPrint("Import failed: $error");
      showFailed(name);
    }
  }

  void showImporting(String name) {
    _dismissTimer?.cancel();
    _status.value = MyImportStatus(MyImportPhase.importing, name);
    _present();
  }

  void showImported(String name) {
    _status.value = MyImportStatus(MyImportPhase.done, name);
    // Re-shows it if the user swiped the snackbar
    _present();
    _armDismiss();
  }

  void showFailed(String name) {
    _status.value = MyImportStatus(MyImportPhase.failed, name);
    _present();
    _armDismiss();
  }

  void _armDismiss() {
    _dismissTimer?.cancel();
    _dismissTimer = Timer(const Duration(seconds: 10),
        () => appMessengerKey.currentState?.hideCurrentSnackBar());
  }

  void _present() {
    final ScaffoldMessengerState? messenger = appMessengerKey.currentState;
    if (messenger == null) return;
    if (_visible) return;
    _visible = true;
    messenger
        .showSnackBar(SnackBar(
          duration: const Duration(minutes: 10),
          content: ValueListenableBuilder<MyImportStatus>(
            valueListenable: _status,
            builder: (context, status, _) => Row(children: [
              Expanded(child: Text(_message(status))),
              if (status.phase == MyImportPhase.done)
                SnackBarAction(
                  label: "OPEN",
                  onPressed: () => openProgram(status.name),
                ),
            ]),
          ),
        ))
        .closed
        .then((_) => _visible = false);
  }

  String _message(MyImportStatus status) => switch (status.phase) {
        MyImportPhase.importing => "Importing ${status.name}…",
        MyImportPhase.done => "Imported ${status.name}",
        MyImportPhase.failed => "Could not import ${status.name}",
      };

  /// Opens [programName] in the editor.
  void openProgram(String programName) {
    final NavigatorState? navigator = appNavigatorKey.currentState;
    if (navigator == null) return;
    // Popping frome the editor saves its code, popping from the run page stops the program
    navigator.popUntil((route) => route.isFirst);
    navigator.pushNamed(MyEditPage.routeName,
        arguments: {"programName": programName});
  }

  @visibleForTesting
  void resetSnackBar() {
    _dismissTimer?.cancel();
    _visible = false;
  }
}
