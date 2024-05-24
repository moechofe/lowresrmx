package com.moechofe.lowresrmx.texture.texture_plugin

import androidx.annotation.NonNull
// import android.view.Surface
import android.graphics.SurfaceTexture

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.view.TextureRegistry;

/** TexturePlugin */
class TexturePlugin: FlutterPlugin, MethodCallHandler {
	private lateinit var textureRegistry: TextureRegistry
	private lateinit var surfaceTexture: SurfaceTexture
	private var textureId: Long = -1

  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private lateinit var channel : MethodChannel

  override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "texture_plugin")
    channel.setMethodCallHandler(this)

		textureRegistry = flutterPluginBinding.textureRegistry
  }

  override fun onMethodCall(call: MethodCall, result: Result) {
    if (call.method == "getPlatformVersion") {
			result.success("Android ${android.os.Build.VERSION.RELEASE}")
		} else if (call.method == "createTexture") {

			// var producer: TextureRegistry.SurfaceProducer = textureRegistry.createSurfaceProducer();
			// producer.setSize(216,384);
			// var surface: Surface = producer.getSurface()

			var entry = textureRegistry.createSurfaceTexture()
			surfaceTexture = entry.surfaceTexture()
			surfaceTexture.setDefaultBufferSize(216,384)
			textureId = entry.id()

			result.success("Texture id: "+textureId.toString())
    } else {
      result.notImplemented()
    }
  }

  override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }
}
