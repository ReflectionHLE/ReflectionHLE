package com.ny00123.reflection_keen;

import org.libsdl.app.SDLActivity; 
import android.view.View;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.Manifest;

public class RefkeenActivity extends SDLActivity implements ActivityCompat.OnRequestPermissionsResultCallback {

	private final int[] requestNotif = new int[1];

	private void waitForUIThread(Object notif) {
		synchronized (notif) {
			try {
				notif.wait();
			} catch (InterruptedException ex) {
				ex.printStackTrace();
			}
		}
	}

	public int requestReadExternalStoragePermission() {
		if (android.os.Build.VERSION.SDK_INT >= 23) {
			if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
				if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {

					final Object alertNotif = new Object();

					runOnUiThread(new Runnable() {
						@Override
						public void run() {
							AlertDialog.Builder builder = new AlertDialog.Builder(mSingleton);
							builder.setMessage("Thanks for your interest in Reflection Keen. Allowing access to shared storage should let you select a supported game installation from more locations.");
							builder.setTitle("Access to shared storage requested");
							builder.setNeutralButton("Continue", new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int id) {
									synchronized (alertNotif) {
										alertNotif.notify();
									}
								}
							});
							builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
								public void onDismiss(final DialogInterface dialog) {
									synchronized (alertNotif) {
										alertNotif.notify();
									}
								}
							});

							AlertDialog dialog = builder.create();
							dialog.show();
						}
					});

					// Wait for onClick from dialog button
					waitForUIThread(alertNotif);
				}

				runOnUiThread(new Runnable() {
					@Override
					public void run() {
						ActivityCompat.requestPermissions(mSingleton, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 0/*Currently unused*/);
					}
				});

				// Wait for onRequestPermissionsResults
				waitForUIThread(requestNotif);

				return requestNotif[0];
			}
		}
		return 1;
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		for (int i = 0; i < permissions.length; i++) {
			if (permissions[i].equals(Manifest.permission.READ_EXTERNAL_STORAGE)) {
				synchronized (requestNotif) {
					requestNotif[0] = (grantResults[i] == PackageManager.PERMISSION_GRANTED) ? 1 : 0;
					requestNotif.notify();
				}
				break;
			}
		}
	}

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
