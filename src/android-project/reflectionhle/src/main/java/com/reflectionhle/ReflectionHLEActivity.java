package com.reflectionhle;

import org.libsdl.app.SDLActivity; 
import android.view.View;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.Manifest;
import android.net.Uri;
import android.provider.Settings;

public class ReflectionHLEActivity extends SDLActivity implements ActivityCompat.OnRequestPermissionsResultCallback {

	private final int[] requestNotif = new int[1];
	private final Object alertNotif = new Object();

	class PermissionRequestExplanation implements Runnable {
		@Override
		public void run() {
			AlertDialog.Builder builder = new AlertDialog.Builder(mSingleton);
			builder.setMessage("Thanks for your interest in ReflectionHLE. Allowing access to shared storage should let you select a supported game installation from more locations.");
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
	}

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
		if (android.os.Build.VERSION.SDK_INT >= 30) {
			if (!android.os.Environment.isExternalStorageManager()) {
				if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.MANAGE_EXTERNAL_STORAGE)) {
					runOnUiThread(new PermissionRequestExplanation());
					// Wait for onClick from dialog button
					waitForUIThread(alertNotif);
				}

				try {
					Intent intent = new Intent();
					intent.setAction(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
					Uri uri = Uri.fromParts("package", this.getPackageName(), null);
					intent.setData(uri);
					startActivity(intent);
				} catch (Exception e) {
					Intent intent = new Intent();
					intent.setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
					startActivity(intent);
				}
			}
		} else if (android.os.Build.VERSION.SDK_INT >= 23) {
			if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
				if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
					runOnUiThread(new PermissionRequestExplanation());
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

	@Override
	protected String[] getLibraries() {
	        return new String[] { "SDL2", "reflectionhle" };
	}
}
