import 'dart:developer' show log;
import 'dart:io' show Platform;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:lowresrmx/data/program.dart';

import 'package:provider/provider.dart';

import 'package:lowresrmx/page/library_page.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/page/run_page.dart';
import 'package:lowresrmx/theme.dart';

import 'package:core_plugin/core_plugin.dart';

void main() {
	// WidgetsFlutterBinding.ensureInitialized();
  // SystemChrome.setEnabledSystemUIMode(SystemUiMode.manual, overlays: [
  //   // SystemUiOverlay.top,
  //   // SystemUiOverlay.bottom,
  // ]);
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  final Runtime coreRuntime;

  MyApp({super.key}) : coreRuntime = Runtime();

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
    widget.coreRuntime.initState();
  }

  @override
  void dispose() {
    super.dispose();
    widget.coreRuntime.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final MyTheme theme = MyTheme(Theme.of(context).textTheme);
    log("MyApp.build() Not good if called multiple times.");
    return ChangeNotifierProvider<Runtime>(
      create: (context) => widget.coreRuntime,
      child: MaterialApp(
          title: 'Flutter Demo',
          theme: theme.light(),
          darkTheme: theme.dark(),
          highContrastTheme: theme.lightHighContrast(),
          highContrastDarkTheme: theme.darkHighContrast(),
          initialRoute: '/',
          routes: {
            '/': (context) => const MyLibraryPage(),
						// '/edit': (context) => const MyEditPage(),
            // '/': (context) => MyEditPage(),
            // '/run': (context) => MyRunPage(),
          }),
    );
  }
}
