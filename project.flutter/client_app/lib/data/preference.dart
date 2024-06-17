import 'package:flutter/foundation.dart';

import 'package:package_info_plus/package_info_plus.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;

import 'package:shared_preferences/shared_preferences.dart';

abstract class MyPreference {
	static Future<String> getProgramDirectory() async {
		final prefs = await SharedPreferences.getInstance();
		String? libraryPath = prefs.getString("programDirectory");
		if (libraryPath == null) {
			libraryPath = (await getApplicationDocumentsDirectory()).path;
			if (defaultTargetPlatform == TargetPlatform.linux ||
					defaultTargetPlatform == TargetPlatform.windows) {
				PackageInfo packageInfo = await PackageInfo.fromPlatform();
				libraryPath = p.join(libraryPath, packageInfo.appName);
			}
		}
		return libraryPath;
	}

	static Future<void> setProgramDirectory(String programDirectory) async {
		final prefs = await SharedPreferences.getInstance();
		await prefs.setString("programDirectory", programDirectory);
	}

  static Future<List<String>> listToolProgram() async {
    final prefs = await SharedPreferences.getInstance();
		// List the programs that are tools
    return prefs
        .getKeys()
        .where((key) => key.endsWith("-isTool"))
        .where((key) => prefs.getBool(key) ?? false)
        .map((key) => key.substring(0, key.length - 7))
        .toList();
  }

  static Future<void> renameProgram(String oldName, String newName) async {
    final prefs = await SharedPreferences.getInstance();
    final isTool = prefs.getBool("$oldName-isTool") ?? false;
    await prefs.setBool("$oldName-isTool", isTool);
    final withTrace = prefs.getBool("$oldName-withTrace") ?? false;
    await prefs.setBool("$oldName-withTrace", withTrace);
    await deleteProgram(oldName);
  }

  static Future<void> deleteProgram(String program) async {
    final prefs = await SharedPreferences.getInstance();
    final keys =
        prefs.getKeys().where((key) => key.startsWith("$program-")).toList();
    for (final key in keys) {
      await prefs.remove(key);
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
