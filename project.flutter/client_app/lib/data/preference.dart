import 'package:shared_preferences/shared_preferences.dart';

abstract class MyPreference {
	static Future<bool> programIsTool(String program) async {
		final prefs = await SharedPreferences.getInstance();
		return prefs.getBool('programIsTool-$program') ?? false;
	}

	static Future<void> setProgramIsTool(String program, bool isTool) async {
		final prefs = await SharedPreferences.getInstance();
		await prefs.setBool('programIsTool-$program', isTool);
	}
}
