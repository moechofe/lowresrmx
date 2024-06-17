import 'dart:async';
import 'dart:developer';
import 'dart:ffi' as ffi;
import 'dart:isolate';
import 'dart:ui' as ui;

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:ffi/ffi.dart' show calloc;
import 'package:image/image.dart' as img;
import 'package:mp_audio_stream/mp_audio_stream.dart';

import 'package:core_plugin/core_plugin_bindings_generated.dart';
import 'package:core_plugin/core_plugin.dart';

import 'package:lowresrmx/data/library.dart';

class Location {
  final int index;
  final int offset;
  Location(this.index, this.offset);
}

/// Used to transport errors from the runtime to the app
class Error {
  int code;
  String msg;
  int position;

  Error({required this.code, required this.msg, required this.position});

  bool get ok => code == 0;

  @override
  String toString() => "Error #$code at pos: $position: $msg";

  Location getLocation(String code) {
    if (ok || this.position == -1) {
      return Location(-1, -1);
    }
    int position = this.position;
    int index = 0;
    int line = 0;
    while (index < code.length) {
      int newline = code.indexOf("\n", index);
      if (newline == -1) {
        break;
      } else if (newline >= position) {
        return Location(line, position - index);
      }
      index = newline + 1;
      ++line;
    }
    return Location(line, position);
  }
}

/// Used to transport some type of message from the app to the isolate
enum IsolateMessageType {
  touchOff,
  traceOn,
  traceOff,
  thumbnail,
}

/// Message used to transport the code from the app to the isolate
class CompileOnlyMessage {
  final String code;
  CompileOnlyMessage(this.code);
}

/// Message used to transport the error from the isolate to the app
class RunningError {
  final Error error;
  RunningError(this.error);
}

/// Message used to transport the thumbnail from the isolate to the app
class Thumbnail {
  static int thumbWidth = 180;
  static int thumbHeight = 180;

  Uint8List pixels;
  Thumbnail(this.pixels);
}

/// Bridge between the core and the app
class Runtime extends ChangeNotifier {
  static int screenWidth = 216;
  static int screenHeight = 384;
  static int bytePerPixel = 4;
  static int bufferSize = screenWidth * screenHeight * bytePerPixel;
  Uint8List? bytesList;
  ui.Image? image;

  final ffi.Pointer<Input> input = calloc();
  final ffi.Pointer<Runner> runner = calloc();
  final ffi.Pointer<ffi.Uint8> pixels = calloc<ffi.Uint8>(bufferSize);
  final ffi.Pointer<CoreDelegate> delegate = calloc();

  final audioStream = getAudioStream();

  /// Keep the computed screen scale after a resize event
  double _screenScale = 1.0;
  double get screenScale => _screenScale;

  // static void didFail(ffi.Pointer<ffi.Void> runner, CoreError error) {
  //   log("Runtime.didFail: $error");
  //   // 	final Runtime runtime = Runtime();
  //   // 	runtime.interpreterDidFail(CoreError.fromPointer(error));
  // }

  // static bool driveWillAccess(
  //     ffi.Pointer<ffi.Void> runner, ffi.Pointer<DataManager> dataManager) {
  //   log("driveWillAccess");
  //   // TODO: Implement
  //   return true;
  // }

  // static void driveDidSave(
  //     ffi.Pointer<ffi.Void> runner, ffi.Pointer<DataManager> dataManager) {
  //   log("driveDidSave");
  // }

  // static void driveIsFull(
  //     ffi.Pointer<ffi.Void> runner, ffi.Pointer<DataManager> dataManager) {
  //   log("driveIsFull");
  // }

  // static void controlsDidChange(
  //     ffi.Pointer<ffi.Void> runner, ControlsInfo controls) {
  //   log("controlsDidChange");
  // }

  // static void persistentRamWillAccess(ffi.Pointer<ffi.Void> runner,
  //     ffi.Pointer<ffi.Uint8> destination, int size) {
  //   log("persistentRamWillAccess");
  // }

  // static void persistentRamDidChange(
  //     ffi.Pointer<ffi.Void> runner, ffi.Pointer<ffi.Uint8> source, int size) {
  //   log("persistentRamDidChange");
  // }

  // void delegateInit() {
  //   delegate.ref.context = ffi.Pointer.fromAddress(runner.address);
  //   delegate.ref.interpreterDidFail =
  //       ffi.Pointer.fromFunction<InterpreterDidFailFunc>(didFail);
  //   delegate.ref.diskDriveWillAccess =
  //       ffi.Pointer.fromFunction<InterpreterDriveWillAccessFunc>(
  //           driveWillAccess, false);
  //   delegate.ref.diskDriveDidSave =
  //       ffi.Pointer.fromFunction<InterpreterDriveDidSaveFunc>(driveDidSave);
  //   delegate.ref.diskDriveIsFull =
  //       ffi.Pointer.fromFunction<InterpreterDriveDidSaveFunc>(driveIsFull);
  //   delegate.ref.controlsDidChange =
  //       ffi.Pointer.fromFunction<InterpreterControlsDidChangeFunc>(
  //           controlsDidChange);
  //   delegate.ref.persistentRamWillAccess =
  //       ffi.Pointer.fromFunction<InterpreterPersistentRamWillAccessFunc>(
  //           persistentRamWillAccess);
  //   delegate.ref.persistentRamDidChange =
  //       ffi.Pointer.fromFunction<InterpreterPersistentRamDidChangeFunc>(
  //           persistentRamDidChange);
  //   runnerSetDelegate(runner, ffi.Pointer.fromAddress(delegate.address));
  // }

  void audioInit() {
    audioStream.init(channels: 2);
  }

  void initState() {
    runnerInit(runner);
    // delegateInit();
    audioInit();
  }

  @override
  void dispose() {
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
        position: err.sourcePosition);
  }

  Error compileOnly(String src) {
    final CoreError err = runnerCompileProgram(runner, src);
    return Error(
        code: err.code,
        msg: runnerGetError(runner, err.code),
        position: err.sourcePosition);
  }

  void resize(double inWidth, double inHeight, double safeTop, double safeLeft,
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
  Error update() {
    final CoreError err = runnerUpdate(runner, input);
    return Error(
        code: err.code,
        msg: runnerGetError(runner, err.code),
        position: err.sourcePosition);
  }

  // TODO: Should I render in compute?
  void render() {
    runnerRender(runner, pixels);
    bytesList = pixels.asTypedList(bufferSize);
    // for (int i = 0; i < bufferSize; i += 4) {
    //   int r = bytesList![i];
    //   int g = bytesList![i + 1];
    //   int b = bytesList![i + 2];
    //   int a = bytesList![i + 3];
    //   if (r > 0 || g > 0 || b > 0) {
    //     // log("r: $r, g: $g, b: $b, a: $a");
    //   }
    //   bytesList![i + 3] = 255;
    // }
  }

  void touchOn(Offset pos) {
    input.ref.touchX = (pos.dx / _screenScale);
    input.ref.touchY = (pos.dy / _screenScale);
    input.ref.touch = true;
  }

  void touchOff() {
    input.ref.touch = false;
  }

  void trace(bool trace) {
    runnerTrace(runner, trace);
  }

  void interpreterDidFail(CoreError error) {
    log("Interpreter failed: $error");
  }
}

/// Used to hold the [Runtime] instance into an isolate.
void isolateEntryPoint(SendPort sendPort) {
  final ReceivePort receivePort = ReceivePort();
  final Runtime runtime = Runtime();

  runtime.initState();

  bool running = false;

  sendPort.send(receivePort.sendPort);

  receivePort.listen((message) {
    if (message is String) {
      // Receive the code and compile it, then send back the error. Start running if no error
      final Error err = runtime.compileAndStart(message);
      sendPort.send(err);
    } else if (message is CompileOnlyMessage) {
      // Receive the code and compile it, then send back the error
      final Error err = runtime.compileOnly(message.code);
      sendPort.send(err);
    } else if (message is bool && message) {
      // Start the running at 60 fps
      log("isolate: Start running");
      running = true;
    } else if (message is bool && !message) {
      // Stop the running
      log("isolate: Stop running");
      running = false;
    } else if (message is List<double>) {
      // Receive the screen size and the safe area
      runtime.resize(message[0], message[1], message[2], message[3], message[4],
          message[5]);
    } else if (message is Offset) {
      // Receive the touch event
      runtime.touchOn(message);
    } else if (message is IsolateMessageType &&
        message == IsolateMessageType.touchOff) {
      // Receive the touch off event
      runtime.touchOff();
    } else if (message is IsolateMessageType &&
        message == IsolateMessageType.traceOn) {
      // Receive the trace on event
      runtime.trace(true);
    } else if (message is IsolateMessageType &&
        message == IsolateMessageType.traceOff) {
      // Receive the trace off event
      runtime.trace(false);
    } else if (message is IsolateMessageType &&
        message == IsolateMessageType.thumbnail) {
      runtime.render();
      sendPort.send(Thumbnail(runtime.bytesList!));
    }
  });

  // Start a 60 fps timer to update and render the runtime
  Timer.periodic(Duration(milliseconds: (1000 / 60).round()), (timer) {
    if (running) {
      Error err = runtime.update();
      runtime.render();
      sendPort.send(runtime.bytesList!);
      if (!err.ok) {
        sendPort.send(RunningError(err));
      }
    }
  });
}

/// To receive the [ui.Image] to be paint on [CustomPainter].
typedef FrameCallback = void Function(ui.Image);

/// To receive the [img.Image] to be saved as thumbnail.
typedef ThumbnailCallback = void Function(img.Image);

/// To receive the error when the program is running.
typedef RunnerErrorCallback = void Function(Error);

/// Used to group all communication with the isolate in one place
class ComPort {
  late final Isolate isolate;
  late final ReceivePort receivePort;
  late final SendPort sendPort;
  final Completer<SendPort> ready = Completer();
  Completer<Error>? compileCompleter;
  FrameCallback? onImage;
  ThumbnailCallback? onThumbnail;
  RunnerErrorCallback? onRunningError;

  /// Setup the communication with the isolate and listen for messages
  Future<SendPort> init() async {
    receivePort = ReceivePort();
    isolate = await Isolate.spawn(isolateEntryPoint, receivePort.sendPort);
    receivePort.listen((message) {
      if (message is SendPort) {
        // Store the sendPort to be used later
        sendPort = message;
        ready.complete(sendPort);
      } else if (message is Error) {
        // Receive the error from the compilation
        compileCompleter!.complete(message);
      } else if (message is RunningError) {
        // Receive the error when program is running
        if (onRunningError != null) {
          onRunningError!(message.error);
        }
      } else if (message is Uint8List && onImage != null) {
        // Decode the image and call the callback
        ui.decodeImageFromPixels(message, Runtime.screenWidth,
            Runtime.screenHeight, ui.PixelFormat.rgba8888, onImage!);
      } else if (message is Thumbnail) {
        // Receive the thumbnail
        img.Image image = img.copyCrop(
            img.Image.fromBytes(
                width: Runtime.screenWidth,
                height: Runtime.screenHeight,
                bytes: message.pixels.buffer,
                numChannels: 4,
                rowStride: Runtime.screenWidth * Runtime.bytePerPixel),
            x: 0,
            y: 0,
            width: Thumbnail.thumbWidth,
            height: Thumbnail.thumbHeight,
            antialias: false);
        onThumbnail!(image);
        // MyLibrary.writeThumbnail(programName, image);
      }
    });
    return ready.future;
  }

  /// Compile and run the code in the isolate
  Future<Error> compileAndRun(String code) async {
    compileCompleter = Completer();
    sendPort.send(code);
    return compileCompleter!.future;
  }

  /// Used to compile and report error in the editor
  Future<Error> compileOnly(String code) async {
    compileCompleter = Completer();
    sendPort.send(CompileOnlyMessage(code));
    return compileCompleter!.future;
  }

  /// Start updating the runtime at 60 fps
  void start() => sendPort.send(true);

  /// Stop updating the runtime
  void stop() => sendPort.send(false);

  /// Update the device screen size and the safe area
  void resize(double inWidth, double inHeight, double safeTop, double safeLeft,
      double safeBottom, double safeRight) {
    sendPort
        .send([inWidth, inHeight, safeTop, safeLeft, safeBottom, safeRight]);
  }

  /// Send the touch event to the runtime
  void touchOn(Offset pos) => sendPort.send(pos);

  /// Send the touch off event to the runtime
  void touchOff() => sendPort.send(IsolateMessageType.touchOff);

  /// Send the trace event to the runtime
  void trace(bool trace) => trace
      ? sendPort.send(IsolateMessageType.traceOn)
      : sendPort.send(IsolateMessageType.traceOff);

	void thumbnail() => sendPort.send(IsolateMessageType.thumbnail);
}
