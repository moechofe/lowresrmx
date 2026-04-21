package com.lowresrmx.client_app

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel

class MainActivity: FlutterActivity()
{
	private val CHANNEL = "lowresrmx/customescheme"
	private var initialLink: String? = null

	override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
		super.configureFlutterEngine(flutterEngine)
		MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).setMethodCallHandler {
			call, result ->
			if (call.method == "getInitialLink") {
				result.success(initialLink)
				// Clear initialLink after use once
				initialLink = null
			} else {
				result.notImplemented()
			}
		}
	}

	override fun onCreate(savedInstanceState: android.os.Bundle?) {
		super.onCreate(savedInstanceState)
		handleIntent(intent)
	}

	override fun onNewIntent(intent: Intent) {
		super.onNewIntent(intent)
		handleIntent(intent)
	}

	private fun handleIntent(intent: Intent?) {
		val appLinkAction = intent?.action
		val appLinkData: Uri? = intent?.data

		if (Intent.ACTION_VIEW == appLinkAction && appLinkData != null) {
			val uri = appLinkData.toString()
			if (flutterEngine != null) {
				// If the Flutter engine is already running, send the link to the Dart side
				MethodChannel(flutterEngine!!.dartExecutor.binaryMessenger, CHANNEL).invokeMethod("onURLReceived", uri)
			} else {
				// If the application is not yet launched, save the link
				initialLink = uri
			}
		}
	}
}
