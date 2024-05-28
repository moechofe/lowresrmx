import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';

abstract class MyPreference {
	static Future<bool> programIsTool(String program) async {
		final prefs = await SharedPreferences.getInstance();
		return prefs.getBool('programIsTool-$program') ?? false;
	}

	static Future<bool> setProgramIsTool(String program, bool isTool) async {
		final prefs = await SharedPreferences.getInstance();
		return await prefs.setBool('programIsTool-$program', isTool);
	}

	static Future<List<String>> listToolProgram() async {
		final prefs = await SharedPreferences.getInstance();
		final keys = prefs.getKeys();
		return keys.where((key) => key.startsWith('programIsTool-')).map((key) => key.substring('programIsTool-'.length)).toList();
	}

	static Future<void> renameProgram(String oldName, String newName) async {
		final prefs = await SharedPreferences.getInstance();
		final isTool = prefs.getBool('programIsTool-$oldName') ?? false;
		await prefs.remove('programIsTool-$oldName');
		await prefs.setBool('programIsTool-$newName', isTool);
	}

	static Future<void> deleteProgram(String program) async {
		final prefs = await SharedPreferences.getInstance();
		await prefs.remove('programIsTool-$program');
	}
}

/// A preference notifier for each program.
class MyProgramPreference extends ChangeNotifier {
	final String program;

	MyProgramPreference(this.program);

	/// Store if the program is a tool or not.
	bool _isTool = false;
	bool get isTool => _isTool;

	Future<MyProgramPreference> loadPreference() async {
		_isTool = await MyPreference.programIsTool(program);
		return this;
	}

	Future<void> setTool(bool isTool) async {
		_isTool = isTool;
		await MyPreference.setProgramIsTool(program, isTool);
		notifyListeners();
	}

}
