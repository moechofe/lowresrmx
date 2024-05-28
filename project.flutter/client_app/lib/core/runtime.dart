import 'dart:ffi' as ffi;
import 'dart:ui' as ui;

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:ffi/ffi.dart' show calloc;
import 'package:image/image.dart' as img;
import 'package:lowresrmx/data/library.dart';
import 'package:mp_audio_stream/mp_audio_stream.dart';

import 'package:core_plugin/core_plugin_bindings_generated.dart';
import 'package:core_plugin/core_plugin.dart';

class Error {
  int code;
  String msg;
  int line;

  Error({required this.code, required this.msg, required this.line});

  bool get ok => code == 0;

  @override
  String toString() => "Error #$code at pos: $line: $msg";
}

class Runtime extends ChangeNotifier {
  static int screenWidth = 216;
  static int screenHeight = 384;
  static int thumbWidth = 180;
  static int thumbHeight = 180;
  static int bytePerPixel = 4;
  static int bufferSize = screenWidth * screenHeight * bytePerPixel;
  Uint8List? bytesList;
  ui.Image? image;

  final ffi.Pointer<Input> input = calloc();
  final ffi.Pointer<Runner> runner = calloc();
  final ffi.Pointer<ffi.Uint8> pixels = calloc<ffi.Uint8>(bufferSize);

  final audioStream = getAudioStream();

  /// Keep the computed screen scale after a resize event
  double _screenScale = 1.0;
  double get screenScale => _screenScale;

  audioInit() {
    audioStream.init(channels: 2);
  }

  initState() {
    runnerInit(runner);
    audioInit();
  }

  @override
  dispose() {
    super.dispose();
    runnerDeinit(runner);
  }

  Error compileAndStart(String src) {
    final CoreError err = runnerCompileProgram(runner, src);
    if (err.code == 0) {
      // TODO: compute secondsSincePowerOn
      runnerStart(runner, 123);
    }
    return Error(
        code: err.code,
        msg: runnerGetError(runner, err.code),
        line: err.sourcePosition);
  }

  resize(double inWidth, double inHeight, double safeTop, double safeLeft,
      double safeBottom, safeRight) {
    double ratio = inWidth / inHeight;
    _screenScale;

    if (ratio > 9.0 / 16.0) {
      _screenScale = inWidth / screenWidth.toDouble();
    } else {
      _screenScale = inHeight / screenHeight.toDouble();
    }

    input.ref.width = (inWidth / _screenScale).toInt();
    input.ref.height = (inHeight / _screenScale).toInt();
    input.ref.top = (safeTop / _screenScale).toInt();
    input.ref.left = (safeLeft / _screenScale).toInt();
    input.ref.bottom = (safeBottom / _screenScale).toInt();
    input.ref.right = (safeRight / _screenScale).toInt();
  }

  // TODO: Should I update in compute?
  update() {
    runnerUpdate(runner, input);
  }

  // TODO: Should I render in compute?
  render() {
    runnerRender(runner, pixels);
    bytesList = pixels.asTypedList(bufferSize);
    ui.decodeImageFromPixels(
        bytesList!, screenWidth, screenHeight, ui.PixelFormat.rgba8888,
        (image) {
      this.image = image;
      notifyListeners();
    });
  }

  renderThumbnail(String programName) {
    img.Image image = img.copyCrop(
        img.Image.fromBytes(
            width: screenWidth,
            height: screenHeight,
            bytes: bytesList!.buffer,
            numChannels: 4,
            rowStride: screenWidth * bytePerPixel),
        x: 0,
        y: 0,
        width: thumbWidth,
        height: thumbHeight,
        antialias: false);
		MyLibrary.writeThumbnail(programName, image);
  }

  touchOn(Offset pos) {
    input.ref.touchX = (pos.dx / _screenScale);
    input.ref.touchY = (pos.dy / _screenScale);
    input.ref.touch = true;
  }

  touchOff() {
    input.ref.touch = false;
  }
}
