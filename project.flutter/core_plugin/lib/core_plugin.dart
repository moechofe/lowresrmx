
import 'dart:ffi' as ffi;
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'core_plugin_bindings_generated.dart';

const String _libName = 'core_plugin';
const MethodChannel _channel = MethodChannel('com.lowresrmx/core_plugin');

Future<int> registerTexture() async {
  return (await _channel.invokeMethod<int>('registerTexture'))!;
}

Future<void> unregisterTexture(int textureId) async {
  await _channel.invokeMethod('unregisterTexture', textureId);
}

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
void runnerDeinit(ffi.Pointer<Runner> runner) => _bindings.runnerDeinit(runner);

void runnerRegisterNativeTexture(int textureId, ffi.Pointer<ffi.Void> nativeHandle) => _bindings.runnerRegisterNativeTexture(textureId, nativeHandle);
void runnerUnregisterNativeTexture(int textureId) => _bindings.runnerUnregisterNativeTexture(textureId);
void runnerRenderToTexture(ffi.Pointer<Runner> runner, int textureId) => _bindings.runnerRenderToTexture(runner, textureId);

CoreError runnerCompileProgram(ffi.Pointer<Runner> runner, String code) => _bindings.runnerCompileProgram(runner, code.toNativeUtf8().cast<ffi.Char>());

String runnerGetError(ffi.Pointer<Runner> runner, int code) => _bindings.runnerGetError(runner, code).cast<Utf8>().toDartString();

void runnerStart(ffi.Pointer<Runner> runner, int scondsSincePowerOn, ffi.Pointer<ffi.Char> originalDataDisk, int originalDataDiskSize) => _bindings.runnerStart(runner, scondsSincePowerOn, originalDataDisk, originalDataDiskSize);
bool runnerShouldRender(ffi.Pointer<Runner> runner) => _bindings.runnerShouldRender(runner);
CoreError runnerUpdate(ffi.Pointer<Runner> runner, ffi.Pointer<Input> input) => _bindings.runnerUpdate(runner, input);
void runnerRender(ffi.Pointer<Runner> runner, ffi.Pointer<ffi.Uint8> pixels) => _bindings.runnerRender(runner, pixels.cast<ffi.Void>());

void runnerTrace(ffi.Pointer<Runner> runner, bool trace) => _bindings.runnerTrace(runner, trace);

int runnerGetSymbolCount(ffi.Pointer<Runner> runner) => _bindings.runnerGetSymbolCount(runner);
String runnerGetSymbolName(ffi.Pointer<Runner> runner, int index) => _bindings.runnerGetSymbolName(runner, index).cast<Utf8>().toDartString();
int runnerGetSymbolPosition(ffi.Pointer<Runner> runner, int index) => _bindings.runnerGetSymbolPosition(runner, index);

void inputKeyDown(ffi.Pointer<Input> input, int key) => _bindings.inputKeyDown(input, key);

void runnerRenderAudio(ffi.Pointer<Runner> runner, ffi.Pointer<ffi.Int16> output, int numSamples, int outputFrequency, int volume) => _bindings.runnerRenderAudio(runner, output, numSamples, outputFrequency, volume);
