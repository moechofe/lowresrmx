package com.lowresrmx.core_plugin

import android.view.Surface
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.view.TextureRegistry

class CorePlugin: FlutterPlugin, MethodChannel.MethodCallHandler {
    private lateinit var channel: MethodChannel
    private lateinit var textureRegistry: TextureRegistry
    private val textures = mutableMapOf<Long, TextureRegistry.SurfaceTextureEntry>()
    private val surfaces = mutableMapOf<Long, Surface>()

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "com.lowresrmx/core_plugin")
        channel.setMethodCallHandler(this)
        textureRegistry = flutterPluginBinding.textureRegistry
    }

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when (call.method) {
            "registerTexture" -> {
                val entry = textureRegistry.createSurfaceTexture()
                val textureId = entry.id()
                entry.surfaceTexture().setDefaultBufferSize(216, 384)
                val surface = Surface(entry.surfaceTexture())
                textures[textureId] = entry
                surfaces[textureId] = surface
                nativeRegisterTexture(textureId, surface)
                result.success(textureId)
            }
            "unregisterTexture" -> {
                val id = call.arguments as Long
                nativeUnregisterTexture(id)
                surfaces.remove(id)?.release()
                textures.remove(id)?.release()
                result.success(null)
            }
            "getSurface" -> {
                val id = call.arguments as Long
                val entry = textures[id]
                if (entry != null) {
                    result.success(entry.surfaceTexture())
                } else {
                    result.error("NOT_FOUND", "Texture not found", null)
                }
            }
            "notifyFrameAvailable" -> {
                // On Android, SurfaceTexture.onFrameAvailable is triggered automatically
                // by ANativeWindow_unlockAndPost.
                result.success(null)
            }
            else -> result.notImplemented()
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
        surfaces.values.forEach { it.release() }
        surfaces.clear()
        textures.values.forEach { it.release() }
        textures.clear()
    }

    private external fun nativeRegisterTexture(textureId: Long, surface: Surface)
    private external fun nativeUnregisterTexture(textureId: Long)

    // JNI methods to be called from C++
    companion object {
        init {
            System.loadLibrary("core_plugin")
        }

        @JvmStatic
        fun getSurface(plugin: CorePlugin, textureId: Long): Surface? {
            return plugin.surfaces[textureId]
        }
    }
}
