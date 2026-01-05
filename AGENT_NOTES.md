# Agent Session Notes - GravityPaint

**Last Updated:** January 5, 2026

## Project Overview
GravityPaint is a physics-based puzzle game (C++17, SDL2, Box2D) where players draw gravity strokes to guide objects to goals.

## Current Task: Deploy to GitHub Pages
We're setting up web deployment using Emscripten/WebAssembly.

### Completed
- [x] GitHub Actions workflow created: `.github/workflows/deploy-pages.yml`
- [x] Web shell HTML created: `web/shell.html` (updated branding from Vectoria to GravityPaint)
- [x] Fixed font loading issue - added Windows system font fallback in `src/graphics/Renderer.cpp`
- [x] Local Windows build works: `build-mingw.bat` -> `build-mingw/GravityPaint.exe`
- [x] Commit created locally: `a15e6ef Add GitHub Pages deployment with Emscripten build`

### Pending - NEEDS TO BE DONE
1. **Push to GitHub** - The repository https://github.com/yaredgit/GravityPaint is EMPTY
   - Push requires a token with `workflow` scope (Factory token doesn't have it)
   - User needs to push manually: `git push -u origin master`
   
2. **Enable GitHub Pages** - After push:
   - Go to repo Settings > Pages
   - Set Source to "GitHub Actions"
   
3. **Run workflow** - May need manual trigger first time:
   - Go to Actions tab, click "Deploy to GitHub Pages", click "Run workflow"

4. **Test deployment** at: `https://yaredgit.github.io/GravityPaint/`

## Known Issues

### Sound not working (Windows build)
- Audio initializes successfully (14 sounds generated)
- Mix_OpenAudio succeeds, sounds play on channels
- But no audible output - possibly format issue with Mix_QuickLoad_RAW
- Needs investigation - check `src/audio/AudioManager.cpp`

### Required DLLs for Windows
Copy these from `C:\msys64\mingw64\bin\` to `build-mingw\`:
- SDL2.dll, SDL2_mixer.dll, SDL2_ttf.dll
- libfreetype-6.dll, zlib1.dll, libpng16-16.dll, libbz2-1.dll
- libharfbuzz-0.dll, libbrotlidec.dll, libbrotlicommon.dll
- libglib-2.0-0.dll, libintl-8.dll, libgraphite2.dll, libiconv-2.dll, libpcre2-8-0.dll
- libgcc_s_seh-1.dll, libstdc++-6.dll, libwinpthread-1.dll
- libFLAC.dll, libmpg123-0.dll, libopusfile-0.dll, libvorbisfile-3.dll
- libvorbis-0.dll, libogg-0.dll, libopus-0.dll
- libwavpack-1.dll, libxmp.dll

## Key Files
- `PROJECT_STATUS.md` - Full project status and feature list
- `include/GravityPaint/Constants.h` - Physics tuning constants
- `src/graphics/Renderer.cpp` - Font loading (lines 35-56)
- `src/audio/AudioManager.cpp` - Programmatic audio generation
- `.github/workflows/deploy-pages.yml` - GitHub Pages deployment workflow
- `web/shell.html` - Browser game container

## Build Commands
```bash
# Windows (MinGW)
cd C:\MyProj\GravityPaint
.\build-mingw.bat
.\build-mingw\GravityPaint.exe

# Clean rebuild
Remove-Item -Recurse -Force build-mingw\*
.\build-mingw.bat
```

## Git Status
- Local branch: master
- Remote: https://github.com/yaredgit/GravityPaint.git
- Local commit ahead of remote (remote is empty)

## Next Steps for New Session
1. Help user push code to GitHub (needs workflow token)
2. Verify GitHub Actions workflow runs
3. Test web deployment
4. Debug sound issue if needed
