import 'dart:async';
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

/// Message used to ask the isolate to advance the engine, then render one frame at 1/60s.
class RenderFrameMsg {
  final int steps;
  RenderFrameMsg(this.steps);
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

  /// True when the program's `ON THUMBNAIL` handler drew the icon.
  final bool programEnded;
  ThumbnailMsg(this.pixels, this.programEnded);
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

  final double keyboardHeight;

  OrientationChangeMsg(this.width, this.height, this.safeTop, this.safeLeft,
      this.safeBottom, this.safeRight, this.keyboardHeight);
}

/// Message used to hand the render isolate a new destination surface.
class SurfaceChangeMsg {
  const SurfaceChangeMsg(
      this.address, this.bytesPerRow, this.width, this.height);

  /// 0 keeps the native handle the C side already has (Android).
  final int address;

  /// 0 means width*4.
  final int bytesPerRow;

  final int width;
  final int height;
}

class MeasurementMsg {
  final double updateTime;
  final double renderTime;
  MeasurementMsg(this.updateTime, this.renderTime);
}

/// Bridge between the core and the app
class Runtime extends ChangeNotifier {
  static const int _audioSampleRate = 44100;
  static const int _audioChannels = 2;
  static const int _audioBufferFrames = 1470;

  MiniaudioPlayer? _audio;

  final ffi.Pointer<ffi.Int16> audioBuffer =
      calloc<ffi.Int16>(_audioBufferFrames * _audioChannels);

  static int screenWidth = 216;
  static int screenHeight = 384;
  static int bytePerPixel = 4;
  static int bufferSize = screenWidth * screenHeight * bytePerPixel;

  /// Byte order the engine emits, mirroring the ABGR define in the plugin build:
  /// core_plugin/src/CMakeLists.txt sets ABGR=1 (R,G,B,A) for Android and desktop,
  /// core_plugin/ios/core_plugin.podspec sets ABGR=0 (B,G,R,A) for iOS.
  static final bool pixelsAreRgba = !Platform.isIOS;

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

  int _keyboardHeight = 0;

  void initState() async {
    runnerInit(runner);
  }

  @override
  void dispose() {
    audioStop();
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
      runnerStart(runner, input, 123,
          ffi.Pointer.fromAddress(dataDiskPtr.address), dataSize);
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
      double safeBottom, double safeRight, double keyboardHeight) {
    double ratio = inWidth / inHeight;

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

    final int occluded = (keyboardHeight / _screenScale).ceil();
    if (occluded != _keyboardHeight) {
      _keyboardHeight = occluded;
      input.ref.keyboardHeight = occluded;
      input.ref.keyboardChange = occluded > 0 ? 1 : -1;
    }
  }

  void keyDown(int ascii) {
    inputKeyDown(input, ascii);
  }

  Error update() {
    final CoreError err = runnerUpdate(runner, input);
    input.ref.keyboardChange = 0;
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

  /// Points the engine at a destination surface and its device-pixel geometry. Called only from
  /// the render isolate, so neither the handle nor the geometry can change under a blit.
  void setSurface(int address, int bytesPerRow, int width, int height) {
    if (textureId == null) return;
    if (address != 0) {
      runnerRegisterNativeTexture(textureId!, ffi.Pointer.fromAddress(address));
    }
    runnerSetTextureGeometry(textureId!, width, height, bytesPerRow);
  }

  /// Renders one frame into the CPU pixel buffer at fantasy resolution, whatever the texture
  /// mode is. Thumbnails need these pixels, not the device-resolution surface.
  void renderPixels() {
    runnerRender(runner, pixels);
    bytesList = pixels.asTypedList(bufferSize);
  }

  void renderFrame() {
    // Faster, the engine upscales straight into the device-resolution surface
    if (textureId != null) {
      runnerRenderToTexture(runner, textureId!);
      // Slower, recreate an image using pixel buffer
    } else {
      renderPixels();
    }
  }

  void audioStart() {
    if (_audio != null) {
      return;
    }
    MiniaudioPlayer? player;
    try {
      player = MiniaudioPlayer(
        sampleRate: _audioSampleRate,
        channels: _audioChannels,
        bufferFrames: _audioBufferFrames,
      );
      player.start();
      _audio = player;
    } catch (e) {
      debugPrint("Audio device unavailable, running without sound: $e");
      try {
        player?.dispose();
      } catch (_) {}
    }
  }

  void audioStop() {
    final MiniaudioPlayer? player = _audio;
    _audio = null;
    if (player == null) {
      return;
    }
    try {
      player.dispose();
    } catch (e) {
      debugPrint("Failed to close the audio device: $e");
    }
  }

  void renderAudio() {
    final MiniaudioPlayer? player = _audio;
    if (player == null) {
      return;
    }
    runnerRenderAudio(runner, audioBuffer, _audioBufferFrames * _audioChannels,
        _audioSampleRate, 0);
    player.write(audioBuffer, _audioBufferFrames);
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

  bool thumbnailPending = false;

  bool startThumbnail() {
    if (!runnerHasThumbnailHandler(runner)) return false;
    if (!runnerStartThumbnail(runner)) return false;
    thumbnailPending = true;
    return true;
  }

  bool get thumbnailReady => runnerIsThumbnailReady(runner);

  void endThumbnail() {
    thumbnailPending = false;
    runnerEndThumbnail(runner);
  }
}

/// Used to hold the [Runtime] instance into an isolate.
void isolateEntryPoint(List<Object?> arguments) {
  final RootIsolateToken rootIsolateToken = arguments[0] as RootIsolateToken;
  final SendPort sendPort = arguments[1] as SendPort;
  final int textureId = arguments[2] as int;
  final int address = arguments[3] as int;
  final int bytesPerRow = arguments[4] as int;
  final int surfaceWidth = arguments[5] as int;
  final int surfaceHeight = arguments[6] as int;

  BackgroundIsolateBinaryMessenger.ensureInitialized(rootIsolateToken);

  final ReceivePort receivePort = ReceivePort();
  final Runtime runtime = Runtime();

  runtime.textureId = textureId;
  runtime.setSurface(address, bytesPerRow, surfaceWidth, surfaceHeight);

  try {
    runtime.initState();
  } catch (e) {
    debugPrint("Error during runtime.initState: $e");
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
      } else if (message is RenderFrameMsg) {
        // Advance the engine the number of 1/60 s frames the ticker owes it, then render once
        Error err = runtime.update();
        for (int step = 1; err.ok && step < message.steps; step++) {
          err = runtime.update();
        }
        runtime.renderFrame();
        if (runtime.textureId != null) {
          sendPort.send(IsolateMessageType.notifyFrame);
        } else {
          sendPort.send(runtime.bytesList!);
        }
        if (!err.ok) {
          sendPort.send(RunningErrorMsg(err));
        }
        if (runtime.thumbnailPending) {
          if (err.ok && runtime.thumbnailReady) {
            runtime.renderPixels();
            sendPort.send(ThumbnailMsg(runtime.bytesList!, true));
          }
          runtime.endThumbnail();
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
        runtime.audioStart();
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.renderAudio) {
        runtime.renderAudio();
      } else if (message is IsolateMessageType &&
          message == IsolateMessageType.audioStop) {
        runtime.audioStop();
      } else if (message is OrientationChangeMsg) {
        // Receive the screen size and the safe area
        runtime.resize(
            message.width,
            message.height,
            message.safeTop,
            message.safeLeft,
            message.safeBottom,
            message.safeRight,
            message.keyboardHeight);
      } else if (message is SurfaceChangeMsg) {
        // Receive the new destination surface after a rotation or an inset change
        runtime.setSurface(message.address, message.bytesPerRow, message.width,
            message.height);
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
        // A program that registered ON THUMBNAIL draws its own icon; the handler runs inside the next update
        if (!runtime.startThumbnail()) {
          runtime.renderPixels();
          sendPort.send(ThumbnailMsg(runtime.bytesList!, false));
        }
      }
    } catch (e, stack) {
      debugPrint("Isolate error: $e\n$stack");
      sendPort.send(RunningErrorMsg(
          Error(code: -1, msg: "Isolate crashed: $e", position: -1)));
    }
  });
}

/// To receive the [ui.Image] to be paint on [CustomPainter].
typedef FrameCallback = void Function(ui.Image);

/// To receive the [img.Image] to be saved as thumbnail.
typedef ThumbnailCallback = void Function(img.Image image, bool programEnded);

/// To receive the error when the program is running.
typedef RunnerErrorCallback = void Function(Error);

/// To received the data disk to be saved on the device.
typedef SaveDataDiskCallback = void Function(String);

/// To receive the keyboard visibility.
typedef KeyboardVisibleCallback = void Function(bool);

/// To receive the input mode changes.
typedef InputModeCallback = void Function(bool);

/// Used to group all communication with the isolate in one place
class ComPort {
  late final Isolate isolate;
  late final ReceivePort receivePort;
  late final SendPort sendPort;

  /// Regulary render the frame.
  late final Ticker ticker;
  Duration prevDuration = Duration.zero;

  // To handle 60 FPS, or try to catch up.
  static const int _stepsPerSecond = 60;
  static const int _stepsPerAudioChunk = 2;
  static const int _maxStepsPerTick = 4;
  static const int _accumulatorPhase = Duration.microsecondsPerSecond ~/ 2;
  int _stepAccumulator = _accumulatorPhase;
  int _stepsSinceAudio = 0;

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

  StreamController<double> deltaTime = StreamController<double>();
  StreamController<double> updateTime = StreamController<double>();
  StreamController<double> renderTime = StreamController<double>();
  StreamController<double> decodeTime = StreamController<double>();
  StreamController<double> runtimeDeltaTime = StreamController<double>();

  int? textureId;

  /// Device-pixel size of the render surface, to skip redundant resizes.
  int _surfaceWidth = 0;
  int _surfaceHeight = 0;

  /// True once SHOWN/SAFE have been pushed to the isolate. The engine sizes the text window when
  /// the program starts, and the run page only lays out (and calls [resize]) after the edit page
  /// has already asked for the compile, so the first run has to seed the geometry itself.
  bool _geometrySent = false;

  /// Setup the communication with the isolate and listen for messages
  Future<SendPort> init() async {
    final Size physical =
        ui.PlatformDispatcher.instance.implicitView?.physicalSize ?? Size.zero;
    int width = physical.width.round();
    int height = physical.height.round();
    if (width <= 0 || height <= 0) {
      // no view metrics yet: the run page's first layout resizes the surface
      width = Runtime.screenWidth;
      height = Runtime.screenHeight;
    }
    final TextureSurface surface = await registerTexture(width, height);
    textureId = surface.textureId;
    _surfaceWidth = width;
    _surfaceHeight = height;

    receivePort = ReceivePort();
    isolate = await Isolate.spawn(isolateEntryPoint, [
      RootIsolateToken.instance!,
      receivePort.sendPort,
      surface.textureId,
      surface.address,
      surface.bytesPerRow,
      width,
      height,
    ]);

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
        ui.decodeImageFromPixels(
            message,
            Runtime.screenWidth,
            Runtime.screenHeight,
            Runtime.pixelsAreRgba
                ? ui.PixelFormat.rgba8888
                : ui.PixelFormat.bgra8888,
            onImage!);
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
                order: Runtime.pixelsAreRgba
                    ? img.ChannelOrder.rgba
                    : img.ChannelOrder.bgra,
                rowStride: Runtime.screenWidth * Runtime.bytePerPixel),
            x: 0,
            y: 0,
            width: ThumbnailMsg.thumbWidth,
            height: ThumbnailMsg.thumbHeight,
            antialias: false);
        onThumbnail!(image, message.programEnded);
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
      } else if (message is MeasurementMsg) {
        updateTime.add(message.updateTime);
        renderTime.add(message.renderTime);
      }
    });

    ticker = Ticker((Duration currDuration) {
      final delta = currDuration - prevDuration;
      deltaTime.add(delta.inMicroseconds / Duration.microsecondsPerSecond);
      prevDuration = currDuration;

      _stepAccumulator += delta.inMicroseconds * _stepsPerSecond;
      int steps = _stepAccumulator ~/ Duration.microsecondsPerSecond;
      if (steps <= 0) {
        // Display faster than 60 FPS
        return;
      }
      if (steps > _maxStepsPerTick) {
        steps = _maxStepsPerTick;
        _stepAccumulator = _accumulatorPhase;
      } else {
        _stepAccumulator -= steps * Duration.microsecondsPerSecond;
      }

      // NOTE: If started before the SendPort is ready, it will crash.
      sendPort.send(RenderFrameMsg(steps));

      // Audio is drained after the frames that filled the ring, not before.
      _stepsSinceAudio += steps;
      while (_stepsSinceAudio >= _stepsPerAudioChunk) {
        _stepsSinceAudio -= _stepsPerAudioChunk;
        sendPort.send(IsolateMessageType.renderAudio);
      }
    });

    return ready.future;
  }

  /// Compile and run the code in the isolate
  Future<Error> compileAndRun(String code, String dataDisk) async {
    _sendInitialGeometry();
    compileCompleter = Completer();
    sendPort.send(CompileAndRunMsg(code, dataDisk));
    return compileCompleter!.future;
  }

  /// Pushes the window geometry read straight from the view, for the case where the program is
  /// started before any page has laid out. Without it the engine boots with SHOWN 0x0 and the
  /// text window collapses, so PRINT writes nothing until the program calls CLS.
  void _sendInitialGeometry() {
    if (_geometrySent) return;
    final ui.FlutterView? view = ui.PlatformDispatcher.instance.implicitView;
    if (view == null) return;
    final double dpr = view.devicePixelRatio;
    if (dpr <= 0) return;
    final ui.ViewPadding padding = view.padding;
    sendPort.send(OrientationChangeMsg(
        view.physicalSize.width / dpr,
        view.physicalSize.height / dpr,
        padding.top / dpr,
        padding.left / dpr,
        padding.bottom / dpr,
        padding.right / dpr,
        view.viewInsets.bottom / dpr));
    _geometrySent = true;
  }

  /// Used to compile and report error in the editor
  Future<Error> compileOnly(String code) async {
    compileCompleter = Completer();
    sendPort.send(CompileOnlyMsg(code));
    return compileCompleter!.future;
  }

  /// Start updating the runtime at 60 fps
  void start() {
    debugPrint("Ticker started");
    sendPort.send(IsolateMessageType.audioStart);
    ticker.start();
    runtimeStopwatch.start();
    // sendPort.send(true);
  }

  /// Stop updating the runtime
  void stop() {
    debugPrint("Ticker stopped");
    ticker.stop();
    sendPort.send(IsolateMessageType.audioStop);
    prevDuration = Duration.zero;
    _stepAccumulator = _accumulatorPhase;
    _stepsSinceAudio = 0;
    runtimeStopwatch.stop();
    runtimeStopwatch.reset();
    prevRuntimeElapsed = 0;
  }

  /// Update the device screen size and the safe area, and resize the render surface to match.
  Future<void> resize(
      double inWidth,
      double inHeight,
      double safeTop,
      double safeLeft,
      double safeBottom,
      double safeRight,
      double keyboardHeight,
      double devicePixelRatio) async {
    sendPort.send(OrientationChangeMsg(inWidth, inHeight, safeTop, safeLeft,
        safeBottom, safeRight, keyboardHeight));
    _geometrySent = true;
    final int width = (inWidth * devicePixelRatio).round();
    final int height = (inHeight * devicePixelRatio).round();
    if (width <= 0 || height <= 0 || textureId == null) return;
    // Dedupe before the await, so the per-build calls from LayoutBuilder cannot queue duplicates.
    if (width == _surfaceWidth && height == _surfaceHeight) return;
    _surfaceWidth = width;
    _surfaceHeight = height;
    final TextureSurface surface =
        await resizeTexture(textureId!, width, height);
    sendPort.send(
        SurfaceChangeMsg(surface.address, surface.bytesPerRow, width, height));
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
