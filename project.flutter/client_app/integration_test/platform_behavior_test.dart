// Device level guards for the edges the widget tests in test/ cannot reach: the real
// core_plugin library, a real render isolate and the real platform channels.
//
// Run with `flutter test integration_test/platform_behavior_test.dart -d <device-id>`.
//
// **Never call pumpAndSettle here.** MyRunPage starts ComPort's 60 Hz Ticker in initState, so a
// frame is always scheduled and the tree never settles — pumpAndSettle just burns its timeout.
// Pump a bounded number of frames instead, with real time passing between them so the run
// isolate gets to answer.
//
// Deliberately not covered here: SYSTEM 9 portrait lock. The engine tracks it in
// interpreter->lockPortrait and reports it in ControlsInfo.isPortraitLocked, but
// core_plugin.c's controlsDidChange drops that field and the client app never calls
// SystemChrome.setPreferredOrientations, so there is no Flutter behaviour to assert.
// test.suite/system/portrait_lock.rmx covers the engine side through the C harness.

import 'dart:isolate';

import 'package:core_plugin/core_plugin.dart' show unregisterTexture;
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:provider/provider.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/page/run_page.dart';

/// Mirrors what MyEditPage hands to MyRunPage.
Widget harness(ComPort comPort) => Provider<ComPort>.value(
      value: comPort,
      child: MaterialApp(
        home: MyRunPage(
          comPort: comPort,
          editingName: "GAME",
          dataDiskName: "GAME.disk",
          executedName: "GAME",
        ),
      ),
    );

/// Pumps until [condition] holds or the budget runs out. Returns the final value of [condition].
Future<bool> pumpUntil(WidgetTester tester, bool Function() condition,
    {Duration budget = const Duration(seconds: 10)}) async {
  final Stopwatch watch = Stopwatch()..start();
  while (watch.elapsed < budget) {
    if (condition()) return true;
    await tester.pump(const Duration(milliseconds: 16));
    await Future<void>.delayed(const Duration(milliseconds: 4));
  }
  return condition();
}

/// Pumps for a fixed wall-clock duration, letting the run isolate make progress.
Future<void> pumpFor(WidgetTester tester, Duration duration) async {
  await pumpUntil(tester, () => false, budget: duration);
}

/// BASIC tail that tells the host "the program got here".
///
/// The host can only observe a running program through ComPort's callbacks, and all of them
/// except onRunningError are edge triggered on isolate-scoped state, which leaks between the
/// tests that share one ComPort. A deliberate illegal POKE is not: it always raises
/// `Illegal Memory Access`, and it is a different code from the `Assertion Failed` an actual
/// broken assertion in the same program would raise, so the two never get confused.
const String signalReached = 'seen:\npoke \$ff88,1\n';

/// Asserts the program signalled [signalReached] rather than failing an ASSERT or timing out.
void expectReached(Error? reported, String reason) {
  expect(reported, isNotNull, reason: reason);
  expect(reported!.msg, contains('Illegal Memory Access'),
      reason: 'the program failed instead: ${reported.msg}');
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late ComPort comPort;

  // A port per test, because everything in ComPort's isolate is sticky and would otherwise
  // leak across tests: Runner.runningError keeps being reported every frame until the next
  // runnerStart, the KeyboardVisibleMsg edge detector remembers the last value, and
  // Runtime.resize only forwards a keyboard inset that *changed*.
  setUp(() async {
    SharedPreferences.setMockInitialValues({});
    comPort = ComPort();
    await comPort.init();
  });

  // Order matters: the isolate blits straight into the platform surface, so the ticker has to
  // stop and the last frame has to land before the isolate dies, and the isolate has to be
  // gone before the texture is released. Killing mid-blit takes the whole app process down.
  tearDown(() async {
    comPort.stop();
    await Future<void>.delayed(const Duration(milliseconds: 200));
    comPort.isolate.kill(priority: Isolate.beforeNextEvent);
    await Future<void>.delayed(const Duration(milliseconds: 200));
    comPort.receivePort.close();
    final int? textureId = comPort.textureId;
    if (textureId != null) {
      await unregisterTexture(textureId);
    }
  });

  /// Mounts the run page and makes sure the engine is actually being stepped.
  ///
  /// MyRunPage starts ComPort's ticker from initState and stops it from dispose, but
  /// flutter_test reuses the widget tree between tests, so a mount is not guaranteed to
  /// re-run initState — and a stale stop from the previous test then leaves the run loop dead
  /// and every expectation below silently timing out. Start it explicitly instead.
  Future<void> mountRunPage(WidgetTester tester) async {
    await tester.pumpWidget(harness(comPort));
    await pumpFor(tester, const Duration(milliseconds: 300));
    if (!comPort.ticker.isActive) comPort.start();
    await pumpFor(tester, const Duration(milliseconds: 300));
    expect(comPort.ticker.isActive, isTrue,
        reason: 'the run loop is not ticking');
  }

  // A failing ASSERT is an ordinary ErrorCode, so it has to travel the whole way out:
  // interpreterDidFail -> Runner.runningError -> runnerUpdate -> RunningErrorMsg ->
  // ComPort.onRunningError -> the AlertDialog raised by _MyRunPageState.reportError.
  testWidgets(
      'a failing ASSERT reaches onRunningError and shows the error dialog',
      (tester) async {
    await mountRunPage(tester);

    Error? reported;
    // The run page installs its own one-shot handler in initState; chain onto it.
    final RunnerErrorCallback? pageHandler = comPort.onRunningError;
    comPort.onRunningError = (Error error) {
      reported = error;
      pageHandler?.call(error);
    };

    final Error compileError =
        await comPort.compileAndRun("assert 1=2\nend\n", "");
    expect(compileError.code, 0,
        reason: 'the program must compile: ${compileError.msg}');

    expect(await pumpUntil(tester, () => reported != null), isTrue,
        reason: 'no running error arrived');
    expect(reported!.code, isNot(0));
    expect(reported!.msg, contains('Assertion Failed'));

    expect(
        await pumpUntil(tester, () => find.text("Error").evaluate().isNotEmpty),
        isTrue,
        reason: 'the error dialog never appeared');
  });

  // =KEYBOARD reads ioRegisters.keyboardHeight, which only CoreInput.keyboardHeight feeds.
  // The widget test in test/keyboard_inset_test.dart stubs ComPort and stops at the resize()
  // arguments; this one drives the real port, so the inset also has to survive the isolate hop
  // and land in the engine.
  //
  // The inset must come from the view, not from a direct comPort.resize() call: MyRunPage's
  // LayoutBuilder pushes the real (zero) inset on every layout, the isolate applies whichever
  // resize arrived last before a frame, and Runtime.resize only forwards a *changed* value.
  //
  // What the engine ends up seeing is in fantasy pixels, not logical ones — Runtime.resize
  // divides by _screenScale — so assert it is non-zero rather than pinning that arithmetic.
  testWidgets('the device keyboard inset reaches the real runtime',
      (tester) async {
    addTearDown(tester.view.reset);
    tester.view.devicePixelRatio = 2.0;
    tester.view.physicalSize = const Size(720, 1600);

    await mountRunPage(tester);

    Error? reported;
    comPort.onRunningError = (Error error) {
      reported = error;
    };

    final Error compileError = await comPort.compileAndRun(
        "do\n  if keyboard>0 then goto seen\n  wait vbl\nloop\n"
            "$signalReached",
        "");
    expect(compileError.code, 0,
        reason: 'the program must compile: ${compileError.msg}');

    await pumpFor(tester, const Duration(milliseconds: 300));
    // The platform reports the keyboard as a bottom view inset, in physical pixels.
    tester.view.viewInsets = const FakeViewPadding(bottom: 672);

    await pumpUntil(tester, () => reported != null);
    expectReached(
        reported, '=KEYBOARD never reported the inset the host pushed');
  });

  // Regression guard for 4042dfa at device level: the screen texture must absorb pointers, or
  // TOUCH stays zero. test/screen_paint_touch_test.dart pins the widget tree; this pins the
  // real page, whose Texture is backed by a live platform surface.
  testWidgets('a tap on the screen texture reaches the engine', (tester) async {
    await mountRunPage(tester);

    Error? reported;
    comPort.onRunningError = (Error error) => reported = error;

    final Error compileError = await comPort.compileAndRun(
        "do\n  if touch then goto pressed\n  wait vbl\nloop\n"
            "pressed:\nassert touch.x>=0\nassert touch.y>=0\n"
            "$signalReached",
        "");
    expect(compileError.code, 0,
        reason: 'the program must compile: ${compileError.msg}');

    final Finder texture = find.byType(Texture);
    expect(texture, findsOneWidget);

    final HitTestResult result =
        tester.hitTestOnBinding(tester.getCenter(texture));
    expect(result.path.any((HitTestEntry entry) => entry.target is TextureBox),
        isTrue,
        reason: 'the texture must absorb the pointer');

    await pumpFor(tester, const Duration(milliseconds: 300));
    final TestGesture gesture =
        await tester.startGesture(tester.getCenter(texture));
    await pumpFor(tester, const Duration(milliseconds: 500));
    await gesture.up();

    await pumpUntil(tester, () => reported != null);
    expectReached(reported, 'the touch never reached the engine');
  });

  // KEYBOARD ON must actually open the system keyboard: the engine reports it through
  // ControlsInfo.keyboardMode, core_plugin turns it into Runner.shouldOpenKeyboard and the
  // run page answers with TextInput.show. Only a real device has that channel.
  //
  // KeyboardVisibleMsg is edge triggered and the isolate outlives a single program, so the
  // program closes the keyboard first: OFF then ON yields a rising edge from any prior state.
  testWidgets('KEYBOARD ON asks the platform for the system keyboard',
      (tester) async {
    await mountRunPage(tester);

    Error? reported;
    comPort.onRunningError = (Error error) => reported = error;

    bool? visible;
    final KeyboardVisibleCallback? pageHandler = comPort.onKeyboardVisible;
    comPort.onKeyboardVisible = (bool open) {
      visible = open;
      pageHandler?.call(open);
    };

    final Error compileError = await comPort.compileAndRun(
        "keyboard off\nwait 4\nkeyboard on\nwait 4\n$signalReached", "");
    expect(compileError.code, 0,
        reason: 'the program must compile: ${compileError.msg}');

    await pumpUntil(tester, () => reported != null);
    expectReached(reported, 'the program never ran');
    expect(visible, isTrue, reason: 'the engine never asked for the keyboard');
  });
}
