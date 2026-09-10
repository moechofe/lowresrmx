import 'dart:developer';

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
		log("Library path: $libraryPath");
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
		log("Keys: ${prefs.getKeys()}");
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

  static Future<void> renameProgram(String oldName, String newName) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool("$newName-isTool", prefs.getBool("$oldName-isTool") ?? false);
    await prefs.setBool("$newName-withTrace", prefs.getBool("$oldName-withTrace") ?? false);
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

	static Future<MyLibrarySort> getLibrarySort() async {
		final prefs = await SharedPreferences.getInstance();
		return MyLibrarySort.values[(prefs.getInt("librarySort")??0).clamp(0,MyLibrarySort.values.length-1)];
	}

	static Future<void> setLibrarySort(MyLibrarySort sort) async {
		final prefs = await SharedPreferences.getInstance();
		prefs.setInt("librarySort",sort.index);
	}

	static Future<MyLibraryGrid> getLibraryGrid() async {
		final prefs = await SharedPreferences.getInstance();
		return MyLibraryGrid.values[(prefs.getInt("libraryGrid")??0).clamp(0,MyLibraryGrid.values.length-1)];
	}

	static Future<void> setLibraryGrid(MyLibraryGrid grid) async {
		final prefs = await SharedPreferences.getInstance();
		prefs.setInt("libraryGrid",grid.index);
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
	}

	double _fontSize = 16;
	double get fontSize => _fontSize;
	set fontSize(double value) {
		_fontSize = value;
		MyPreference.setEditorFontSize(value);
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

	MyLibraryGrid _grid = MyLibraryGrid.three;
	MyLibraryGrid get grid => _grid;
	set grid(MyLibraryGrid value) {
		_grid = value;
		MyPreference.setLibraryGrid(value);
		notifyListeners();
	}
}
