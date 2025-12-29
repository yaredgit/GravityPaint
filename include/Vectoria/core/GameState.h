#pragma once

#include "Vectoria/Types.h"

namespace Vectoria {

class Game;

class GameState {
public:
    explicit GameState(Game* game) : m_game(game) {}
    virtual ~GameState() = default;

    virtual void enter() = 0;
    virtual void exit() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(const TouchPoint& touch) = 0;

protected:
    Game* m_game;
};

// Menu state
class MenuState : public GameState {
public:
    explicit MenuState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

private:
    float m_titlePulse = 0.0f;
    int m_selectedOption = 0;
};

// Playing state - main gameplay
class PlayingState : public GameState {
public:
    explicit PlayingState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

    void addGravityStroke(const GravityStroke& stroke);
    const std::vector<GravityStroke>& getStrokes() const { return m_gravityStrokes; }

private:
    void updateGravityStrokes(float deltaTime);
    void checkLevelCompletion();

    std::vector<GravityStroke> m_gravityStrokes;
    GravityStroke m_currentStroke;
    bool m_isDrawingStroke = false;
    float m_levelTime = 0.0f;
    bool m_levelComplete = false;
};

// Paused state
class PausedState : public GameState {
public:
    explicit PausedState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

private:
    int m_selectedOption = 0;
};

// Level complete state
class LevelCompleteState : public GameState {
public:
    explicit LevelCompleteState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

private:
    int m_starsEarned = 0;
    float m_animationTime = 0.0f;
};

// Game over state
class GameOverState : public GameState {
public:
    explicit GameOverState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

private:
    float m_fadeIn = 0.0f;
};

// Tutorial state
class TutorialState : public GameState {
public:
    explicit TutorialState(Game* game);
    void enter() override;
    void exit() override;
    void update(float deltaTime) override;
    void render() override;
    void handleInput(const TouchPoint& touch) override;

private:
    int m_tutorialStep = 0;
    float m_stepTime = 0.0f;
};

} // namespace Vectoria
