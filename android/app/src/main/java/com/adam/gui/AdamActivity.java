package com.adam.gui;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.system.Os;
import org.libsdl.app.SDLActivity;

public class ADAMActivity extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String cachePath = getCacheDir().getAbsolutePath();
        try {
            Os.setenv("TMPDIR", cachePath, true);
        } catch (Exception ignored) {}
        try {
            SDLActivity.nativeSetenv("TMPDIR", cachePath);
        } catch (Throwable ignored) {}

        String userDir = Environment.getExternalStorageDirectory().getAbsolutePath();
        if (!userDir.endsWith("/")) {
            userDir += "/";
        }
        try {
            Os.setenv("ADAM_USER_DIR", userDir, true);
        } catch (Exception ignored) {}
        try {
            SDLActivity.nativeSetenv("ADAM_USER_DIR", userDir);
        } catch (Throwable ignored) {}

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                try {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                    intent.addCategory("android.intent.category.DEFAULT");
                    intent.setData(Uri.parse(String.format("package:%s", getPackageName())));
                    startActivity(intent);
                } catch (Exception e) {
                    try {
                        Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                        startActivity(intent);
                    } catch (Exception ignored) {}
                }
            }
        }
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "adam-core",
            "adam-module-asterix",
            "adam-module-can",
            "adam-module-network",
            "adam-module-recrep",
            "adam-module-serial",
            "adam-gui"
        };
    }

    @Override
    protected String getMainFunction() {
        return "SDL_main";
    }
}
