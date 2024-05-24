import 'dart:async';
import 'dart:developer' show log;
import 'dart:ffi' as ffi;
import 'dart:typed_data' show Uint8List;
import 'dart:ui' as ui;

import 'package:ffi/ffi.dart' show calloc;
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:sw_rend/software_texture.dart';

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
  static int imageWidth = 216;
  static int imageHeight = 384;
  static int bufferSize = imageWidth * imageHeight * 32;
  ui.Image? image;

  final ffi.Pointer<Input> input = calloc();
  final ffi.Pointer<Runner> runner = calloc();
  final ffi.Pointer<ffi.Uint8> pixels = calloc<ffi.Uint8>(bufferSize);

  initState() {
    runnerInit(runner);

    // TEMPORARY
    input.ref.width = 216;
    input.ref.height = 384;
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

	resize(double inWidth, double inHeight, double safeTop, double safeLeft, double safeBottom, safeRight)
	{
		double ratio = inWidth / inHeight;
		double scale;

		if (ratio > 9.0/16.0) {
			scale = inWidth/imageWidth.toDouble();
		} else {
			scale = inHeight/imageHeight.toDouble();
		}

		input.ref.width = (inWidth / scale).toInt();
		input.ref.height = (inHeight / scale).toInt();
		input.ref.top = (safeTop / scale).toInt();
		input.ref.left = (safeLeft / scale).toInt();
		input.ref.bottom = (safeBottom / scale).toInt();
		input.ref.right = (safeRight / scale).toInt();
	}

  update() {
    runnerUpdate(runner, input);
  }

  render() {
    runnerRender(runner, pixels);
    Uint8List src = pixels.asTypedList(bufferSize);
    ui.decodeImageFromPixels(src, imageWidth, imageHeight, ui.PixelFormat.rgba8888,
        (image) {
					this.image = image;
      notifyListeners();
    });
  }
}
