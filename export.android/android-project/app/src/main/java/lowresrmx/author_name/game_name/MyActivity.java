package lowresrmx.author_name.game_name;

import android.os.Bundle;
import android.view.View;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import org.libsdl.app.SDLActivity;

public class MyActivity extends SDLActivity {

	private static native void nativeSetKeyboardEnabled(boolean enabled);
	private static native void nativeSetKeyboardHeight(int height);

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

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
