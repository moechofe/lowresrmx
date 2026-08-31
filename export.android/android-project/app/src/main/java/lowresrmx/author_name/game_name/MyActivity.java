package lowresrmx.author_name.game_name;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Choreographer;
import android.view.View;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import org.libsdl.app.SDLActivity;

public class MyActivity extends SDLActivity {

	private static native void nativeSetKeyboardEnabled(boolean enabled);
	private static native void nativeSetKeyboardHeight(int height);
	private static native int nativeMachinePeek(int address);
	private static native boolean nativeMachinePoke(int address, int value);

	// This address will contains 123 when user tap the Game Creator link
	private static final int PEEK_ADDRESS = 0xd999;

	private Choreographer.FrameCallback frameCallback;

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

		frameCallback = new Choreographer.FrameCallback() {
			@Override
			public void doFrame(long frameTimeNanos) {
				int value = nativeMachinePeek(PEEK_ADDRESS);
				if (value > 0) {
					// Consume the request so the program can post a new one next frame.
					nativeMachinePoke(PEEK_ADDRESS, 0);
					onD999(value);
				}
				Choreographer.getInstance().postFrameCallback(this);
			}
		};

		// Keyboard height detection
		final View decorView = getWindow().getDecorView();
		decorView.getViewTreeObserver().addOnGlobalLayoutListener(new android.view.ViewTreeObserver.OnGlobalLayoutListener() {
			private int lastKeyboardHeight = 0;
			private boolean lastKeyboardShown = false;
			@Override
			public void onGlobalLayout() {
				android.graphics.Rect r = new android.graphics.Rect();
				decorView.getWindowVisibleDisplayFrame(r);
				int screenHeight = decorView.getRootView().getHeight();
				int keyboardHeight = screenHeight - r.bottom;
				// Use a threshold to avoid false positives from system UI
				int threshold = (int)(screenHeight * 0.15); // 15% of screen height
				boolean isKeyboardVisible = keyboardHeight > threshold;
				if (keyboardHeight != lastKeyboardHeight) {
					lastKeyboardHeight = keyboardHeight;
					if (!lastKeyboardShown && isKeyboardVisible) {
						nativeSetKeyboardEnabled(true);
						nativeSetKeyboardHeight(keyboardHeight);
					}
					else if (lastKeyboardShown && !isKeyboardVisible) {
						nativeSetKeyboardEnabled(false);
						nativeSetKeyboardHeight(0);
					}
					lastKeyboardShown = isKeyboardVisible;
				}
			}
		});
	}

	/** Called with each non-zero value posted at PEEK_ADDRESS; the value is reset to 0 before this runs. */
	private void onD999(int value) {
		if (value == 123) {
			openBrowser("https://ret.ro.it/about.html");
		}
	}

	private void openBrowser(String url) {
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		try {
			startActivity(intent);
		} catch (ActivityNotFoundException e) {
			Log.w("MyActivity", "No browser available to open " + url, e);
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		Choreographer.getInstance().postFrameCallback(frameCallback);
	}

	@Override
	protected void onPause() {
		Choreographer.getInstance().removeFrameCallback(frameCallback);
		super.onPause();
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			hideSystemUI();
		}
	}

	private void hideSystemUI() {
		View decorView = getWindow().getDecorView();
		WindowInsetsControllerCompat windowInsetsController =
				WindowCompat.getInsetsController(getWindow(), decorView);
		if (windowInsetsController != null) {
			windowInsetsController.hide(WindowInsetsCompat.Type.systemBars());
			windowInsetsController.setSystemBarsBehavior(
					WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
		}
	}

	@Override
	protected String[] getArguments() { return new String[] { "app.rmx" }; }

	@Override
	protected String[] getLibraries() {
		return new String[] { "lowresrmx" };
	}
}
