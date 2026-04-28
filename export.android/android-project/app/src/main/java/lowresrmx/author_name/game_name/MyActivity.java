package lowresrmx.author_name.game_name;

import android.content.res.Configuration;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;

public class MyActivity extends SDLActivity {

	public native void nativeInitCoreWrapper();
	public native void nativeCoreUpdate();

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		nativeInitCoreWrapper();
	}

	@Override
	protected String[] getLibraries() {
		return new String[] { "lowresrmx" };
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		nativeCoreUpdate();
	}
}
