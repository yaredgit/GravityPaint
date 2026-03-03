#include "GravityPaint/core/Game.h"
#include <SDL.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#ifdef GRAVITYPAINT_ANDROID
#include <android/log.h>
#define LOG_TAG "Vectoria"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#endif

#ifdef __EMSCRIPTEN__
void emscripten_main_loop() {
    auto& game = GravityPaint::Game::getInstance();
    if (game.isRunning()) {
        game.runOneFrame();
    }
}
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    LOGI("Starting GravityPaint...");

    // Get screen size for mobile or use defaults
    int screenWidth = GravityPaint::DEFAULT_SCREEN_WIDTH;
    int screenHeight = GravityPaint::DEFAULT_SCREEN_HEIGHT;

#ifdef __EMSCRIPTEN__
    // On web, initialize from real canvas size to avoid viewport/input mismatch on mobile.
    int canvasW = 0;
    int canvasH = 0;
    if (emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH) == EMSCRIPTEN_RESULT_SUCCESS &&
        canvasW > 0 && canvasH > 0) {
        screenWidth = canvasW;
        screenHeight = canvasH;
    } else {
        screenWidth = 540;
        screenHeight = 960;
    }
    LOGI("Web build - canvas size: %dx%d", screenWidth, screenHeight);
#elif defined(GRAVITYPAINT_ANDROID) || defined(GRAVITYPAINT_IOS)
    // On mobile, get actual screen size
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        screenWidth = displayMode.w;
        screenHeight = displayMode.h;
    }
#else
    // On desktop, use smaller window for development
    screenWidth = 540;
    screenHeight = 960;
#endif

    auto& game = GravityPaint::Game::getInstance();

    if (!game.initialize(screenWidth, screenHeight)) {
        LOGE("Failed to initialize game!");
        return 1;
    }

    LOGI("Game initialized successfully");
    LOGI("Screen: %dx%d", screenWidth, screenHeight);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_main_loop, 0, 1);
#else
    game.run();
    game.shutdown();
#endif

    LOGI("Vectoria shutdown complete");
    return 0;
}
