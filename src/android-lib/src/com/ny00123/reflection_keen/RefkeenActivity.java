package com.ny00123.reflection_keen;

import org.libsdl.app.SDLActivity; 
import android.view.View;

public class RefkeenActivity extends SDLActivity {

	private void setStickyImmersionMode() {
		if (android.os.Build.VERSION.SDK_INT >= 19) {
			getWindow().getDecorView().setSystemUiVisibility(
			    View.SYSTEM_UI_FLAG_LAYOUT_STABLE // 16
			    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION // 16
			    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN // 16
			    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // 14
			    | View.SYSTEM_UI_FLAG_FULLSCREEN // 16
			    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY // 19
			);
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		setStickyImmersionMode();
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			setStickyImmersionMode();
		}
	}
}
