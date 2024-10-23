import 'dart:developer' show log;
import 'package:flutter/material.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/page/library_page.dart';
import 'package:lowresrmx/theme.dart';
import 'package:provider/provider.dart';

late final ComPort comPort;

void main() async {
  comPort = ComPort();
  await comPort.init();
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => MyAppState();
}

class MyAppState extends State<MyApp> {

  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final MyTheme theme = MyTheme(Theme.of(context).textTheme);
    log("MyApp.build() Not good if called multiple times.");
    return MultiProvider(
        providers: [
          ChangeNotifierProvider<MyLibrary>(create: (_) => MyLibrary()),
          Provider<ComPort>(create: (_) => comPort),
          ChangeNotifierProvider<MyEditorPreference>(
              create: (_) => MyEditorPreference()),
        ],
        child: MaterialApp(
        	debugShowCheckedModeBanner: false,
        	title: 'LowResRMX',
        	theme: theme.light(),
        	darkTheme: theme.dark(),
        	highContrastTheme: theme.lightHighContrast(),
        	highContrastDarkTheme: theme.darkHighContrast(),
					restorationScopeId: 'root',
        	initialRoute: '/',
        	routes: {
        		'/': (context) => const MyLibraryPage(),
						MyEditPage.routeName: (context) => const MyEditPage(),
        	},
        ));
  }
}
