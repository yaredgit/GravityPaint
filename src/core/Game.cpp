#include "GravityPaint/core/Game.h"
#include "GravityPaint/core/GameState.h"
#include "GravityPaint/core/InputManager.h"
#include "GravityPaint/core/ResourceManager.h"
#include "GravityPaint/physics/PhysicsWorld.h"
#include "GravityPaint/graphics/Renderer.h"
#include "GravityPaint/audio/AudioManager.h"
#include "GravityPaint/level/LevelManager.h"
#include "GravityPaint/ui/HUD.h"
#include "GravityPaint/Constants.h"

#include <SDL.h>
#include <fstream>

namespace GravityPaint {

Game& Game::getInstance() {
    static Game instance;
    return instance;
}

bool Game::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize SDL
    uint32_t sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS;
#ifndef __EMSCRIPTEN__
    sdlFlags |= SDL_INIT_SENSOR;
#endif
    if (SDL_Init(sdlFlags) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return false;
    }

    // Create window
    uint32_t windowFlags = SDL_WINDOW_SHOWN;
#if defined(GRAVITYPAINT_ANDROID) || defined(GRAVITYPAINT_IOS)
    windowFlags |= SDL_WINDOW_FULLSCREEN;
#else
    windowFlags |= SDL_WINDOW_RESIZABLE;
#endif

    m_window = SDL_CreateWindow(
        "Vectoria",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_screenWidth,
        m_screenHeight,
        windowFlags
    );

    if (!m_window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    // Initialize subsystems
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->initialize(m_window, m_screenWidth, m_screenHeight)) {
        SDL_Log("Renderer initialization failed");
        return false;
    }

    m_inputManager = std::make_unique<InputManager>();
    m_resourceManager = std::make_unique<ResourceManager>();
    
    if (!m_resourceManager->initialize(m_renderer->getSDLRenderer())) {
        SDL_Log("Resource manager initialization failed");
        return false;
    }

    m_audioManager = std::make_unique<AudioManager>();
    if (!m_audioManager->initialize()) {
        SDL_Log("Audio manager initialization failed (continuing without audio)");
    }

    m_physicsWorld = std::make_unique<PhysicsWorld>();
    if (!m_physicsWorld->initialize()) {
        SDL_Log("Physics world initialization failed");
        return false;
    }

    m_levelManager = std::make_unique<LevelManager>();
    if (!m_levelManager->initialize(m_screenWidth, m_screenHeight)) {
        SDL_Log("Level manager initialization failed");
        return false;
    }

    m_hud = std::make_unique<HUD>(m_screenWidth, m_screenHeight);
    
    // Set up click sound callback
    m_hud->setClickSoundCallback([this]() {
        if (m_soundEnabled) {
            m_audioManager->playSound(SoundEffect::ButtonClick);
        }
    });

    // Start with menu state
    changeState(GameStateType::Menu);

    m_running = true;
    m_lastFrameTime = SDL_GetPerformanceCounter();

    return true;
}

void Game::run() {
    while (m_running) {
        runOneFrame();
    }
}

void Game::runOneFrame() {
    calculateDeltaTime();
    processEvents();
    
    // Always call update - it handles input for all states including paused
    update(m_deltaTime);
    
    render();

#ifndef __EMSCRIPTEN__
    // Frame rate limiting (not needed for Emscripten - browser handles it)
    uint64_t frameEnd = SDL_GetPerformanceCounter();
    float frameTime = static_cast<float>(frameEnd - m_lastFrameTime) / SDL_GetPerformanceFrequency();
    if (frameTime < TARGET_FRAME_TIME) {
        SDL_Delay(static_cast<uint32_t>((TARGET_FRAME_TIME - frameTime) * 1000));
    }
#endif
}

void Game::shutdown() {
    m_currentState.reset();
    m_hud.reset();
    m_levelManager->shutdown();
    m_levelManager.reset();
    m_physicsWorld->shutdown();
    m_physicsWorld.reset();
    m_audioManager->shutdown();
    m_audioManager.reset();
    m_resourceManager->shutdown();
    m_resourceManager.reset();
    m_inputManager.reset();
    m_renderer->shutdown();
    m_renderer.reset();

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void Game::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    m_screenWidth = event.window.data1;
                    m_screenHeight = event.window.data2;
                }
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (m_currentStateType == GameStateType::Playing) {
                        pauseGame();
                    } else if (m_currentStateType == GameStateType::Paused) {
                        resumeGame();
                    }
                }
                break;

            default:
                break;
        }

        m_inputManager->processEvent(event);
    }
}

void Game::update(float deltaTime) {
    m_inputManager->update(deltaTime);

    // Update combo timer
    if (m_combo > 0) {
        m_comboTimer -= deltaTime;
        if (m_comboTimer <= 0) {
            resetCombo();
        }
    }

    if (m_currentState) {
        bool pressed = m_inputManager->isMouseButtonPressed(SDL_BUTTON_LEFT);
        bool down = m_inputManager->isMouseButtonDown(SDL_BUTTON_LEFT);
        bool released = m_inputManager->isMouseButtonReleased(SDL_BUTTON_LEFT);
        
        // Process on press
        if (pressed) {
            TouchPoint pressTouch;
            pressTouch.position = m_inputManager->getMousePosition();
            pressTouch.isActive = true;
            pressTouch.id = 0;
            m_currentState->handleInput(pressTouch);
        }
        // Process continuous drag for drawing strokes
        else if (down) {
            TouchPoint dragTouch;
            dragTouch.position = m_inputManager->getMousePosition();
            dragTouch.isActive = true;
            dragTouch.id = 0;
            m_currentState->handleInput(dragTouch);
        }
        // Process on release
        else if (released) {
            TouchPoint releaseTouch;
            releaseTouch.position = m_inputManager->getMousePosition();
            releaseTouch.isActive = false;
            releaseTouch.id = 0;
            m_currentState->handleInput(releaseTouch);
        }

        m_currentState->update(deltaTime);
    }

    m_hud->update(deltaTime);
}

void Game::render() {
    m_renderer->beginFrame();
    m_renderer->clear(Color(15, 15, 30));

    if (m_currentState) {
        m_currentState->render();
    }

    m_hud->render(m_renderer.get());

    m_renderer->endFrame();
}

void Game::calculateDeltaTime() {
    uint64_t currentTime = SDL_GetPerformanceCounter();
    m_deltaTime = static_cast<float>(currentTime - m_lastFrameTime) / SDL_GetPerformanceFrequency();
    m_lastFrameTime = currentTime;

    // Clamp delta time to avoid physics explosions
    if (m_deltaTime > 0.1f) {
        m_deltaTime = 0.1f;
    }
}

void Game::changeState(GameStateType newState) {
    if (m_currentState) {
        m_currentState->exit();
    }

    m_currentStateType = newState;

    switch (newState) {
        case GameStateType::Menu:
            m_currentState = std::make_unique<MenuState>(this);
            break;
        case GameStateType::DifficultySelect:
            m_currentState = std::make_unique<DifficultySelectState>(this);
            break;
        case GameStateType::LevelSelect:
            m_currentState = std::make_unique<LevelSelectState>(this);
            break;
        case GameStateType::Settings:
            m_currentState = std::make_unique<SettingsState>(this);
            break;
        case GameStateType::Playing:
            m_currentState = std::make_unique<PlayingState>(this);
            break;
        case GameStateType::Paused:
            m_currentState = std::make_unique<PausedState>(this);
            break;
        case GameStateType::LevelComplete:
            m_currentState = std::make_unique<LevelCompleteState>(this);
            break;
        case GameStateType::GameOver:
            m_currentState = std::make_unique<GameOverState>(this);
            break;
        case GameStateType::Tutorial:
            m_currentState = std::make_unique<TutorialState>(this);
            break;
    }

    if (m_currentState) {
        m_currentState->enter();
    }
}

void Game::pauseGame() {
    if (m_currentStateType == GameStateType::Playing) {
        m_paused = true;
        changeState(GameStateType::Paused);
    }
}

void Game::resumeGame() {
    if (m_currentStateType == GameStateType::Paused) {
        m_paused = false;
        changeState(GameStateType::Playing);
    }
}

void Game::restartLevel() {
    // Restarting costs a life
    loseLife();
    
    // If still alive, restart the level
    if (m_lives > 0) {
        m_levelManager->reloadCurrentLevel();
        resetScore();
        changeState(GameStateType::Playing);
    }
    // Otherwise loseLife() already changed to GameOver
}

void Game::nextLevel() {
    if (m_levelManager->hasNextLevel()) {
        m_levelManager->nextLevel();
        resetScore();
        changeState(GameStateType::Playing);
    } else {
        // All levels complete
        changeState(GameStateType::Menu);
    }
}

void Game::addScore(int points) {
    int multiplier = 1 + m_combo;
    m_score += points * multiplier;
    
    if (m_score > m_highScore) {
        m_highScore = m_score;
    }

    m_hud->setScore(m_score);
    m_hud->setHighScore(m_highScore);
}

void Game::resetScore() {
    m_score = 0;
    m_combo = 0;
    m_comboTimer = 0;
    m_hud->setScore(0);
    m_hud->setCombo(0);
}

void Game::incrementCombo() {
    m_combo++;
    m_comboTimer = COMBO_TIMEOUT;
    m_hud->setCombo(m_combo);
}

void Game::resetCombo() {
    m_combo = 0;
    m_comboTimer = 0;
    m_hud->setCombo(0);
}

void Game::loseLife() {
    m_lives--;
    if (m_lives <= 0) {
        m_lives = 0;
        changeState(GameStateType::GameOver);
    }
}

void Game::resetLives() {
    m_lives = m_maxLives;
}

void Game::addLife() {
    if (m_lives < m_maxLives) {
        m_lives++;
    }
}

void Game::saveProgress() {
    std::ofstream file("gravitypaint_save.dat", std::ios::binary);
    if (!file.is_open()) return;
    
    // Save settings
    file.write(reinterpret_cast<const char*>(&m_difficulty), sizeof(m_difficulty));
    file.write(reinterpret_cast<const char*>(&m_gameMode), sizeof(m_gameMode));
    file.write(reinterpret_cast<const char*>(&m_soundEnabled), sizeof(m_soundEnabled));
    file.write(reinterpret_cast<const char*>(&m_musicEnabled), sizeof(m_musicEnabled));
    file.write(reinterpret_cast<const char*>(&m_highScore), sizeof(m_highScore));
    
    // Save level progress
    m_levelManager->saveProgress("gravitypaint_levels.dat");
}

void Game::loadProgress() {
    std::ifstream file("gravitypaint_save.dat", std::ios::binary);
    if (!file.is_open()) return;
    
    // Load settings
    file.read(reinterpret_cast<char*>(&m_difficulty), sizeof(m_difficulty));
    file.read(reinterpret_cast<char*>(&m_gameMode), sizeof(m_gameMode));
    file.read(reinterpret_cast<char*>(&m_soundEnabled), sizeof(m_soundEnabled));
    file.read(reinterpret_cast<char*>(&m_musicEnabled), sizeof(m_musicEnabled));
    file.read(reinterpret_cast<char*>(&m_highScore), sizeof(m_highScore));
    
    // Load level progress
    m_levelManager->loadProgress("gravitypaint_levels.dat");
}

} // namespace GravityPaint
