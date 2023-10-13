package com.alexbatalov.fallout2ce;

import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class Run extends SDLActivity {
    public static String mod;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getArguments() {
        return new String[]{mod};
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
            "fallout2-ce",
        };
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.exit(0);
    }
}
