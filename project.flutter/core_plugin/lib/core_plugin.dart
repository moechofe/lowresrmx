
import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

import 'core_plugin_bindings_generated.dart';

const String _libName = 'core_plugin';

/// The dynamic library in which the symbols for [CorePluginBindings] can be found.
final ffi.DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return ffi.DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return ffi.DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final CorePluginBindings _bindings = CorePluginBindings(_dylib);

void runnerInit(ffi.Pointer<Runner> runner) => _bindings.runnerInit(runner);
void runnerSetDelegate(ffi.Pointer<Runner> runner, ffi.Pointer<CoreDelegate> delegate) => _bindings.runnerSetDelegate(runner, delegate);
void runnerDeinit(ffi.Pointer<Runner> runner) => _bindings.runnerDeinit(runner);

CoreError runnerCompileProgram(ffi.Pointer<Runner> runner, String code) => _bindings.runnerCompileProgram(runner, code.toNativeUtf8().cast<ffi.Char>());

String runnerGetError(ffi.Pointer<Runner> runner, int code) => _bindings.runnerGetError(runner, code).cast<Utf8>().toDartString();

void runnerStart(ffi.Pointer<Runner> runner, int scondsSincePowerOn) => _bindings.runnerStart(runner, scondsSincePowerOn);
CoreError runnerUpdate(ffi.Pointer<Runner> runner, ffi.Pointer<Input> input) => _bindings.runnerUpdate(runner, input);
void runnerRender(ffi.Pointer<Runner> runner, ffi.Pointer<ffi.Uint8> pixels) => _bindings.runnerRender(runner, pixels.cast<ffi.Void>());

void runnerTrace(ffi.Pointer<Runner> runner, bool trace) => _bindings.runnerTrace(runner, trace);

typedef InterpreterDidFailFunc = ffi.Void Function(ffi.Pointer<ffi.Void> context, CoreError coreError);
typedef InterpreterDriveWillAccessFunc = ffi.Bool Function(ffi.Pointer<ffi.Void> context, ffi.Pointer<DataManager> dataManager);
typedef InterpreterDriveDidSaveFunc = ffi.Void Function(ffi.Pointer<ffi.Void> context, ffi.Pointer<DataManager> dataManager);
typedef InterpreterControlsDidChangeFunc = ffi.Void Function(ffi.Pointer<ffi.Void> context, ControlsInfo controlsInfo);
typedef InterpreterPersistentRamWillAccessFunc = ffi.Void Function(ffi.Pointer<ffi.Void> context, ffi.Pointer<ffi.Uint8> destination, ffi.Int size);
typedef InterpreterPersistentRamDidChangeFunc = ffi.Void Function(ffi.Pointer<ffi.Void> context, ffi.Pointer<ffi.Uint8> source, ffi.Int size);



