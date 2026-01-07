#pragma once

namespace GravityPaint {

// Screen and rendering
constexpr int DEFAULT_SCREEN_WIDTH = 1080;
constexpr int DEFAULT_SCREEN_HEIGHT = 1920;
constexpr int TARGET_FPS = 60;
constexpr float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;

// Physics constants
constexpr float PHYSICS_SCALE = 30.0f;  // Pixels per meter for Box2D
constexpr float DEFAULT_GRAVITY_X = 0.0f;
constexpr float DEFAULT_GRAVITY_Y = 3.5f;  // Slower gravity for better gameplay
constexpr float MAX_GRAVITY_STRENGTH = 20.0f;
constexpr float GRAVITY_STROKE_LIFETIME = 2.0f;
constexpr float GRAVITY_STROKE_RADIUS = 150.0f;
constexpr int PHYSICS_VELOCITY_ITERATIONS = 8;
constexpr int PHYSICS_POSITION_ITERATIONS = 3;

// Gameplay
constexpr float MIN_SWIPE_DISTANCE = 15.0f;   // Reduced for better sensitivity
constexpr float MAX_SWIPE_DISTANCE = 300.0f;  // Reach max strength faster
constexpr int MAX_ACTIVE_STROKES = 5;
constexpr float ENERGY_TRANSFER_RATE = 0.7f;
constexpr float ENERGY_DECAY_RATE = 0.1f;
constexpr float MAX_OBJECT_ENERGY = 100.0f;

// Scoring
constexpr int BASE_GOAL_SCORE = 100;
constexpr int CHAIN_MULTIPLIER = 50;
constexpr int EFFICIENCY_BONUS = 25;
constexpr int TIME_BONUS_PER_SECOND = 10;
constexpr float COMBO_TIMEOUT = 1.5f;

// Visual
constexpr float PARTICLE_LIFETIME = 1.0f;
constexpr int MAX_PARTICLES = 500;
constexpr float TRAIL_LIFETIME = 0.5f;
constexpr int TRAIL_MAX_POINTS = 50;

// Audio
constexpr int AUDIO_FREQUENCY = 44100;
constexpr int AUDIO_CHANNELS = 2;
constexpr int AUDIO_CHUNK_SIZE = 2048;
constexpr float MASTER_VOLUME = 0.8f;
constexpr float MUSIC_VOLUME = 0.5f;
constexpr float SFX_VOLUME = 0.7f;

// Level
constexpr int MAX_LEVELS = 100;
constexpr float LEVEL_TIME_LIMIT = 90.0f;
constexpr int STARS_FOR_UNLOCK = 2;

// UI
constexpr float HUD_PADDING = 20.0f;
constexpr float BUTTON_HEIGHT = 80.0f;
constexpr float FONT_SIZE_SMALL = 24.0f;
constexpr float FONT_SIZE_MEDIUM = 36.0f;
constexpr float FONT_SIZE_LARGE = 48.0f;

// File paths
constexpr const char* ASSET_PATH = "assets/";
constexpr const char* LEVEL_PATH = "assets/levels/";
constexpr const char* SOUND_PATH = "assets/sounds/";
constexpr const char* SPRITE_PATH = "assets/sprites/";
constexpr const char* SAVE_FILE = "vectoria_save.dat";

} // namespace GravityPaint
