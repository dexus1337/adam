package com.adam.gui;

import android.os.Bundle;
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
