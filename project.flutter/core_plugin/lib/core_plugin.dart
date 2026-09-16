
import 'dart:ffi' as ffi;
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'core_plugin_bindings_generated.dart';

const String _libName = 'core_plugin';
const MethodChannel _channel = MethodChannel('com.lowresrmx/core_plugin');

/// A platform surface the engine renders into, sized in device pixels.
class TextureSurface {
  const TextureSurface({required this.textureId, required this.address, required this.bytesPerRow});

  final int textureId;

  /// Base address of the surface memory; 0 when C already holds a native handle (Android).
  final int address;

  /// Bytes per row; 0 when C takes the pitch from the surface at render time (Android).
  final int bytesPerRow;

  static TextureSurface fromMap(Map<Object?, Object?> map) => TextureSurface(
      textureId: map["textureId"]! as int,
      address: map["address"]! as int,
      bytesPerRow: map["bytesPerRow"]! as int);
}

Future<TextureSurface> registerTexture(int width, int height) async =>
    TextureSurface.fromMap((await _channel.invokeMapMethod<Object?, Object?>(
        'registerTexture', {"width": width, "height": height}))!);

Future<TextureSurface> resizeTexture(int textureId, int width, int height) async =>
    TextureSurface.fromMap((await _channel.invokeMapMethod<Object?, Object?>(
        'resizeTexture', {"textureId": textureId, "width": width, "height": height}))!);

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
void runnerSetTextureGeometry(int textureId, int width, int height, int pitch) => _bindings.runnerSetTextureGeometry(textureId, width, height, pitch);

CoreError runnerCompileProgram(ffi.Pointer<Runner> runner, String code) {
  final ffi.Pointer<Utf8> native = code.toNativeUtf8();
  try {
    return _bindings.runnerCompileProgram(runner, native.cast<ffi.Char>());
  } finally {
    malloc.free(native);
  }
}

String runnerGetError(ffi.Pointer<Runner> runner, int code) => _bindings.runnerGetError(runner, code).cast<Utf8>().toDartString();

void runnerStart(ffi.Pointer<Runner> runner, ffi.Pointer<Input> input, int scondsSincePowerOn, ffi.Pointer<ffi.Char> originalDataDisk, int originalDataDiskSize) => _bindings.runnerStart(runner, input, scondsSincePowerOn, originalDataDisk, originalDataDiskSize);
bool runnerShouldRender(ffi.Pointer<Runner> runner) => _bindings.runnerShouldRender(runner);
CoreError runnerUpdate(ffi.Pointer<Runner> runner, ffi.Pointer<Input> input) => _bindings.runnerUpdate(runner, input);
void runnerRender(ffi.Pointer<Runner> runner, ffi.Pointer<ffi.Uint8> pixels) => _bindings.runnerRender(runner, pixels.cast<ffi.Void>());

void runnerTrace(ffi.Pointer<Runner> runner, bool trace) => _bindings.runnerTrace(runner, trace);

int runnerNumAssertions(ffi.Pointer<Runner> runner) => _bindings.runnerNumAssertions(runner);
int runnerState(ffi.Pointer<Runner> runner) => _bindings.runnerState(runner);

ffi.Pointer<ffi.Void> syntaxCreate() => _bindings.syntaxCreate();
void syntaxDestroy(ffi.Pointer<ffi.Void> syntax) => _bindings.syntaxDestroy(syntax);
int syntaxScan(ffi.Pointer<ffi.Void> syntax, String sourceCode) {
  final ffi.Pointer<Utf8> native = sourceCode.toNativeUtf8();
  try {
    return _bindings.syntaxScan(syntax, native.cast<ffi.Char>());
  } finally {
    malloc.free(native);
  }
}

ffi.Pointer<SyntaxSpan> syntaxSpans(ffi.Pointer<ffi.Void> syntax) => _bindings.syntaxSpans(syntax);

void inputKeyDown(ffi.Pointer<Input> input, int key) => _bindings.inputKeyDown(input, key);

void runnerRenderAudio(ffi.Pointer<Runner> runner, ffi.Pointer<ffi.Int16> output, int numSamples, int outputFrequency, int volume) => _bindings.runnerRenderAudio(runner, output, numSamples, outputFrequency, volume);
