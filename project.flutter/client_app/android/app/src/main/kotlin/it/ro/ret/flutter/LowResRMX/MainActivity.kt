package it.ro.ret.flutter.LowResRMX

import android.content.Intent
import android.net.Uri
import android.provider.OpenableColumns
import android.util.Log
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import java.io.File

class MainActivity : FlutterActivity() {
    private val CHANNEL = "com.lowresrmx/import"
    private val EXTENSION = ".rmx"

    private var channel: MethodChannel? = null
    private var dartReady = false
    private val pending = mutableListOf<Map<String, Any?>>()

    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        channel = MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).apply {
            setMethodCallHandler { call, result ->
                if (call.method == "drainPendingImports") {
                    dartReady = true
                    result.success(pending.toList())
                    pending.clear()
                } else {
                    result.notImplemented()
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: android.os.Bundle?) {
        super.onCreate(savedInstanceState)
        handleIntent(intent)
        setIntent(Intent(Intent.ACTION_MAIN))
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        handleIntent(intent)
        setIntent(Intent(Intent.ACTION_MAIN))
    }

    private fun handleIntent(intent: Intent?) {
        when (intent?.action) {
            Intent.ACTION_VIEW -> {
                val data = intent.data ?: return
                when (data.scheme) {
                    "lowresrmx" -> emit(mapOf("kind" to "uri", "uri" to data.toString()))
                    "content", "file" -> stage(data)
                }
            }

            Intent.ACTION_SEND -> {
                val uri = intent.getParcelableExtra<Uri>(Intent.EXTRA_STREAM) ?: return
                stage(uri)
            }

            Intent.ACTION_SEND_MULTIPLE -> {
                val uris = intent.getParcelableArrayListExtra<Uri>(Intent.EXTRA_STREAM) ?: return
                for (uri in uris) stage(uri)
            }
        }
    }

    private fun stage(uri: Uri) {
        try {
            val name = displayName(uri) ?: return
            if (!name.endsWith(EXTENSION, ignoreCase = true)) return

            val dir = File(cacheDir, "import").apply { mkdirs() }
            val target = File(dir, "${System.nanoTime()}$EXTENSION")
            contentResolver.openInputStream(uri)?.use { input ->
                target.outputStream().use { output -> input.copyTo(output) }
            } ?: return

            emit(
                mapOf(
                    "kind" to "file",
                    "path" to target.absolutePath,
                    "name" to name.substring(0, name.length - EXTENSION.length),
                ),
            )
        } catch (e: Exception) {
            Log.w("MainActivity", "Could not stage shared file", e)
        }
    }

    private fun displayName(uri: Uri): String? {
        contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null)
            ?.use { cursor ->
                if (cursor.moveToFirst()) {
                    val index = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
                    if (index >= 0 && !cursor.isNull(index)) return cursor.getString(index)
                }
            }
        return uri.lastPathSegment?.substringAfterLast('/')
    }

    private fun emit(payload: Map<String, Any?>) {
        if (dartReady) {
            channel?.invokeMethod("import", payload)
        } else {
            pending.add(payload)
        }
    }
}
