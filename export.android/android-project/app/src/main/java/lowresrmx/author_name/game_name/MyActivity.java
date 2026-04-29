package lowresrmx.author_name.game_name;

import android.content.res.Configuration;
import android.os.Bundle;
import android.view.View;
import android.util.Log;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import org.libsdl.app.SDLActivity;

public class MyActivity extends SDLActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
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
	protected String[] getArguments() {
		return new String[] {
			"app.rmx"
		};
	}

	@Override
	protected String[] getLibraries() {
		return new String[] { "lowresrmx" };
	}
}
