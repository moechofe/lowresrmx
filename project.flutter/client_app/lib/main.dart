import 'dart:developer' show log;
import 'package:flutter/material.dart';
import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/library.dart';
import 'package:lowresrmx/data/preference.dart';
import 'package:lowresrmx/page/edit_page.dart';
import 'package:lowresrmx/page/library_page.dart';
import 'package:lowresrmx/theme.dart';
import 'package:provider/provider.dart';

import 'dart:math' as math;
import 'dart:async';
import 'dart:typed_data';
import 'package:mp_audio_stream/mp_audio_stream.dart';

late final ComPort comPort;

void main() async {
  comPort = ComPort();
  await comPort.init();
  runApp(const MyApp());


  final audioStream = getAudioStream();

  //default init params: {int bufferMilliSec = 3000,
  //                      int waitingBufferMilliSec = 100,
  //                      int channels = 1,
  //                      int sampleRate = 44100}
  audioStream.init(channels: 2); //Call this from Flutter's State.initState() method

  //For web platform, call this after user interaction
  audioStream.resume();

  // generating a stereo sine-wave PCM stream
  const rate = 44100;
  const freqL = 440;
  const freqR = 660;
  const dur = 10;

  Float32List samples = Float32List(rate);

  for (var t = 0; t < dur; t++) {
    int pos = 0;

    for (var i = 0; i < rate; i++) {
      samples[pos++] = math.sin(2 * math.pi * ((i * freqL) % rate) / rate);
      samples[pos++] = math.sin(2 * math.pi * ((i * freqR) % rate) / rate);

      if (pos == samples.length) {
        pos = 0;

        // playback the generated PCM stream
        audioStream.push(samples);
      }
    }
    if (t > 0) {
      await Future.delayed(const Duration(seconds: 1));
    }
  }

  //Call this from Flutter's State.dispose()\
  audioStream.uninit();
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
