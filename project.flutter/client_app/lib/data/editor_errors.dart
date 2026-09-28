import 'package:flutter/foundation.dart';

import 'package:lowresrmx/core/runtime.dart';

/// An error message and the place in the edited source it refers to.
class MyReportedError {
  final String message;
  final Location location;
  const MyReportedError(this.message, this.location);

  /// Null when [err] is ok. [code] is the source [err.position] indexes.
  static MyReportedError? of(Error err, String code) =>
      err.ok ? null : MyReportedError(err.msg, err.getLocation(code));
}

/// The errors the editor shows. [compiled] follows the continuous compilation;
/// [pinned] is what the banner shows until the code it was reported against changes.
class MyEditorErrors extends ChangeNotifier {
  MyReportedError? compiled;
  MyReportedError? pinned;
  int? _pinnedCodeHash;

  /// The pinned error wins, so a runtime error survives a clean compile.
  MyReportedError? get marked => pinned ?? compiled;
  int get markedRow => marked?.location.row ?? -1;

  void setCompiled(MyReportedError? error) {
    compiled = error;
    notifyListeners();
  }

  void pin(MyReportedError error, int codeHash) {
    pinned = error;
    _pinnedCodeHash = codeHash;
    notifyListeners();
  }

  void pinMarked(int codeHash) {
    final MyReportedError? error = marked;
    if (error != null) pin(error, codeHash);
  }

  void unpin() {
    if (pinned == null) return;
    pinned = null;
    _pinnedCodeHash = null;
    notifyListeners();
  }

  void codeChanged(int codeHash) {
    if (pinned != null && codeHash != _pinnedCodeHash) unpin();
  }
}
