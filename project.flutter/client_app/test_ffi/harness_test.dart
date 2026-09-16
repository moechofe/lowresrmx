// Host side harness for the engine, driven through the core_plugin FFI boundary.
//
// Guards that a plain Dart consumer of the plugin sees the same verdicts as the C harness:
// a failing ASSERT surfaces as a non-zero CoreError, a clean program reaches StateEnd with
// assertions counted, and a bad program fails to compile with the expected error code.
//
// Runs with `dart test test_ffi/`, never `flutter test`: there is no plugin bundle under
// `flutter test`, so DynamicLibrary.open would fail. It imports the generated bindings
// directly rather than core_plugin.dart, which pulls in a Flutter MethodChannel.
//
// This file lives in test_ffi/ and not test/ because .gitignore holds project.flutter/*/test.

import 'dart:ffi' as ffi;
import 'dart:io';

import 'package:core_plugin/core_plugin_bindings_generated.dart';
import 'package:ffi/ffi.dart';
import 'package:test/test.dart';

const String _defaultLibraryPath =
    'build/linux/x64/debug/bundle/lib/libcore_plugin.so';
const String _buildCommand =
    'cd project.flutter/client_app && flutter build linux --debug';

const int _stateEnd =
    5; // enum State: StateNoProgram, Evaluate, Input, Paused, WaitForDisk, End

/// The subset of the '@ annotation language a host side Dart driver can honour: no frame
/// accurate host actions, no golden hashes. Those stay with the C harness.
class Scenario {
  int frames = 600;
  int runs = 1;
  String? expectError;
  String? expectCompileError;
  String? skip;

  /// '@check entries are evaluated by the C harness only; this driver just knows they exist,
  /// so a case verified purely by host checks is not judged on its assertion count.
  int checks = 0;

  static Scenario parse(String source) {
    final Scenario scenario = Scenario();
    for (final String raw in source.split('\n')) {
      final String line = raw.trim();
      if (line.startsWith('#')) break;
      if (!line.startsWith("'@")) continue;

      final List<String> parts = line.substring(2).trim().split(RegExp(r'\s+'));
      final String name = parts.first.toLowerCase().replaceAll('-', '');
      final String value = parts.length > 1 ? parts.sublist(1).join(' ') : '';
      switch (name) {
        case 'frames':
          scenario.frames = int.parse(value);
        case 'runs':
          scenario.runs = int.parse(value);
        case 'expecterror':
          scenario.expectError = value;
        case 'expectcompileerror':
          scenario.expectCompileError = value;
        case 'skip':
          scenario.skip = value.isEmpty ? 'skipped' : value;
        case 'check':
          scenario.checks++;
      }
    }
    return scenario;
  }
}

String _normalize(String text) =>
    text.toLowerCase().replaceAll(RegExp(r'[\s_-]'), '');

class Engine {
  Engine(this._bindings);

  final CorePluginBindings _bindings;

  /// Resolves a numeric code or an ErrorStrings name, the same way test.harness/scenario.c does.
  int errorCode(ffi.Pointer<Runner> runner, String text) {
    final int? numeric = int.tryParse(text);
    if (numeric != null) return numeric;

    final String wanted = _normalize(text);
    for (int code = 1; code < ErrorCode.ErrorMax; code++) {
      final String candidate =
          _bindings.runnerGetError(runner, code).cast<Utf8>().toDartString();
      if (_normalize(candidate) == wanted) return code;
    }
    throw ArgumentError('unknown error code $text');
  }

  String errorName(ffi.Pointer<Runner> runner, int code) =>
      _bindings.runnerGetError(runner, code).cast<Utf8>().toDartString();
}

void main() {
  final String libraryPath =
      Platform.environment['LOWRESRMX_CORE_PLUGIN_SO'] ?? _defaultLibraryPath;

  if (!File(libraryPath).existsSync()) {
    test('engine suite', () {
      markTestSkipped('$libraryPath is missing, build it with: $_buildCommand');
    }, skip: '$libraryPath is missing, build it with: $_buildCommand');
    return;
  }

  final CorePluginBindings bindings = CorePluginBindings(
      ffi.DynamicLibrary.open(File(libraryPath).absolute.path));
  final Engine engine = Engine(bindings);

  // Only the platform independent parts of the suite: io/ needs frame accurate host input and
  // video/ + audio/ need golden hashes, both of which stay with the C harness. The Flutter
  // plugin also builds with ABGR=1, so its frames are a different byte order by design.
  final List<File> cases = <File>[
    for (final String group in <String>['language', 'memory', 'system'])
      ...Directory('../../test.suite/$group')
          .listSync()
          .whereType<File>()
          .where((File file) =>
              file.path.endsWith('.rmx') || file.path.endsWith('.nx'))
  ]..sort((File a, File b) => a.path.compareTo(b.path));

  test('the suite directory was found', () {
    expect(cases, isNotEmpty);
  });

  for (final File file in cases) {
    final String name = file.uri.pathSegments
        .sublist(file.uri.pathSegments.length - 2)
        .join('/');

    test(name, () {
      final String source = file.readAsStringSync();
      final Scenario scenario = Scenario.parse(source);

      final ffi.Pointer<Runner> runner = calloc<Runner>();
      final ffi.Pointer<CoreInput> input = calloc<CoreInput>();
      final ffi.Pointer<Utf8> code = source.toNativeUtf8();
      try {
        input.ref.width = 216;
        input.ref.height = 384;

        // One core for the whole case: runnerCompileProgram keeps the persistent RAM
        // (core_compileProgram with resetPersistent false), which is what makes '@runs work
        // here without a sandbox file.
        bindings.runnerInit(runner);

        for (int run = 0; run < scenario.runs; run++) {
          final CoreError compileError =
              bindings.runnerCompileProgram(runner, code.cast<ffi.Char>());
          if (scenario.expectCompileError != null) {
            expect(compileError.code,
                engine.errorCode(runner, scenario.expectCompileError!),
                reason: 'expected a compile error, got '
                    '${engine.errorName(runner, compileError.code)}');
            return;
          }
          expect(compileError.code, 0,
              reason:
                  'unexpected compile error ${engine.errorName(runner, compileError.code)}');

          int frames = 0;
          CoreError runtimeError = compileError;
          bindings.runnerStart(runner, input, 0, ffi.nullptr, 0);
          while (frames < scenario.frames &&
              bindings.runnerState(runner) != _stateEnd) {
            runtimeError = bindings.runnerUpdate(runner, input);
            frames++;
            if (runtimeError.code != 0) break;
          }

          if (run + 1 < scenario.runs) {
            expect(runtimeError.code, 0,
                reason: 'run ${run + 1} failed with '
                    '${engine.errorName(runner, runtimeError.code)}');
            continue;
          }

          if (scenario.expectError != null) {
            expect(runtimeError.code,
                engine.errorCode(runner, scenario.expectError!),
                reason: 'expected ${scenario.expectError}, got '
                    '${engine.errorName(runner, runtimeError.code)}');
          } else {
            expect(runtimeError.code, 0,
                reason:
                    '${engine.errorName(runner, runtimeError.code)} after $frames frames');
            expect(bindings.runnerState(runner), _stateEnd,
                reason: 'still running after $frames frames');
            if (scenario.checks == 0) {
              expect(bindings.runnerNumAssertions(runner), greaterThan(0),
                  reason: 'no assertions ran');
            }
          }
        }
      } finally {
        bindings.runnerDeinit(runner);
        calloc.free(code);
        calloc.free(input);
        calloc.free(runner);
      }
    }, skip: scenarioSkipReason(file));
  }
}

String? scenarioSkipReason(File file) =>
    Scenario.parse(file.readAsStringSync()).skip;
