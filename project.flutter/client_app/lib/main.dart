import 'dart:io';
import 'package:flutter/material.dart';
import 'package:lowresrmx/app_keys.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/import_service.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/data/sync_manager.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/page/library_page.dart';
import 'package:lowresrmx/theme.dart';
import 'package:provider/provider.dart';

late final ComPort comPort;

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  final InstallChange change = await MyPreference.consumeInstallChange();
  if (change != InstallChange.unchanged) {
    debugPrint("main() install change: $change");
    await onInstallChanged(change);
  }

  comPort = ComPort();
  await comPort.init();
  runApp(const MyApp());
}

Future<void> onInstallChanged(InstallChange change) async {
  if (change == InstallChange.downgraded) {
    return;
  }
  await MyLibrary.reinstallDefaultPrograms();
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
    if (Platform.isAndroid || Platform.isIOS) {
      WidgetsBinding.instance
          .addPostFrameCallback((_) => MyImportService().start());
    }
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final MyTheme theme = MyTheme(Theme.of(context).textTheme);
    debugPrint("MyApp.build() Not good if called multiple times.");
    return MultiProvider(
        providers: [
          ChangeNotifierProvider<SyncManager>(create: (_) => SyncManager()),
          ChangeNotifierProvider<MyLibrary>(create: (_) => MyLibrary()),
          Provider<ComPort>(create: (_) => comPort),
          ChangeNotifierProvider<MyEditorPreference>(
              create: (_) => MyEditorPreference()..init()),
          ChangeNotifierProvider<MyLibraryPreference>(
              create: (_) => MyLibraryPreference()..init()),
        ],
        child: MaterialApp(
          debugShowCheckedModeBanner: false,
          title: 'LowResRMX',
          theme: theme.light(),
          darkTheme: theme.dark(),
          highContrastTheme: theme.lightHighContrast(),
          highContrastDarkTheme: theme.darkHighContrast(),
          restorationScopeId: 'root',
          navigatorKey: appNavigatorKey,
          scaffoldMessengerKey: appMessengerKey,
          initialRoute: '/',
          routes: {
            '/': (context) => const MyLibraryPage(),
            MyEditPage.routeName: (context) => const MyEditPage(),
          },
        ));
  }
}
