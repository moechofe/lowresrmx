package com.lowresrmx.core_plugin

import android.view.Surface
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.view.TextureRegistry

class CorePlugin: FlutterPlugin, MethodChannel.MethodCallHandler {
    private lateinit var channel: MethodChannel
    private lateinit var textureRegistry: TextureRegistry
    // SurfaceProducer, not createSurfaceTexture(): a SurfaceTexture is sampled through
    // getTransformMatrix(), whose half-texel crop resamples the frame with a (w-1)/w scale and
    // softens every fantasy-pixel edge by one device pixel. The producer is ImageReader-backed
    // and composited 1:1.
    private val producers = mutableMapOf<Long, TextureRegistry.SurfaceProducer>()

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "com.lowresrmx/core_plugin")
        channel.setMethodCallHandler(this)
        textureRegistry = flutterPluginBinding.textureRegistry
    }

    /// Surface geometry as the method channel reports it on both platforms. The address is the
    /// ANativeWindow pointer, adopted by the render isolate exactly like the iOS pixel buffer, so
    /// the platform thread never mutates a handle the renderer is blitting into. bytesPerRow is 0
    /// because the pitch comes from the locked buffer.
    private fun surfaceMap(textureId: Long, surface: Surface, width: Int, height: Int): Map<String, Any> =
        mapOf(
            "textureId" to textureId,
            "address" to nativeSurfaceHandle(surface, width, height),
            "bytesPerRow" to 0
        )

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {
            "registerTexture" -> {
                val width = (call.argument<Int>("width") ?: 216).coerceAtLeast(1)
                val height = (call.argument<Int>("height") ?: 384).coerceAtLeast(1)
                val producer = textureRegistry.createSurfaceProducer()
                producer.setSize(width, height)
                val textureId = producer.id()
                producers[textureId] = producer
                result.success(surfaceMap(textureId, producer.surface, width, height))
            }
            "resizeTexture" -> {
                val id = (call.argument<Number>("textureId"))!!.toLong()
                val width = (call.argument<Int>("width") ?: 216).coerceAtLeast(1)
                val height = (call.argument<Int>("height") ?: 384).coerceAtLeast(1)
                val producer = producers[id]
                if (producer != null) {
                    producer.setSize(width, height)
                    // A resized producer hands out a new Surface; the render isolate adopts it
                    // and releases the window it was using.
                    result.success(surfaceMap(id, producer.surface, width, height))
                } else {
                    result.error("NOT_FOUND", "Texture not found", null)
                }
            }
            "unregisterTexture" -> {
                // A Dart int arrives as Integer for small values, so cast through Number.
                val id = (call.arguments as Number).toLong()
                nativeUnregisterTexture(id)
                producers.remove(id)?.release()
                result.success(null)
            }
            "notifyFrameAvailable" -> {
                // On Android, the frame is published by ANativeWindow_unlockAndPost.
                result.success(null)
            }
            else -> result.notImplemented()
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
        producers.values.forEach { it.release() }
        producers.clear()
    }

    /// Wraps the surface in an ANativeWindow, requests RGBA_8888 at the device-pixel size and
    /// returns the pointer. Ownership of that reference passes to the C side.
    private external fun nativeSurfaceHandle(surface: Surface, width: Int, height: Int): Long
    private external fun nativeUnregisterTexture(textureId: Long)

    companion object {
        init {
            System.loadLibrary("core_plugin")
        }
    }
}
