#include "Vectoria/core/GameState.h"
#include "Vectoria/core/Game.h"
#include "Vectoria/core/InputManager.h"
#include "Vectoria/physics/PhysicsWorld.h"
#include "Vectoria/physics/PhysicsObject.h"
#include "Vectoria/graphics/Renderer.h"
#include "Vectoria/audio/AudioManager.h"
#include "Vectoria/level/LevelManager.h"
#include "Vectoria/level/Level.h"
#include "Vectoria/ui/HUD.h"
#include "Vectoria/Constants.h"

namespace Vectoria {

// MenuState
MenuState::MenuState(Game* game) : GameState(game) {}

void MenuState::enter() {
    m_titlePulse = 0.0f;
    m_selectedOption = 0;
    
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    
    float centerX = m_game->getScreenWidth() / 2.0f;
    float startY = m_game->getScreenHeight() * 0.5f;
    float buttonWidth = 300.0f;
    float buttonHeight = 70.0f;
    float spacing = 90.0f;

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY, buttonWidth, buttonHeight),
        "PLAY",
        [this]() { m_game->changeState(GameStateType::Playing); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing, buttonWidth, buttonHeight),
        "TUTORIAL",
        [this]() { m_game->changeState(GameStateType::Tutorial); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 2, buttonWidth, buttonHeight),
        "QUIT",
        [this]() { SDL_Event quit; quit.type = SDL_QUIT; SDL_PushEvent(&quit); }
    );
}

void MenuState::exit() {
    m_game->getHUD()->clearButtons();
}

void MenuState::update(float deltaTime) {
    m_titlePulse += deltaTime * 2.0f;
}

void MenuState::render() {
    auto* renderer = m_game->getRenderer();
    
    // Draw title with pulse effect
    float pulse = 1.0f + 0.1f * std::sin(m_titlePulse);
    renderer->drawTextCentered(
        "VECTORIA",
        Vec2(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.25f),
        Color::cyan(),
        72.0f * pulse
    );

    renderer->drawTextCentered(
        "Gravity Painter",
        Vec2(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.32f),
        Color(180, 180, 200),
        24.0f
    );

    // Draw decorative gravity field visualization
    float time = m_titlePulse;
    Vec2 center(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.15f);
    for (int i = 0; i < 8; ++i) {
        float angle = (i / 8.0f) * 3.14159f * 2.0f + time * 0.5f;
        float radius = 80.0f + 20.0f * std::sin(time + i);
        Vec2 pos = center + Vec2(std::cos(angle), std::sin(angle)) * radius;
        renderer->drawCircle(pos, 10.0f, Color::cyan(), false);
    }
}

void MenuState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// PlayingState
PlayingState::PlayingState(Game* game) : GameState(game) {}

void PlayingState::enter() {
    m_gravityStrokes.clear();
    m_isDrawingStroke = false;
    m_levelTime = 0.0f;
    m_levelComplete = false;

    auto* levelManager = m_game->getLevelManager();
    auto* physics = m_game->getPhysicsWorld();
    auto* hud = m_game->getHUD();

    // Load first level if not loaded
    if (!levelManager->getCurrentLevel()) {
        levelManager->loadLevel(1);
    }

    levelManager->startLevel();

    auto* level = levelManager->getCurrentLevel();
    if (level) {
        // Set up physics boundaries
        physics->createBoundaries(level->getWidth(), level->getHeight());
        physics->createGoalZone(level->getGoalZone().center(), 
                               Vec2(level->getGoalZone().w, level->getGoalZone().h));

        // Spawn initial objects
        levelManager->spawnObjects(physics);

        // Update HUD
        hud->setLevelNumber(level->getId());
        hud->setTimeLimit(level->getTimeLimit());
        hud->setObjective(level->getObjective() ? level->getObjective()->getDescription() : "Reach the goal!");
        hud->setStrokeCount(0, level->getMaxStrokes());
    }

    hud->setPauseButtonVisible(true);
    hud->setVisible(true);
}

void PlayingState::exit() {
    m_game->getHUD()->setPauseButtonVisible(false);
}

void PlayingState::update(float deltaTime) {
    if (m_levelComplete) return;

    m_levelTime += deltaTime;
    
    auto* physics = m_game->getPhysicsWorld();
    auto* levelManager = m_game->getLevelManager();
    auto* hud = m_game->getHUD();

    // Update gravity strokes
    updateGravityStrokes(deltaTime);

    // Apply gravity from strokes to physics
    physics->applyGravityFromStrokes(m_gravityStrokes);

    // Update physics
    physics->update(deltaTime);

    // Update spawns
    levelManager->updateSpawns(deltaTime, physics);
    levelManager->updateLevel(deltaTime);

    // Update HUD
    hud->setLevelTime(m_levelTime);
    hud->setProgress(levelManager->getCurrentLevel()->getObjective() 
                     ? levelManager->getCurrentLevel()->getObjective()->getProgress() : 0.0f);

    // Check for level completion
    checkLevelCompletion();
}

void PlayingState::render() {
    auto* renderer = m_game->getRenderer();
    auto* physics = m_game->getPhysicsWorld();
    auto* level = m_game->getLevelManager()->getCurrentLevel();

    // Draw background grid
    renderer->drawGrid(50.0f, Color(30, 30, 50, 100));

    // Draw goal zone
    if (level) {
        renderer->drawGoalZone(level->getGoalZone());
    }

    // Draw gravity strokes
    for (const auto& stroke : m_gravityStrokes) {
        renderer->drawGravityStroke(stroke);
    }

    // Draw current stroke being drawn
    if (m_isDrawingStroke && m_currentStroke.points.size() > 1) {
        renderer->drawGravityStroke(m_currentStroke);
    }

    // Draw physics objects
    for (const auto& obj : physics->getObjects()) {
        renderer->drawPhysicsObject(obj.get());
    }

    // Draw gravity field visualization
    if (!m_gravityStrokes.empty()) {
        // Visual feedback of combined gravity
        float avgAngle = 0;
        for (const auto& stroke : m_gravityStrokes) {
            avgAngle += std::atan2(stroke.direction.y, stroke.direction.x);
        }
        avgAngle /= m_gravityStrokes.size();
        m_game->getHUD()->setCurrentGravityAngle(avgAngle * 180.0f / 3.14159f);
    }
}

void PlayingState::handleInput(const TouchPoint& touch) {
    // Check HUD first
    if (m_game->getHUD()->handleTouch(touch)) {
        return;
    }

    if (touch.isActive) {
        if (!m_isDrawingStroke) {
            // Start new stroke
            m_isDrawingStroke = true;
            m_currentStroke = GravityStroke();
            m_currentStroke.points.push_back(touch.position);
            m_currentStroke.color = Color::cyan();
        } else {
            // Continue stroke
            m_currentStroke.points.push_back(touch.position);
        }
    } else if (m_isDrawingStroke) {
        // Finish stroke
        m_isDrawingStroke = false;

        if (m_currentStroke.points.size() >= 2) {
            // Calculate stroke direction
            Vec2 start = m_currentStroke.points.front();
            Vec2 end = m_currentStroke.points.back();
            Vec2 delta = end - start;
            float distance = delta.length();

            if (distance >= MIN_SWIPE_DISTANCE) {
                m_currentStroke.direction = delta.normalized();
                m_currentStroke.strength = std::min(distance / MAX_SWIPE_DISTANCE, 1.0f) * MAX_GRAVITY_STRENGTH;
                m_currentStroke.lifetime = 0;
                m_currentStroke.maxLifetime = GRAVITY_STROKE_LIFETIME;
                m_currentStroke.isActive = true;

                // Limit active strokes
                if (m_gravityStrokes.size() >= MAX_ACTIVE_STROKES) {
                    m_gravityStrokes.erase(m_gravityStrokes.begin());
                }

                m_gravityStrokes.push_back(m_currentStroke);
                m_game->getHUD()->setStrokeCount(
                    static_cast<int>(m_gravityStrokes.size()),
                    m_game->getLevelManager()->getCurrentLevel()->getMaxStrokes()
                );

                m_game->getAudioManager()->playSound(SoundEffect::GravitySwipe);
            }
        }

        m_currentStroke = GravityStroke();
    }
}

void PlayingState::addGravityStroke(const GravityStroke& stroke) {
    if (m_gravityStrokes.size() >= MAX_ACTIVE_STROKES) {
        m_gravityStrokes.erase(m_gravityStrokes.begin());
    }
    m_gravityStrokes.push_back(stroke);
}

void PlayingState::updateGravityStrokes(float deltaTime) {
    for (auto it = m_gravityStrokes.begin(); it != m_gravityStrokes.end();) {
        it->lifetime += deltaTime;
        if (it->lifetime >= it->maxLifetime) {
            it = m_gravityStrokes.erase(it);
        } else {
            ++it;
        }
    }
}

void PlayingState::checkLevelCompletion() {
    auto* physics = m_game->getPhysicsWorld();
    auto* level = m_game->getLevelManager()->getCurrentLevel();
    
    if (!level || !level->getObjective()) return;

    if (level->getObjective()->isComplete()) {
        m_levelComplete = true;
        
        // Calculate score
        int timeBonus = static_cast<int>((level->getTimeLimit() - m_levelTime) * TIME_BONUS_PER_SECOND);
        if (timeBonus < 0) timeBonus = 0;
        
        int strokeBonus = (level->getMaxStrokes() - static_cast<int>(m_gravityStrokes.size())) * EFFICIENCY_BONUS;
        if (strokeBonus < 0) strokeBonus = 0;

        m_game->addScore(BASE_GOAL_SCORE + timeBonus + strokeBonus);
        m_game->getLevelManager()->completeLevel(m_game->getScore(), m_levelTime);
        m_game->getAudioManager()->playSound(SoundEffect::LevelComplete);
        m_game->changeState(GameStateType::LevelComplete);
    } else if (level->getObjective()->isFailed()) {
        m_game->changeState(GameStateType::GameOver);
    }
}

// PausedState
PausedState::PausedState(Game* game) : GameState(game) {}

void PausedState::enter() {
    auto* hud = m_game->getHUD();
    hud->clearButtons();

    float centerX = m_game->getScreenWidth() / 2.0f;
    float centerY = m_game->getScreenHeight() / 2.0f;
    float buttonWidth = 250.0f;
    float buttonHeight = 60.0f;
    float spacing = 80.0f;

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY - spacing, buttonWidth, buttonHeight),
        "RESUME",
        [this]() { m_game->resumeGame(); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY, buttonWidth, buttonHeight),
        "RESTART",
        [this]() { m_game->restartLevel(); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY + spacing, buttonWidth, buttonHeight),
        "MENU",
        [this]() { m_game->changeState(GameStateType::Menu); }
    );
}

void PausedState::exit() {
    m_game->getHUD()->clearButtons();
}

void PausedState::update(float /*deltaTime*/) {}

void PausedState::render() {
    auto* renderer = m_game->getRenderer();
    
    // Dim overlay
    renderer->drawRect(
        Rect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
             static_cast<float>(m_game->getScreenHeight())),
        Color(0, 0, 0, 180)
    );

    renderer->drawTextCentered(
        "PAUSED",
        Vec2(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.3f),
        Color::white(),
        48.0f
    );
}

void PausedState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// LevelCompleteState
LevelCompleteState::LevelCompleteState(Game* game) : GameState(game) {}

void LevelCompleteState::enter() {
    m_animationTime = 0.0f;
    
    auto* level = m_game->getLevelManager()->getCurrentLevel();
    if (level) {
        m_starsEarned = level->calculateStars(m_game->getScore());
    }

    auto* hud = m_game->getHUD();
    hud->clearButtons();

    float centerX = m_game->getScreenWidth() / 2.0f;
    float buttonY = m_game->getScreenHeight() * 0.7f;
    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    float spacing = 220.0f;

    hud->addButton(
        Rect(centerX - spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "MENU",
        [this]() { m_game->changeState(GameStateType::Menu); }
    );

    hud->addButton(
        Rect(centerX + spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "NEXT",
        [this]() { m_game->nextLevel(); }
    );

    m_game->getAudioManager()->playSound(SoundEffect::StarEarned);
}

void LevelCompleteState::exit() {
    m_game->getHUD()->clearButtons();
}

void LevelCompleteState::update(float deltaTime) {
    m_animationTime += deltaTime;
}

void LevelCompleteState::render() {
    auto* renderer = m_game->getRenderer();
    float centerX = m_game->getScreenWidth() / 2.0f;

    renderer->drawTextCentered(
        "LEVEL COMPLETE!",
        Vec2(centerX, m_game->getScreenHeight() * 0.25f),
        Color::green(),
        48.0f
    );

    // Animated stars
    float starY = m_game->getScreenHeight() * 0.4f;
    float starSpacing = 80.0f;
    for (int i = 0; i < 3; ++i) {
        float starX = centerX + (i - 1) * starSpacing;
        float delay = i * 0.3f;
        float scale = (m_animationTime > delay) ? std::min((m_animationTime - delay) * 3.0f, 1.0f) : 0.0f;
        
        Color starColor = (i < m_starsEarned) ? Color::yellow() : Color(80, 80, 80);
        float radius = 25.0f * scale;
        
        // Draw star shape (simplified as circle for now)
        renderer->drawCircle(Vec2(starX, starY), radius, starColor, true);
    }

    // Score
    renderer->drawTextCentered(
        "Score: " + std::to_string(m_game->getScore()),
        Vec2(centerX, m_game->getScreenHeight() * 0.55f),
        Color::white(),
        36.0f
    );
}

void LevelCompleteState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// GameOverState
GameOverState::GameOverState(Game* game) : GameState(game) {}

void GameOverState::enter() {
    m_fadeIn = 0.0f;

    auto* hud = m_game->getHUD();
    hud->clearButtons();

    float centerX = m_game->getScreenWidth() / 2.0f;
    float buttonY = m_game->getScreenHeight() * 0.6f;
    float buttonWidth = 200.0f;
    float buttonHeight = 60.0f;
    float spacing = 220.0f;

    hud->addButton(
        Rect(centerX - spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "MENU",
        [this]() { m_game->changeState(GameStateType::Menu); }
    );

    hud->addButton(
        Rect(centerX + spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "RETRY",
        [this]() { m_game->restartLevel(); }
    );

    m_game->getAudioManager()->playSound(SoundEffect::GameOver);
}

void GameOverState::exit() {
    m_game->getHUD()->clearButtons();
}

void GameOverState::update(float deltaTime) {
    m_fadeIn = std::min(m_fadeIn + deltaTime * 2.0f, 1.0f);
}

void GameOverState::render() {
    auto* renderer = m_game->getRenderer();

    // Red overlay
    uint8_t alpha = static_cast<uint8_t>(m_fadeIn * 150);
    renderer->drawRect(
        Rect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
             static_cast<float>(m_game->getScreenHeight())),
        Color(80, 0, 0, alpha)
    );

    float centerX = m_game->getScreenWidth() / 2.0f;

    renderer->drawTextCentered(
        "GAME OVER",
        Vec2(centerX, m_game->getScreenHeight() * 0.35f),
        Color::red(),
        48.0f
    );

    renderer->drawTextCentered(
        "Score: " + std::to_string(m_game->getScore()),
        Vec2(centerX, m_game->getScreenHeight() * 0.45f),
        Color::white(),
        30.0f
    );
}

void GameOverState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// TutorialState
TutorialState::TutorialState(Game* game) : GameState(game) {}

void TutorialState::enter() {
    m_tutorialStep = 0;
    m_stepTime = 0.0f;

    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->showTutorialHint("Swipe to create gravity fields!");
}

void TutorialState::exit() {
    m_game->getHUD()->hideTutorialHint();
    m_game->getHUD()->clearButtons();
}

void TutorialState::update(float deltaTime) {
    m_stepTime += deltaTime;

    // Auto-advance tutorial steps
    if (m_stepTime > 5.0f) {
        m_tutorialStep++;
        m_stepTime = 0.0f;

        switch (m_tutorialStep) {
            case 1:
                m_game->getHUD()->showTutorialHint("Guide objects to the green goal zone");
                break;
            case 2:
                m_game->getHUD()->showTutorialHint("Create chain reactions for bonus points!");
                break;
            case 3:
                m_game->changeState(GameStateType::Playing);
                break;
        }
    }
}

void TutorialState::render() {
    auto* renderer = m_game->getRenderer();
    float centerX = m_game->getScreenWidth() / 2.0f;
    float centerY = m_game->getScreenHeight() / 2.0f;

    // Draw tutorial visualization
    switch (m_tutorialStep) {
        case 0: {
            // Show swipe gesture
            float t = std::fmod(m_stepTime, 2.0f) / 2.0f;
            Vec2 start(centerX - 100, centerY);
            Vec2 end(centerX + 100, centerY - 50);
            Vec2 current = Vec2::lerp(start, end, t);
            
            renderer->drawLine(start, current, Color::cyan(), 3.0f);
            renderer->drawCircle(current, 15.0f, Color::cyan(), true);
            break;
        }
        case 1: {
            // Show goal zone
            renderer->drawGoalZone(Rect(centerX - 75, centerY + 100, 150, 100));
            renderer->drawCircle(Vec2(centerX, centerY - 50), 20.0f, Color::white(), true);
            break;
        }
        case 2: {
            // Show chain reaction
            float t = std::fmod(m_stepTime, 1.5f);
            for (int i = 0; i < 3; ++i) {
                float delay = i * 0.3f;
                if (t > delay) {
                    float scale = std::min((t - delay) * 2.0f, 1.0f);
                    Vec2 pos(centerX + (i - 1) * 60, centerY);
                    renderer->drawCircle(pos, 20.0f * scale, Color::orange(), true);
                }
            }
            break;
        }
    }

    renderer->drawTextCentered(
        "TAP TO CONTINUE",
        Vec2(centerX, m_game->getScreenHeight() * 0.85f),
        Color(150, 150, 150),
        20.0f
    );
}

void TutorialState::handleInput(const TouchPoint& touch) {
    if (!touch.isActive) {
        m_tutorialStep++;
        m_stepTime = 0.0f;
        
        if (m_tutorialStep >= 3) {
            m_game->changeState(GameStateType::Playing);
        }
    }
}

} // namespace Vectoria
