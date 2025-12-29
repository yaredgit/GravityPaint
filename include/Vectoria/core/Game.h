#pragma once

#include "Vectoria/Types.h"
#include <memory>
#include <SDL.h>

namespace Vectoria {

class GameState;
class InputManager;
class ResourceManager;
class PhysicsWorld;
class Renderer;
class AudioManager;
class LevelManager;
class HUD;

class Game {
public:
    static Game& getInstance();

    bool initialize(int screenWidth = DEFAULT_SCREEN_WIDTH, int screenHeight = DEFAULT_SCREEN_HEIGHT);
    void run();
    void shutdown();

    void changeState(GameStateType newState);
    void pauseGame();
    void resumeGame();
    void restartLevel();
    void nextLevel();

    // Accessors
    InputManager* getInputManager() const { return m_inputManager.get(); }
    ResourceManager* getResourceManager() const { return m_resourceManager.get(); }
    PhysicsWorld* getPhysicsWorld() const { return m_physicsWorld.get(); }
    Renderer* getRenderer() const { return m_renderer.get(); }
    AudioManager* getAudioManager() const { return m_audioManager.get(); }
    LevelManager* getLevelManager() const { return m_levelManager.get(); }
    HUD* getHUD() const { return m_hud.get(); }

    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }
    float getDeltaTime() const { return m_deltaTime; }
    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }
    GameStateType getCurrentState() const { return m_currentStateType; }

    // Score management
    void addScore(int points);
    void resetScore();
    int getScore() const { return m_score; }
    int getHighScore() const { return m_highScore; }
    void incrementCombo();
    void resetCombo();
    int getCombo() const { return m_combo; }

private:
    Game() = default;
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void processEvents();
    void update(float deltaTime);
    void render();
    void calculateDeltaTime();

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;

    std::unique_ptr<GameState> m_currentState;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<PhysicsWorld> m_physicsWorld;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<LevelManager> m_levelManager;
    std::unique_ptr<HUD> m_hud;

    GameStateType m_currentStateType = GameStateType::Menu;
    
    int m_screenWidth = DEFAULT_SCREEN_WIDTH;
    int m_screenHeight = DEFAULT_SCREEN_HEIGHT;
    float m_deltaTime = 0.0f;
    uint64_t m_lastFrameTime = 0;
    
    bool m_running = false;
    bool m_paused = false;

    int m_score = 0;
    int m_highScore = 0;
    int m_combo = 0;
    float m_comboTimer = 0.0f;
};

} // namespace Vectoria
