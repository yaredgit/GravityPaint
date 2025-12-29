package com.gravitypaint.game;

import org.libsdl.app.SDLActivity;

public class GravityPaintActivity extends SDLActivity {
    
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "SDL2_mixer",
            "SDL2_ttf",
            "gravitypaint"
        };
    }
    
    @Override
    protected String getMainFunction() {
        return "main";
    }
}
