import 'dart:async';
import 'dart:developer' show log;
import 'dart:io';

import 'package:app_links/app_links.dart';
import 'package:flutter/material.dart';
import 'package:path/path.dart' as p;
import 'package:share_handler/share_handler.dart';

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

  StreamSubscription<SharedMedia>? _mediaSub;
  final ValueNotifier<MyImportStatus> _status =
      ValueNotifier(const MyImportStatus(MyImportPhase.importing, ""));
  bool _visible = false;
  Timer? _dismissTimer;

  Future<void> start() async {
    if (_mediaSub != null) return;

    // For shared from device
    final handler = ShareHandlerPlatform.instance;
    final media = await handler.getInitialSharedMedia();
    if (media != null && _isProgram(media)) {
      importSharedMedia(media);
    }
    _mediaSub = handler.sharedMediaStream.listen((media) {
      if (_isProgram(media)) {
        importSharedMedia(media);
      }
    });

    // For shared from browser
    final appLinks = AppLinks();
    final Uri? initialUri = await appLinks.getInitialLink();
    if (initialUri != null) {
      importUri(initialUri);
    }
    appLinks.uriLinkStream.listen(importUri);
  }

  bool _isProgram(SharedMedia media) {
    if (media.attachments == null) {
      return false;
    }
    if (media.attachments!.length != 1) {
      return false;
    }
    final SharedAttachment attachment = media.attachments![0]!;
    if (p.extension(attachment.path) != MyLibrary.codeExtension) {
      return false;
    }
    return true;
  }

  Future<void> importSharedMedia(SharedMedia media) async {
    final String path = media.attachments![0]!.path;
    final String name = p.basenameWithoutExtension(path);
    showImporting(name);
    try {
      final File file = File(path);
      if (!await file.exists()) {
        log("Shared file not found: $path");
        showFailed(name);
        return;
      }
      showImported(await MyLibrary.importCode(name, await file.readAsString()));
    } catch (error) {
      log("Import failed: $error");
      showFailed(name);
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
      log("Import failed: $error");
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
