#include "Vectoria/core/Game.h"
#include <SDL.h>
#include <iostream>

#ifdef VECTORIA_ANDROID
#include <android/log.h>
#define LOG_TAG "Vectoria"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__); printf("\n")
#define LOGE(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#endif

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    LOGI("Starting Vectoria...");

    // Get screen size for mobile or use defaults
    int screenWidth = Vectoria::DEFAULT_SCREEN_WIDTH;
    int screenHeight = Vectoria::DEFAULT_SCREEN_HEIGHT;

#if defined(VECTORIA_ANDROID) || defined(VECTORIA_IOS)
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

    auto& game = Vectoria::Game::getInstance();

    if (!game.initialize(screenWidth, screenHeight)) {
        LOGE("Failed to initialize game!");
        return 1;
    }

    LOGI("Game initialized successfully");
    LOGI("Screen: %dx%d", screenWidth, screenHeight);

    game.run();
    game.shutdown();

    LOGI("Vectoria shutdown complete");
    return 0;
}
