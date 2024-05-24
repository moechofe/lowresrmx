import 'dart:developer' show log;
import 'dart:io';

import 'package:flutter/foundation.dart';

import 'package:package_info_plus/package_info_plus.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;

import 'package:lowresrmx/data/program.dart';

abstract class MyLibrary
{
	static String extension = ".rmx";

	static Future<Directory> getLibraryDir() async {
		final documentDir = await getApplicationDocumentsDirectory();
		String libraryPath = documentDir.path;
		if (defaultTargetPlatform == TargetPlatform.linux || defaultTargetPlatform == TargetPlatform.windows) {
      PackageInfo packageInfo = await PackageInfo.fromPlatform();
      libraryPath = p.join(libraryPath, packageInfo.appName);
    }
		log("libraryPath: $libraryPath");
		final Directory libraryDir = Directory(libraryPath);
		if (!await libraryDir.exists()) await libraryDir.create(recursive: true);
		return libraryDir;
	}

	static Future<File> getCodeFile(String programName) async {
		final documentDir = await getLibraryDir();
		final codePath = p.join(documentDir.path, "$programName$extension");
		final codeFile = File(codePath);
		if (await codeFile.exists()) {
			return codeFile;
		} else {
			throw "Fail to read code for $programName";
		}
	}

	static Future<File> getThumbFile(String programName) async {
		final documentDir = await getLibraryDir();
		final codePath = p.join(documentDir.path, "$programName.png");
		final codeFile = File(codePath);
		if (await codeFile.exists()) {
			return codeFile;
		} else {
			throw "Fail to read thumbnail for $programName";
		}
	}

	static Future<File> createProgram() async {
		final Directory libraryDir = await getLibraryDir();
		String programPath = p.join(libraryDir.path, "unnamed$extension");
		File programFile = File(programPath);
		int counter = -1;
		while (await programFile.exists())
		{
			counter+=1;
			programPath = p.join(libraryDir.path, "unnamed $counter$extension");
			programFile = File(programPath);
		}
		await programFile.create();
		log("created: $programPath");
		return programFile;
	}

	static Future<List<MyProgram>> buildList() async {
		final Directory libraryDir = await getLibraryDir();
		return libraryDir.list().where((entry) {
			log("entry: ${entry.path}");
			return p.extension(entry.path)==extension; }).map((entry) => MyProgram.fromFSE(entry)).toList();
	}

	// TODO: convert to UPPERCASE using settings
	static Future<String> readCode(String programName) async {
		final File codeFile = await getCodeFile(programName);
		return codeFile.readAsString();
	}

	static Future<void> writeCode(String programName, String code) async {
		final File codeFile = await getCodeFile(programName);
		codeFile.writeAsString(code);
	}
}
