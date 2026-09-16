import 'package:flutter/foundation.dart';
import 'package:lowresrmx/data/library.dart';

import 'package:package_info_plus/package_info_plus.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;

import 'package:shared_preferences/shared_preferences.dart';

enum InstallChange { unchanged, freshInstall, updated, downgraded }

abstract class MyPreference {
  static Future<String> getProgramDirectory() async {
    // final prefs = await SharedPreferences.getInstance();
    // String? libraryPath = prefs.getString("programDirectory");
    String? libraryPath;
    // if (libraryPath == null) {
    libraryPath = (await getApplicationDocumentsDirectory()).path;
    if (defaultTargetPlatform == TargetPlatform.linux ||
        defaultTargetPlatform == TargetPlatform.windows) {
      PackageInfo packageInfo = await PackageInfo.fromPlatform();
      libraryPath = p.join(libraryPath, packageInfo.appName);
    }
    // }
    debugPrint("Library path: $libraryPath");
    return libraryPath;
  }

  // static Future<void> resetProgramDirectory() async {
  // 	final prefs = await SharedPreferences.getInstance();
  // 	await prefs.remove("programDirectory");
  // }

  // static Future<void> setProgramDirectory(String programDirectory) async {
  // 	final prefs = await SharedPreferences.getInstance();
  // 	await prefs.setString("programDirectory", programDirectory);
  // }

  static Future<List<String>> listToolProgram() async {
    final prefs = await SharedPreferences.getInstance();
    debugPrint("Keys: ${prefs.getKeys()}");
    // List the programs that are tools
    return prefs
        .getKeys()
        .where((key) => key.endsWith("-isTool"))
        .where((key) => prefs.getBool(key) ?? false)
        .map((key) => key.substring(0, key.length - 7))
        .toList();
  }

  static Future<void> setToolProgram(String program) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("$program-isTool", true);
  }

  static Future<void> copyProgram(String source, String target) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool(
        "$target-isTool", prefs.getBool("$source-isTool") ?? false);
    await prefs.setBool(
        "$target-withTrace", prefs.getBool("$source-withTrace") ?? false);
    await prefs.setDouble(
        "$target-scrollOffset", prefs.getDouble("$source-scrollOffset") ?? 0.0);
  }

  static Future<void> renameProgram(String oldName, String newName) async {
    await copyProgram(oldName, newName);
    await deleteProgram(oldName);
  }

  static Future<void> deleteProgram(String program) async {
    final prefs = await SharedPreferences.getInstance();
    final keys =
        prefs.getKeys().where((key) => key.startsWith("$program-")).toList();
    for (final key in keys) {
      prefs.remove(key);
    }
  }

  static Future<double> getEditorFontSize() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getDouble("editorFontSize") ?? 16.0;
  }

  static Future<void> setEditorFontSize(double fontSize) async {
    final prefs = await SharedPreferences.getInstance();
    prefs.setDouble("editorFontSize", fontSize);
  }

  static Future<bool> getEditorBigFinger() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getBool("editorBigFinger") ?? false;
  }

  static Future<void> setEditorBigFinger(bool bigFinger) async {
    final prefs = await SharedPreferences.getInstance();
    prefs.setBool("editorBigFinger", bigFinger);
  }

  /// Vertical scroll offset of the code editor, in logical pixels.
  static Future<double> getProgramScrollOffset(String program) async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getDouble("$program-scrollOffset") ?? 0.0;
  }

  static Future<void> setProgramScrollOffset(
      String program, double offset) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setDouble("$program-scrollOffset", offset);
  }

  static Future<MyLibrarySort> getLibrarySort() async {
    final prefs = await SharedPreferences.getInstance();
    return MyLibrarySort.values[(prefs.getInt("librarySort") ?? 0)
        .clamp(0, MyLibrarySort.values.length - 1)];
  }

  static Future<void> setLibrarySort(MyLibrarySort sort) async {
    final prefs = await SharedPreferences.getInstance();
    prefs.setInt("librarySort", sort.index);
  }

  static Future<MyLibraryGrid> getLibraryGrid() async {
    final prefs = await SharedPreferences.getInstance();
    return MyLibraryGrid.values[(prefs.getInt("libraryGrid") ?? 0)
        .clamp(0, MyLibraryGrid.values.length - 1)];
  }

  static Future<void> setLibraryGrid(MyLibraryGrid grid) async {
    final prefs = await SharedPreferences.getInstance();
    prefs.setInt("libraryGrid", grid.index);
  }

  static Future<InstallChange> consumeInstallChange() async {
    final prefs = await SharedPreferences.getInstance();
    final PackageInfo info = await PackageInfo.fromPlatform();
    final String current = "${info.version}+${info.buildNumber}";
    final String? previous = prefs.getString("installedBuild");
    if (previous == current) {
      return InstallChange.unchanged;
    }
    await prefs.setString("installedBuild", current);
    if (previous == null) {
      return InstallChange.freshInstall;
    }
    return compareBuilds(current, previous) > 0
        ? InstallChange.updated
        : InstallChange.downgraded;
  }

  static int compareBuilds(String a, String b) {
    final List<String> as = a.split(RegExp(r'[.+]'));
    final List<String> bs = b.split(RegExp(r'[.+]'));
    final int count = as.length > bs.length ? as.length : bs.length;
    for (int i = 0; i < count; i++) {
      final String av = i < as.length ? as[i] : "0";
      final String bv = i < bs.length ? bs[i] : "0";
      final int? ai = int.tryParse(av);
      final int? bi = int.tryParse(bv);
      final int order =
          (ai != null && bi != null) ? ai.compareTo(bi) : av.compareTo(bv);
      if (order != 0) {
        return order;
      }
    }
    return 0;
  }

  static Future<String?> getGoogleAccount() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString("googleAccount");
  }

  static Future<void> setGoogleAccount(String? email) async {
    final prefs = await SharedPreferences.getInstance();
    if (email == null) {
      prefs.remove("googleAccount");
    } else {
      prefs.setString("googleAccount", email);
    }
  }

  static Future<String?> getSyncBaseline() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString("syncBaseline");
  }

  static Future<void> setSyncBaseline(String json) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString("syncBaseline", json);
  }

  static Future<String?> getSyncHashCache() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString("syncHashCache");
  }

  static Future<void> setSyncHashCache(String json) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString("syncHashCache", json);
  }

  /// True once the one-time rename of absolute-path Drive files ran.
  static Future<bool> getSyncPathMigrated() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getBool("syncPathMigrated") ?? false;
  }

  static Future<void> setSyncPathMigrated() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("syncPathMigrated", true);
    await prefs.remove("lastSynced");
  }

  static Future<DateTime?> getSyncLastFull() async {
    final prefs = await SharedPreferences.getInstance();
    final String? raw = prefs.getString("syncLastFull");
    return raw == null ? null : DateTime.tryParse(raw);
  }

  static Future<void> setSyncLastFull(DateTime when) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString("syncLastFull", when.toIso8601String());
  }

  /// Drops every per-account sync artifact. A baseline left over from another
  /// account lists files the new Drive folder never had, and the next full sync
  /// would read that as "deleted elsewhere" and delete them locally.
  static Future<void> clearSyncState() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.remove("syncBaseline");
    await prefs.remove("syncHashCache");
    await prefs.remove("syncLastFull");
    await prefs.remove("syncPathMigrated");
  }
}

/// A preference notifier for each program.
class MyProgramPreference extends ChangeNotifier {
  final String program;

  MyProgramPreference(this.program);

  /// Store if the program is a tool or not.
  bool _isTool = false;
  bool get isTool => _isTool;

  /// Store if the program should be run with trace or not.
  bool _withTrace = false;
  bool get withTrace => _withTrace;

  Future<MyProgramPreference> loadPreference() async {
    final prefs = await SharedPreferences.getInstance();
    _isTool = prefs.getBool("$program-isTool") ?? false;
    _withTrace = prefs.getBool("$program-withTrace") ?? false;
    return this;
  }

  Future<void> setTool(bool isTool) async {
    _isTool = isTool;
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("$program-isTool", isTool);
  }

  Future<void> setWithTrace(bool withTrace) async {
    _withTrace = withTrace;
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("$program-withTrace", withTrace);
  }
}

class MyEditorPreference extends ChangeNotifier {
  Future<void> init() async {
    _fontSize = await MyPreference.getEditorFontSize();
    _bigFinger = await MyPreference.getEditorBigFinger();
    notifyListeners();
  }

  double _fontSize = 16;
  double get fontSize => _fontSize;
  set fontSize(double value) {
    _fontSize = value;
    MyPreference.setEditorFontSize(value);
    notifyListeners();
  }

  bool _bigFinger = false;
  bool get bigFinger => _bigFinger;
  set bigFinger(bool value) {
    _bigFinger = value;
    MyPreference.setEditorBigFinger(value);
    notifyListeners();
  }
}

class MyLibraryPreference extends ChangeNotifier {
  Future<void> init() async {
    _sort = await MyPreference.getLibrarySort();
    _grid = await MyPreference.getLibraryGrid();
    notifyListeners();
  }

  MyLibrarySort _sort = MyLibrarySort.name;
  MyLibrarySort get sort => _sort;
  set sort(MyLibrarySort value) {
    _sort = value;
    MyPreference.setLibrarySort(value);
    notifyListeners();
  }

  MyLibraryGrid _grid = MyLibraryGrid.medium;
  MyLibraryGrid get grid => _grid;
  set grid(MyLibraryGrid value) {
    _grid = value;
    MyPreference.setLibraryGrid(value);
    notifyListeners();
  }
}
