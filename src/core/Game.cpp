#include "Vectoria/core/Game.h"
#include "Vectoria/core/GameState.h"
#include "Vectoria/core/InputManager.h"
#include "Vectoria/core/ResourceManager.h"
#include "Vectoria/physics/PhysicsWorld.h"
#include "Vectoria/graphics/Renderer.h"
#include "Vectoria/audio/AudioManager.h"
#include "Vectoria/level/LevelManager.h"
#include "Vectoria/ui/HUD.h"
#include "Vectoria/Constants.h"

#include <SDL.h>

namespace Vectoria {

Game& Game::getInstance() {
    static Game instance;
    return instance;
}

bool Game::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_SENSOR) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return false;
    }

    // Create window
    uint32_t windowFlags = SDL_WINDOW_SHOWN;
#if defined(VECTORIA_ANDROID) || defined(VECTORIA_IOS)
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
    if (!m_levelManager->initialize()) {
        SDL_Log("Level manager initialization failed");
        return false;
    }

    m_hud = std::make_unique<HUD>(m_screenWidth, m_screenHeight);

    // Start with menu state
    changeState(GameStateType::Menu);

    m_running = true;
    m_lastFrameTime = SDL_GetPerformanceCounter();

    return true;
}

void Game::run() {
    while (m_running) {
        calculateDeltaTime();
        processEvents();
        
        if (!m_paused) {
            update(m_deltaTime);
        }
        
        render();

        // Frame rate limiting
        uint64_t frameEnd = SDL_GetPerformanceCounter();
        float frameTime = static_cast<float>(frameEnd - m_lastFrameTime) / SDL_GetPerformanceFrequency();
        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(static_cast<uint32_t>((TARGET_FRAME_TIME - frameTime) * 1000));
        }
    }
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
        // Handle input
        for (int i = 0; i < m_inputManager->getActiveTouchCount(); ++i) {
            m_currentState->handleInput(m_inputManager->getTouchPoint(i));
        }

        // Also handle mouse as touch on desktop
        if (m_inputManager->isMouseButtonDown(SDL_BUTTON_LEFT)) {
            TouchPoint mouseTouch;
            mouseTouch.position = m_inputManager->getMousePosition();
            mouseTouch.isActive = true;
            m_currentState->handleInput(mouseTouch);
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
    m_levelManager->resetLevel();
    m_physicsWorld->reset();
    resetScore();
    changeState(GameStateType::Playing);
}

void Game::nextLevel() {
    if (m_levelManager->hasNextLevel()) {
        m_levelManager->nextLevel();
        m_physicsWorld->reset();
        resetScore();
        changeState(GameStateType::Playing);
    } else {
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

} // namespace Vectoria
