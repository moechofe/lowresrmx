// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error
// TODO: TEST to reach the end of a program, how it is handled compare to produce an error

import 'dart:async';
import 'dart:developer';
import 'dart:ffi' as ffi;
import 'dart:isolate';
import 'dart:ui' as ui;
import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:ffi/ffi.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:image/image.dart' as img;
// import 'package:flutter_soloud/flutter_soloud.dart';
import 'package:flutter_miniaudio/flutter_miniaudio.dart';

import 'package:core_plugin/core_plugin_bindings_generated.dart';
import 'package:core_plugin/core_plugin.dart';

import 'package:lowresrmx/data/outline_entry.dart';

class Location {
  late final int row;
  late final int column;
  Location(this.row, this.column);

  Location.fromCode(String code, int offset) {
    int index = 0;
    int line = 0;
    while (index < code.length) {
      int newline = code.indexOf("\n", index);
      if (newline == -1) {
        break;
      } else if (newline >= offset) {
        row = line;
        column = offset - index;
        return;
      }
      index = newline + 1;
      ++line;
    }
    row = line;
    column = offset - index;
  }
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
    if (ok || position == -1) {
      return Location(-1, -1);
    } else {
      return Location.fromCode(code, position);
    }
  }

  Map<String, dynamic> toMap() {
    return {
      "code": code,
      "msg": msg,
      "position": position,
    };
  }

  static Error? fromMap(Map<String, dynamic>? map) {
    if (map == null) {
      return null;
    }
    return Error(code: map["code"], msg: map["msg"], position: map["position"]);
  }
}

/// Used to transport some type of message from the app to the isolate
enum IsolateMessageType {
  touchOff,
  traceOn,
  traceOff,
  thumbnail,
  renderFrame,
	audioStart,
	audioStop,
	renderAudio,
  notifyFrame,
}

// FIXME: Message should have Message in their name. Common!

/// Message used to transport the code and the data disk from the app to the isolate
class CompileAndRunMsg {
  final String code;
  final String dataDisk;
  CompileAndRunMsg(this.code, this.dataDisk);
}

/// Message used to transport the code from the app to the isolate
class CompileOnlyMsg {
  final String code;
  CompileOnlyMsg(this.code);
}

/// Message used to transport the error from the isolate to the app
class RunningErrorMsg {
  final Error error;
  RunningErrorMsg(this.error);
}

/// Message used to transport the thumbnail from the isolate to the app
class ThumbnailMsg {
  static int thumbWidth = 180;
  static int thumbHeight = 180;

  Uint8List pixels;
  ThumbnailMsg(this.pixels);
}

/// Message used to transport the data disk from the isolate to the app
class DataDiskMsg {
  final String dataDisk;
  DataDiskMsg(this.dataDisk);
}

/// Message used to transport the keyboard visibility from the isolate to the app
class KeyboardVisibleMsg {
  final bool open;
  KeyboardVisibleMsg(this.open);
}

class InputModeMsg {
  final bool enable;
  InputModeMsg(this.enable);
}

/// Message used to transport the keyboard key down event from the app to the isolate
class KeyboardKeyDownMsg {
  final int ascii;
  KeyboardKeyDownMsg(this.ascii);
}

class OrientationChangeMsg {
  final double width;
  final double height;
  final double safeTop;
  final double safeLeft;
  final double safeBottom;
  final double safeRight;
  OrientationChangeMsg(this.width, this.height, this.safeTop, this.safeLeft,
      this.safeBottom, this.safeRight);
}

class MeasurementMsg {
  final double updateTime;
  final double renderTime;
  MeasurementMsg(this.updateTime, this.renderTime);
}

/// Bridge between the core and the app
class Runtime extends ChangeNotifier {

	final audio = MiniaudioPlayer(
  	sampleRate: 44100,
  	channels: 2,
  	bufferFrames: 1470,
	);
	final ffi.Pointer<ffi.Int16> audioBuffer = calloc<ffi.Int16>(1470 * 2);

  static int screenWidth = 216;
  static int screenHeight = 384;
  static int bytePerPixel = 4;
  static int bufferSize = screenWidth * screenHeight * bytePerPixel;
  Uint8List? bytesList;
  ui.Image? image;
  String? dataDiskToSave;
  bool keyboardOpen = false;
	bool inputMode = false;
  int? textureId;

  final ffi.Pointer<Input> input = calloc();
  final ffi.Pointer<Runner> runner = calloc();
  final ffi.Pointer<ffi.Uint8> pixels = calloc<ffi.Uint8>(bufferSize);
  final ffi.Pointer<CoreDelegate> delegate = calloc();

  /// Keep the computed screen scale after a resize event
  double _screenScale = 1.0;
  double get screenScale => _screenScale;

  void initState() async {
    runnerInit(runner);
  }

  @override
  void dispose() {
		audio.dispose();
    runnerDeinit(runner);
    super.dispose();
  }

  Future<Error> compileAndStart(String src, String dataDisk) async {
    final CoreError err = runnerCompileProgram(runner, src);
    if (err.code == 0) {
      final Uint8List dataList = Uint8List.fromList(dataDisk.codeUnits);
      final int dataSize = dataList.length;
      final ffi.Pointer<ffi.Uint8> dataDiskPtr = calloc<ffi.Uint8>(dataSize);
      dataDiskPtr.asTypedList(dataDisk.length).setAll(0, dataList);
      // TODO: compute secondsSincePowerOn
      runnerStart(runner, 123, ffi.Pointer.fromAddress(dataDiskPtr.address), dataSize);
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

  void keyDown(int ascii) {
    inputKeyDown(input, ascii);
  }

  Error update() {
    final CoreError err = runnerUpdate(runner, input);
    if (runner.ref.shouldSaveDisk) {
      dataDiskToSave = runner.ref.dataDisk
          .cast<Utf8>()
          .toDartString(length: runner.ref.dataDiskSize);
      runner.ref.shouldSaveDisk = false;
    }
    keyboardOpen = runner.ref.shouldOpenKeyboard;
    inputMode = runner.ref.shouldEnableInputMode;
    return Error(
        code: err.code,
        msg: runnerGetError(runner, err.code),
        position: err.sourcePosition);
  }

  void renderFrame() {
		// Faster, use texture uploaded to the GPU
    if (textureId != null) {
      runnerRenderToTexture(runner, textureId!);
		// Slower, recreate an image using pixel buffer
    } else {
    	runnerRender(runner, pixels);
    	bytesList = pixels.asTypedList(bufferSize);
    }
  }

	void renderAudio() {
		runnerRenderAudio(runner, audioBuffer, 1470 * 2, 44100, 0);
		audio.write(audioBuffer, 1470);
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

  List<OutlineEntry> getOutline() {
    int count = runnerGetSymbolCount(runner);
    List<OutlineEntry> list = [];
    for (int i = 0; i < count; ++i) {
      String name = runnerGetSymbolName(runner, i);
      int position = runnerGetSymbolPosition(runner, i);
      list.add(OutlineEntry(name, position));
    }
    return list;
  }
}

/// Used to hold the [Runtime] instance into an isolate.
void isolateEntryPoint(List<Object?> arguments) {
  final RootIsolateToken rootIsolateToken = arguments[0] as RootIsolateToken;
  final SendPort sendPort = arguments[1] as SendPort;
  final int textureId = arguments[2] as int;
  final int? nativeHandle = arguments[3] as int?;

  BackgroundIsolateBinaryMessenger.ensureInitialized(rootIsolateToken);

  final ReceivePort receivePort = ReceivePort();
  final Runtime runtime = Runtime();

  runtime.textureId = textureId;

  if (nativeHandle != null) {
    runnerRegisterNativeTexture(
      textureId, ffi.Pointer.fromAddress(nativeHandle));
  }

  double updateTime, renderTime;

  try {
    runtime.initState();
  } catch (e) {
    log("Error during runtime.initState: $e");
  }

  // Remember the keyboard state to avoid sending the same message each frame.
  bool currKeyboardOpen = false;
  bool currInputMode = false;

  sendPort.send(receivePort.sendPort);

  receivePort.listen((message) async {
    try {
      if (message is CompileAndRunMsg) {
        // Receive the code and compile it, then send back the error. Start running if no error
        final Error err =
            await runtime.compileAndStart(message.code, message.dataDisk);
        sendPort.send(err);
      } else if (message is CompileOnlyMsg) {
        // Receive the code and compile it, then send back the error
        final Error err = runtime.compileOnly(message.code);
        sendPort.send(err);
        // Also send the list of outline entries
        final List<OutlineEntry> outline = runtime.getOutline();
        sendPort.send(outline);
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.renderFrame) {
        // Render the frame
        var stopwatch = Stopwatch()..start();
        Error err = runtime.update();
        updateTime =
            stopwatch.elapsed.inMicroseconds / Duration.microsecondsPerSecond;
        stopwatch.reset();
        runtime.renderFrame();
        renderTime =
            stopwatch.elapsed.inMicroseconds / Duration.microsecondsPerSecond;
        sendPort.send(MeasurementMsg(updateTime, renderTime));
        if (runtime.textureId != null) {
          sendPort.send(IsolateMessageType.notifyFrame);
        } else {
          sendPort.send(runtime.bytesList!);
        }
        if (!err.ok) {
          sendPort.send(RunningErrorMsg(err));
        }
        if (runtime.dataDiskToSave != null) {
          sendPort.send(DataDiskMsg(runtime.dataDiskToSave!));
          runtime.dataDiskToSave = null;
        }
        if (runtime.keyboardOpen != currKeyboardOpen) {
          sendPort.send(KeyboardVisibleMsg(runtime.keyboardOpen));
          currKeyboardOpen = runtime.keyboardOpen;
        }
        if (runtime.inputMode != currInputMode) {
          sendPort.send(InputModeMsg(runtime.inputMode));
          currInputMode = runtime.inputMode;
        }
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.audioStart) {
        runtime.audio.start();
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.renderAudio) {
        runtime.renderAudio();
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.audioStop) {
        runtime.audio.stop();
      } else if (message is OrientationChangeMsg) {
        // Receive the screen size and the safe area
        runtime.resize(message.width, message.height, message.safeTop,
            message.safeLeft, message.safeBottom, message.safeRight);
      } else if (message is KeyboardKeyDownMsg) {
        runtime.keyDown(message.ascii);
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
        runtime.renderFrame();
        sendPort.send(ThumbnailMsg(runtime.bytesList!));
      }
    } catch (e, stack) {
      log("Isolate error: $e\n$stack");
      sendPort.send(RunningErrorMsg(
          Error(code: -1, msg: "Isolate crashed: $e", position: -1)));
    }
  });
}

/// To receive the [ui.Image] to be paint on [CustomPainter].
typedef FrameCallback = void Function(ui.Image);

/// To receive the [img.Image] to be saved as thumbnail.
typedef ThumbnailCallback = void Function(img.Image);

/// To receive the error when the program is running.
typedef RunnerErrorCallback = void Function(Error);

/// To received the data disk to be saved on the device.
typedef SaveDataDiskCallback = void Function(String);

/// To receive the keyboard visibility.
typedef KeyboardVisibleCallback = void Function(bool);

/// To receive the input mode changes.
typedef InputModeCallback = void Function(bool);

/// To receive the outline entries.
typedef OutlineCallback = void Function(List<OutlineEntry>);

/// Used to group all communication with the isolate in one place
class ComPort {
  late final Isolate isolate;
  late final ReceivePort receivePort;
  late final SendPort sendPort;
	/// Regulary render the frame.
  late final Ticker ticker;
  Duration prevDuration = Duration.zero;
  Stopwatch runtimeStopwatch = Stopwatch();
  int prevRuntimeElapsed = 0;
  final Completer<SendPort> ready = Completer();
  Completer<Error>? compileCompleter;
  FrameCallback? onImage;
  ThumbnailCallback? onThumbnail;
  RunnerErrorCallback? onRunningError;
  SaveDataDiskCallback? onSaveDataDisk;
  KeyboardVisibleCallback? onKeyboardVisible;
  InputModeCallback? onInputMode;
  OutlineCallback? onOutline;

  StreamController<double> deltaTime = StreamController<double>();
  StreamController<double> updateTime = StreamController<double>();
  StreamController<double> renderTime = StreamController<double>();
  StreamController<double> decodeTime = StreamController<double>();
  StreamController<double> runtimeDeltaTime = StreamController<double>();

  int? textureId;

  /// Setup the communication with the isolate and listen for messages
  Future<SendPort> init() async {
    final int id = await registerTexture();
    textureId = id;
    int? handle;
    if (Platform.isIOS) {
      handle = await const MethodChannel('com.lowresrmx/core_plugin')
          .invokeMethod<int>('getBufferAddress', id);
    }

    receivePort = ReceivePort();
    isolate = await Isolate.spawn(isolateEntryPoint,
        [RootIsolateToken.instance!, receivePort.sendPort, id, handle]);

    receivePort.listen((message) {
      if (message is SendPort) {
        // Store the sendPort to be used later
        sendPort = message;
        ready.complete(sendPort);
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.notifyFrame) {
        // Notify the UI that a new frame is available for the texture
        const MethodChannel('com.lowresrmx/core_plugin')
            .invokeMethod('notifyFrameAvailable', textureId);
        // We still trigger a frame callback if needed for UI rebuilds
        if (onImage != null) {
          // Note: In texture mode, we don't have a ui.Image object here.
          // The UI should use the Texture widget.
        }
      } else if (message is Error) {
        // Receive the error from the compilation
        compileCompleter!.complete(message);
      } else if (message is RunningErrorMsg) {
        // Receive the error when program is running
        if (onRunningError != null) {
          onRunningError!(message.error);
        }
      } else if (message is Uint8List && onImage != null) {
        final int delta =
            runtimeStopwatch.elapsedMicroseconds - prevRuntimeElapsed;
        runtimeDeltaTime.add(delta / Duration.microsecondsPerSecond);
        prevRuntimeElapsed = runtimeStopwatch.elapsedMicroseconds;
        var stopwatch = Stopwatch()..start();
        // Decode the image and call the callback
        ui.decodeImageFromPixels(message, Runtime.screenWidth,
            Runtime.screenHeight, ui.PixelFormat.rgba8888, onImage!);
        decodeTime.add(
            stopwatch.elapsed.inMicroseconds / Duration.microsecondsPerSecond);
      } else if (message is ThumbnailMsg) {
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
            width: ThumbnailMsg.thumbWidth,
            height: ThumbnailMsg.thumbHeight,
            antialias: false);
        onThumbnail!(image);
      } else if (message is DataDiskMsg) {
        if (onSaveDataDisk != null) {
          onSaveDataDisk!(message.dataDisk);
        }
      } else if (message is KeyboardVisibleMsg) {
        if (onKeyboardVisible != null) {
          onKeyboardVisible!(message.open);
        }
      } else if (message is InputModeMsg) {
        if (onInputMode != null) {
          onInputMode!(message.enable);
        }
      } else if (message is List<OutlineEntry>) {
        // Receive the outline entries
        if (onOutline != null) {
          onOutline!(message);
        }
      } else if (message is MeasurementMsg) {
        updateTime.add(message.updateTime);
        renderTime.add(message.renderTime);
      }
    });

		bool tickerByTwo = true;

    ticker = Ticker((Duration currDuration) {
      final delta = currDuration - prevDuration;
      deltaTime.add(delta.inMicroseconds / Duration.microsecondsPerSecond);
      prevDuration = currDuration;

      // NOTE: If started before the SendPort is ready, it will crash.
      sendPort.send(IsolateMessageType.renderFrame);

			if (tickerByTwo) {
				sendPort.send(IsolateMessageType.renderAudio);
			}

			tickerByTwo = !tickerByTwo;
    });

    return ready.future;
  }

  /// Compile and run the code in the isolate
  Future<Error> compileAndRun(String code, String dataDisk) async {
    compileCompleter = Completer();
    sendPort.send(CompileAndRunMsg(code, dataDisk));
    return compileCompleter!.future;
  }

  /// Used to compile and report error in the editor
  Future<Error> compileOnly(String code) async {
    compileCompleter = Completer();
    sendPort.send(CompileOnlyMsg(code));
    return compileCompleter!.future;
  }

  /// Start updating the runtime at 60 fps
  void start() {
		log("Ticker started");
		sendPort.send(IsolateMessageType.audioStart);
    ticker.start();
    runtimeStopwatch.start();
    // sendPort.send(true);
  }

  /// Stop updating the runtime
  void stop() {
		log("Ticker stopped");
    ticker.stop();
		sendPort.send(IsolateMessageType.audioStop);
    prevDuration = Duration.zero;
    runtimeStopwatch.stop();
    runtimeStopwatch.reset();
    prevRuntimeElapsed = 0;
  }

  /// Update the device screen size and the safe area
  void resize(double inWidth, double inHeight, double safeTop, double safeLeft,
      double safeBottom, double safeRight) {
    sendPort.send(OrientationChangeMsg(
        inWidth, inHeight, safeTop, safeLeft, safeBottom, safeRight));
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

  void keyDown(int ascii) {
    sendPort.send(KeyboardKeyDownMsg(ascii));
  }
}
