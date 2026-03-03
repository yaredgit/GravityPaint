#include "GravityPaint/core/GameState.h"
#include "GravityPaint/core/Game.h"
#include "GravityPaint/core/InputManager.h"
#include "GravityPaint/physics/PhysicsWorld.h"
#include "GravityPaint/physics/PhysicsObject.h"
#include "GravityPaint/graphics/Renderer.h"
#include "GravityPaint/audio/AudioManager.h"
#include "GravityPaint/level/LevelManager.h"
#include "GravityPaint/level/Level.h"
#include "GravityPaint/ui/HUD.h"
#include "GravityPaint/Constants.h"
#include <algorithm>

namespace GravityPaint {

namespace {
float getUiScale(const Game* game) {
    float shortSide = static_cast<float>(std::min(game->getScreenWidth(), game->getScreenHeight()));
    return std::clamp(shortSide / 900.0f, 0.72f, 1.25f);
}
}

// MenuState
MenuState::MenuState(Game* game) : GameState(game) {}

void MenuState::enter() {
    m_titlePulse = 0.0f;
    m_selectedOption = 0;
    
    // Load saved progress
    m_game->loadProgress();
    
    // Stop any playing sounds and play menu music
    m_game->getAudioManager()->stopAllSounds();
    if (m_game->isMusicEnabled()) {
        m_game->getAudioManager()->playSound("menu_music", 0.5f);
    }
    
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->setVisible(true);
    
    Game* game = m_game;
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float startY = m_game->getScreenHeight() * 0.45f;
    float buttonWidth = 300.0f * uiScale;
    float buttonHeight = 65.0f * uiScale;
    float spacing = 80.0f * uiScale;

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY, buttonWidth, buttonHeight),
        "CAMPAIGN",
        [game]() { 
            game->setGameMode(GameMode::Campaign);
            game->changeState(GameStateType::DifficultySelect); 
        }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing, buttonWidth, buttonHeight),
        "ENDLESS",
        [game]() { 
            game->setGameMode(GameMode::Endless);
            game->changeState(GameStateType::DifficultySelect);
        }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 2, buttonWidth, buttonHeight),
        "SETTINGS",
        [game]() { game->changeState(GameStateType::Settings); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 3, buttonWidth, buttonHeight),
        "QUIT",
        []() { SDL_Event quit; quit.type = SDL_QUIT; SDL_PushEvent(&quit); }
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
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float screenH = static_cast<float>(m_game->getScreenHeight());
    
    // Gradient background
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), screenH);
    renderer->drawGradientRect(screenRect, 
                               Color(10, 18, 32), Color(26, 24, 44),
                               Color(8, 14, 24), Color(18, 20, 34));
    renderer->drawStarfield(m_titlePulse);
    renderer->drawGrid(56.0f, Color(70, 110, 140, 28));
    
    // Draw title with pulse effect
    float pulse = 1.0f + 0.08f * std::sin(m_titlePulse);
    renderer->drawTextCentered(
        "GRAVITY PAINT",
        Vec2(centerX, screenH * 0.2f),
        Color(205, 235, 255),
        56.0f * pulse * uiScale
    );

    renderer->drawTextCentered(
        "Swipe to Control Gravity",
        Vec2(centerX, screenH * 0.28f),
        Color(170, 182, 210),
        20.0f * uiScale
    );
    
    // Show high score
    if (m_game->getHighScore() > 0) {
        renderer->drawTextCentered(
            "High Score: " + std::to_string(m_game->getHighScore()),
            Vec2(centerX, screenH * 0.35f),
            Color(250, 220, 120),
            18.0f * uiScale
        );
    }

    // Draw decorative gravity field visualization
    float time = m_titlePulse;
    Vec2 center(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.15f);
    for (int i = 0; i < 8; ++i) {
        float angle = (i / 8.0f) * 3.14159f * 2.0f + time * 0.5f;
        float radius = (80.0f + 20.0f * std::sin(time + i)) * uiScale;
        Vec2 pos = center + Vec2(std::cos(angle), std::sin(angle)) * radius;
        renderer->drawCircle(pos, 10.0f * uiScale, Color(130, 235, 255, 180), false);
    }
}

void MenuState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// PlayingState
PlayingState::PlayingState(Game* game) : GameState(game) {}

void PlayingState::enter() {
    m_gravityStrokes.clear();
    m_particles.clear();
    m_isDrawingStroke = false;
    m_levelTime = 0.0f;
    m_levelComplete = false;

    // Play gameplay music
    m_game->getAudioManager()->stopAllSounds();
    if (m_game->isMusicEnabled()) {
        m_game->getAudioManager()->playSound("gameplay_music", 0.4f);
    }

    auto* levelManager = m_game->getLevelManager();
    auto* physics = m_game->getPhysicsWorld();
    auto* hud = m_game->getHUD();
    
    if (!levelManager || !physics || !hud) {
        return;
    }
    
    hud->clearButtons();
    physics->reset();

    if (!levelManager->getCurrentLevel()) {
        levelManager->loadLevel(1);
    }

    levelManager->startLevel();

    auto* level = levelManager->getCurrentLevel();
    if (!level) {
        return;
    }
    
    physics->createBoundaries(level->getWidth(), level->getHeight());
    physics->createGoalZone(level->getGoalZone().center(), 
                           Vec2(level->getGoalZone().w, level->getGoalZone().h));

    levelManager->spawnObjects(physics);

    hud->setLevelNumber(level->getId());
    hud->setTimeLimit(level->getTimeLimit());
    hud->setObjective(level->getObjective() ? level->getObjective()->getDescription() : "Reach the goal!");
    hud->setStrokeCount(0, level->getMaxStrokes());
    hud->setLives(m_game->getLives(), m_game->getMaxLives());

    hud->setPauseButtonVisible(true);
    hud->setVisible(true);
    
    Game* game = m_game;
    float uiScale = getUiScale(m_game);
    hud->addButton(
        Rect(m_game->getScreenWidth() - (120.0f * uiScale), 50.0f * uiScale, 110.0f * uiScale, 40.0f * uiScale),
        "RESTART",
        [game]() { 
            game->restartLevel();
        }
    );
}

void PlayingState::exit() {
    m_game->getHUD()->clearButtons();
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
    
    // Update objective progress
    levelManager->updateObjectiveProgress(physics);
    
    // Update particles
    updateParticles(deltaTime);
    
    // Check for objects reaching goal and spawn particles
    auto* level = levelManager->getCurrentLevel();
    if (level) {
        static std::vector<int> celebratedObjects;
        for (const auto& obj : physics->getObjects()) {
            if (obj && obj->isActive() && obj->hasReachedGoal()) {
                // Spawn celebration particles when object first reaches goal
                if (std::find(celebratedObjects.begin(), celebratedObjects.end(), obj->getId()) == celebratedObjects.end()) {
                    spawnGoalParticles(obj->getPosition(), obj->getColor());
                    celebratedObjects.push_back(obj->getId());
                    if (celebratedObjects.size() > 100) celebratedObjects.erase(celebratedObjects.begin());
                }
            }
        }
    }

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

    // Draw gradient background
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
                    static_cast<float>(m_game->getScreenHeight()));
    renderer->drawGradientRect(screenRect, 
                               Color(6, 12, 24),
                               Color(10, 20, 38),
                               Color(4, 8, 18),
                               Color(6, 14, 28));

    // Draw background grid
    renderer->drawStarfield(m_levelTime);
    renderer->drawGrid(50.0f, Color(45, 150, 190, 70));

    // Draw goal zone
    if (level) {
        Rect goal = level->getGoalZone();
        renderer->drawGoalZone(goal);
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
    
    // Draw particles
    for (const auto& p : m_particles) {
        if (p.isAlive()) {
            Color c = p.color;
            c.a = static_cast<uint8_t>(p.alpha() * 255);
            renderer->drawCircle(p.position, p.size * p.alpha(), c, true);
        }
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

    if (level->getObjective()->isComplete() && !m_levelComplete) {
        m_levelComplete = true;
        SDL_Log("LEVEL COMPLETE! Changing state...");
        
        // Calculate score
        int timeBonus = static_cast<int>((level->getTimeLimit() - m_levelTime) * TIME_BONUS_PER_SECOND);
        if (timeBonus < 0) timeBonus = 0;
        
        int strokeBonus = (level->getMaxStrokes() - static_cast<int>(m_gravityStrokes.size())) * EFFICIENCY_BONUS;
        if (strokeBonus < 0) strokeBonus = 0;

        m_game->addScore(BASE_GOAL_SCORE + timeBonus + strokeBonus);
        m_game->getLevelManager()->completeLevel(m_game->getScore(), m_levelTime);
        m_game->changeState(GameStateType::LevelComplete);
    } else if (level->getObjective()->isFailed()) {
        // Lose a life but continue if lives remain
        m_game->loseLife();
        if (m_game->getLives() > 0) {
            // Reset level and continue playing
            m_game->getHUD()->showMessage("Try Again!", 1.5f);
            m_game->getLevelManager()->reloadCurrentLevel();
            m_game->getPhysicsWorld()->reset();
            m_game->getLevelManager()->startLevel();
            m_game->getLevelManager()->spawnObjects(m_game->getPhysicsWorld());
            m_game->getHUD()->setLives(m_game->getLives(), m_game->getMaxLives());
            m_levelTime = 0.0f;
            m_gravityStrokes.clear();
        }
        // If lives == 0, loseLife() already changed state to GameOver
    }
}

void PlayingState::updateParticles(float deltaTime) {
    for (auto& p : m_particles) {
        if (p.isAlive()) {
            p.position += p.velocity * deltaTime;
            p.velocity.y += 100.0f * deltaTime; // Gravity
            p.velocity *= 0.98f; // Drag
            p.life -= deltaTime;
            p.rotation += p.rotationSpeed * deltaTime;
        }
    }
    
    // Remove dead particles
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const SimpleParticle& p) { return !p.isAlive(); }),
        m_particles.end()
    );
}

void PlayingState::spawnGoalParticles(const Vec2& position, const Color& color) {
    for (int i = 0; i < 20; ++i) {
        SimpleParticle p;
        p.position = position;
        float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28318f;
        float speed = 100.0f + (static_cast<float>(rand()) / RAND_MAX) * 200.0f;
        p.velocity = Vec2(std::cos(angle) * speed, std::sin(angle) * speed - 150.0f);
        p.color = color;
        p.size = 4.0f + (static_cast<float>(rand()) / RAND_MAX) * 8.0f;
        p.life = 0.5f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f;
        p.maxLife = p.life;
        p.rotationSpeed = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 10.0f;
        m_particles.push_back(p);
    }
}

void PlayingState::spawnCollisionParticles(const Vec2& position, const Color& color) {
    for (int i = 0; i < 8; ++i) {
        SimpleParticle p;
        p.position = position;
        float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28318f;
        float speed = 50.0f + (static_cast<float>(rand()) / RAND_MAX) * 100.0f;
        p.velocity = Vec2(std::cos(angle) * speed, std::sin(angle) * speed);
        p.color = color;
        p.size = 2.0f + (static_cast<float>(rand()) / RAND_MAX) * 4.0f;
        p.life = 0.2f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
        p.maxLife = p.life;
        m_particles.push_back(p);
    }
}

// PausedState
PausedState::PausedState(Game* game) : GameState(game) {}

void PausedState::enter() {
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->setVisible(true);

    Game* game = m_game;
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float centerY = m_game->getScreenHeight() / 2.0f;
    float buttonWidth = 250.0f * uiScale;
    float buttonHeight = 60.0f * uiScale;
    float spacing = 80.0f * uiScale;

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY - spacing, buttonWidth, buttonHeight),
        "RESUME",
        [game]() { game->resumeGame(); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY, buttonWidth, buttonHeight),
        "RESTART",
        [game]() { game->restartLevel(); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, centerY + spacing, buttonWidth, buttonHeight),
        "MENU",
        [game]() { game->changeState(GameStateType::Menu); }
    );
}

void PausedState::exit() {
    m_game->getHUD()->clearButtons();
}

void PausedState::update(float /*deltaTime*/) {}

void PausedState::render() {
    auto* renderer = m_game->getRenderer();
    float uiScale = getUiScale(m_game);
    
    // Dim overlay
    renderer->drawRect(
        Rect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
             static_cast<float>(m_game->getScreenHeight())),
        Color(0, 6, 15, 175)
    );
    renderer->drawGrid(64.0f, Color(60, 115, 145, 24));

    renderer->drawTextCentered(
        "PAUSED",
        Vec2(m_game->getScreenWidth() / 2.0f, m_game->getScreenHeight() * 0.3f),
        Color(170, 245, 255),
        48.0f * uiScale
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
    hud->setVisible(true);

    float centerX = m_game->getScreenWidth() / 2.0f;
    float buttonY = m_game->getScreenHeight() * 0.7f;
    float uiScale = getUiScale(m_game);
    float buttonWidth = 200.0f * uiScale;
    float buttonHeight = 60.0f * uiScale;
    float spacing = 220.0f * uiScale;

    Game* game = m_game;  // Capture game pointer, not 'this'
    
    hud->addButton(
        Rect(centerX - spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "MENU",
        [game]() { game->changeState(GameStateType::Menu); }
    );

    hud->addButton(
        Rect(centerX + spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "NEXT",
        [game]() { game->nextLevel(); }
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
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
                    static_cast<float>(m_game->getScreenHeight()));
    renderer->drawGradientRect(screenRect,
                               Color(10, 20, 36), Color(24, 42, 54),
                               Color(8, 14, 24), Color(16, 30, 42));
    renderer->drawStarfield(m_animationTime);
    renderer->drawGrid(58.0f, Color(90, 130, 150, 24));

    renderer->drawTextCentered(
        "LEVEL COMPLETE!",
        Vec2(centerX, m_game->getScreenHeight() * 0.25f),
        Color(165, 235, 195),
        48.0f * uiScale
    );

    // Animated stars
    float starY = m_game->getScreenHeight() * 0.4f;
    float starSpacing = 80.0f * uiScale;
    for (int i = 0; i < 3; ++i) {
        float starX = centerX + (i - 1) * starSpacing;
        float delay = i * 0.3f;
        float scale = (m_animationTime > delay) ? std::min((m_animationTime - delay) * 3.0f, 1.0f) : 0.0f;
        
        Color starColor = (i < m_starsEarned) ? Color(255, 222, 160) : Color(90, 95, 110);
        float radius = 25.0f * uiScale * scale;
        
        // Draw star shape (simplified as circle for now)
        renderer->drawCircle(Vec2(starX, starY), radius, starColor, true);
    }

    // Score
    renderer->drawTextCentered(
        "Score: " + std::to_string(m_game->getScore()),
        Vec2(centerX, m_game->getScreenHeight() * 0.55f),
        Color(215, 245, 255),
        36.0f * uiScale
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
    hud->setVisible(true);

    Game* game = m_game;
    float centerX = m_game->getScreenWidth() / 2.0f;
    float buttonY = m_game->getScreenHeight() * 0.6f;
    float uiScale = getUiScale(m_game);
    float buttonWidth = 200.0f * uiScale;
    float buttonHeight = 60.0f * uiScale;
    float spacing = 220.0f * uiScale;

    hud->addButton(
        Rect(centerX - spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "MENU",
        [game]() { game->changeState(GameStateType::Menu); }
    );

    hud->addButton(
        Rect(centerX + spacing/2 - buttonWidth/2, buttonY, buttonWidth, buttonHeight),
        "TRY AGAIN",
        [game]() { 
            // Reset lives and restart from current level
            game->resetLives();
            game->resetScore();
            game->getLevelManager()->reloadCurrentLevel();
            game->changeState(GameStateType::Playing);
        }
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
    float uiScale = getUiScale(m_game);
    float screenW = static_cast<float>(m_game->getScreenWidth());
    float screenH = static_cast<float>(m_game->getScreenHeight());

    // Gradient background with red tint
    Rect screenRect(0, 0, screenW, screenH);
    renderer->drawGradientRect(screenRect, 
                               Color(30, 6, 12), Color(46, 10, 20),
                               Color(16, 2, 8), Color(26, 4, 14));
    renderer->drawGrid(55.0f, Color(190, 65, 90, 45));

    // Red overlay fade
    uint8_t alpha = static_cast<uint8_t>(m_fadeIn * 100);
    renderer->drawRect(screenRect, Color(110, 8, 20, alpha));

    float centerX = screenW / 2.0f;

    renderer->drawTextCentered(
        "GAME OVER",
        Vec2(centerX, screenH * 0.25f),
        Color(255, 80, 80),
        52.0f * uiScale
    );

    renderer->drawTextCentered(
        "Out of Lives!",
        Vec2(centerX, screenH * 0.35f),
        Color(225, 165, 175),
        24.0f * uiScale
    );

    renderer->drawTextCentered(
        "Final Score: " + std::to_string(m_game->getScore()),
        Vec2(centerX, screenH * 0.45f),
        Color::white(),
        32.0f * uiScale
    );
    
    // Show high score if beaten
    if (m_game->getScore() >= m_game->getHighScore() && m_game->getScore() > 0) {
        renderer->drawTextCentered(
            "NEW HIGH SCORE!",
            Vec2(centerX, screenH * 0.52f),
            Color(255, 230, 135),
            24.0f * uiScale
        );
    }
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
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float centerY = m_game->getScreenHeight() / 2.0f;

    // Draw tutorial visualization
    switch (m_tutorialStep) {
        case 0: {
            // Show swipe gesture
            float t = std::fmod(m_stepTime, 2.0f) / 2.0f;
            Vec2 start(centerX - 100.0f * uiScale, centerY);
            Vec2 end(centerX + 100.0f * uiScale, centerY - 50.0f * uiScale);
            Vec2 current = Vec2::lerp(start, end, t);
            
            renderer->drawLine(start, current, Color::cyan(), 3.0f * uiScale);
            renderer->drawCircle(current, 15.0f * uiScale, Color::cyan(), true);
            break;
        }
        case 1: {
            // Show goal zone
            renderer->drawGoalZone(Rect(centerX - 75.0f * uiScale, centerY + 100.0f * uiScale, 150.0f * uiScale, 100.0f * uiScale));
            renderer->drawCircle(Vec2(centerX, centerY - 50.0f * uiScale), 20.0f * uiScale, Color::white(), true);
            break;
        }
        case 2: {
            // Show chain reaction
            float t = std::fmod(m_stepTime, 1.5f);
            for (int i = 0; i < 3; ++i) {
                float delay = i * 0.3f;
                if (t > delay) {
                    float scale = std::min((t - delay) * 2.0f, 1.0f);
                    Vec2 pos(centerX + (i - 1) * 60.0f * uiScale, centerY);
                    renderer->drawCircle(pos, 20.0f * uiScale * scale, Color::orange(), true);
                }
            }
            break;
        }
    }

    renderer->drawTextCentered(
        "TAP TO CONTINUE",
        Vec2(centerX, m_game->getScreenHeight() * 0.85f),
        Color(150, 150, 150),
        20.0f * uiScale
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

// DifficultySelectState
DifficultySelectState::DifficultySelectState(Game* game) : GameState(game) {}

void DifficultySelectState::enter() {
    m_animTime = 0.0f;
    
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->setVisible(true);
    
    Game* game = m_game;
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float startY = m_game->getScreenHeight() * 0.35f;
    float buttonWidth = 280.0f * uiScale;
    float buttonHeight = 70.0f * uiScale;
    float spacing = 90.0f * uiScale;
    
    // Lambda to start game based on mode
    auto startWithDifficulty = [game](Difficulty diff) {
        game->setDifficulty(diff);
        game->getLevelManager()->setGameDifficulty(diff);
        
        if (game->getGameMode() == GameMode::Campaign) {
            game->changeState(GameStateType::LevelSelect);
        } else {
            // Endless mode - start at level 1 with infinite progression
            game->resetLives();
            game->resetScore();
            game->getLevelManager()->loadLevel(1);
            game->changeState(GameStateType::Playing);
        }
    };

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY, buttonWidth, buttonHeight),
        "EASY",
        [startWithDifficulty]() { startWithDifficulty(Difficulty::Easy); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing, buttonWidth, buttonHeight),
        "MEDIUM",
        [startWithDifficulty]() { startWithDifficulty(Difficulty::Medium); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 2, buttonWidth, buttonHeight),
        "HARD",
        [startWithDifficulty]() { startWithDifficulty(Difficulty::Hard); }
    );

    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 3.5f, buttonWidth, buttonHeight),
        "BACK",
        [game]() { game->changeState(GameStateType::Menu); }
    );
}

void DifficultySelectState::exit() {
    m_game->getHUD()->clearButtons();
}

void DifficultySelectState::update(float deltaTime) {
    m_animTime += deltaTime;
}

void DifficultySelectState::render() {
    auto* renderer = m_game->getRenderer();
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float screenH = static_cast<float>(m_game->getScreenHeight());

    // Gradient background
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), screenH);
    renderer->drawGradientRect(screenRect, 
                               Color(12, 18, 34), Color(28, 30, 50),
                               Color(8, 12, 24), Color(18, 24, 40));
    renderer->drawStarfield(m_animTime);
    renderer->drawGrid(56.0f, Color(88, 120, 150, 26));

    // Show game mode
    std::string modeText;
    switch (m_game->getGameMode()) {
        case GameMode::Campaign: modeText = "CAMPAIGN MODE"; break;
        case GameMode::Endless: modeText = "ENDLESS MODE"; break;
        case GameMode::TimeAttack: modeText = "TIME ATTACK"; break;
        case GameMode::Zen: modeText = "ZEN MODE"; break;
    }
    renderer->drawTextCentered(modeText, Vec2(centerX, screenH * 0.12f), Color(190, 215, 255), 24.0f * uiScale);

    renderer->drawTextCentered(
        "SELECT DIFFICULTY",
        Vec2(centerX, screenH * 0.2f),
        Color(225, 238, 255),
        36.0f * uiScale
    );
    
    // Mode description
    std::string modeDesc;
    switch (m_game->getGameMode()) {
        case GameMode::Campaign: modeDesc = "Complete 50 levels with increasing challenge"; break;
        case GameMode::Endless: modeDesc = "How far can you go? Levels get harder!"; break;
        case GameMode::TimeAttack: modeDesc = "Race against the clock!"; break;
        case GameMode::Zen: modeDesc = "No timer, no pressure, just play"; break;
    }
    renderer->drawTextCentered(modeDesc, Vec2(centerX, screenH * 0.75f), Color(170, 182, 205), 16.0f * uiScale);
}

void DifficultySelectState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// LevelSelectState
LevelSelectState::LevelSelectState(Game* game) : GameState(game) {}

void LevelSelectState::enter() {
    m_animTime = 0.0f;
    m_selectedLevel = 1;
    
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->setVisible(true);
    
    Game* game = m_game;
    float screenW = static_cast<float>(m_game->getScreenWidth());
    float screenH = static_cast<float>(m_game->getScreenHeight());
    float uiScale = getUiScale(m_game);
    
    // Grid of level buttons (5 columns x 4 rows visible)
    int cols = 5;
    int visibleRows = 4;
    float buttonSize = 70.0f * uiScale;
    float padding = 15.0f * uiScale;
    float gridWidth = cols * (buttonSize + padding) - padding;
    float startX = (screenW - gridWidth) / 2;
    float startY = screenH * 0.25f;
    
    auto* levelManager = m_game->getLevelManager();
    int totalLevels = levelManager->getTotalLevelCount();
    int levelsToShow = std::min(totalLevels, cols * visibleRows);
    
    for (int i = 0; i < levelsToShow; ++i) {
        int levelId = i + 1;
        int col = i % cols;
        int row = i / cols;
        
        float x = startX + col * (buttonSize + padding);
        float y = startY + row * (buttonSize + padding);
        
        bool unlocked = levelManager->isLevelUnlocked(levelId);
        const auto& progress = levelManager->getLevelProgress(levelId);
        
        std::string label = unlocked ? std::to_string(levelId) : "X";
        
        auto* button = hud->addButton(
            Rect(x, y, buttonSize, buttonSize),
            label,
            [game, levelId, unlocked]() {
                if (unlocked) {
                    game->resetLives();  // Fresh start with full lives
                    game->resetScore();
                    game->getLevelManager()->setGameDifficulty(game->getDifficulty());
                    game->getLevelManager()->loadLevel(levelId);
                    game->changeState(GameStateType::Playing);
                }
            }
        );
        
        // Color based on completion status
        if (!unlocked) {
            button->normalColor = Color(52, 54, 64, 190);
            button->hoverColor = Color(64, 66, 78, 205);
            button->pressedColor = Color(78, 80, 92, 215);
        } else if (progress.completed) {
            button->normalColor = Color(70, 112, 96, 210);
            button->hoverColor = Color(88, 136, 118, 220);
            button->pressedColor = Color(98, 150, 130, 228);
        } else {
            button->normalColor = Color(58, 82, 112, 205);
            button->hoverColor = Color(74, 102, 132, 220);
            button->pressedColor = Color(90, 120, 150, 230);
        }
    }
    
    // Back button
    hud->addButton(
        Rect(screenW/2 - 100.0f * uiScale, screenH - 90.0f * uiScale, 200.0f * uiScale, 58.0f * uiScale),
        "BACK",
        [game]() { game->changeState(GameStateType::DifficultySelect); }
    );
}

void LevelSelectState::exit() {
    m_game->getHUD()->clearButtons();
}

void LevelSelectState::update(float deltaTime) {
    m_animTime += deltaTime;
}

void LevelSelectState::render() {
    auto* renderer = m_game->getRenderer();
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float screenH = static_cast<float>(m_game->getScreenHeight());

    // Gradient background
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), screenH);
    renderer->drawGradientRect(screenRect, 
                               Color(10, 18, 34), Color(24, 28, 46),
                               Color(8, 12, 24), Color(16, 20, 36));
    renderer->drawStarfield(m_animTime);
    renderer->drawGrid(56.0f, Color(90, 120, 145, 24));

    renderer->drawTextCentered(
        "SELECT LEVEL",
        Vec2(centerX, screenH * 0.1f),
        Color(225, 238, 255),
        36.0f * uiScale
    );
    
    // Show total stars
    int totalStars = m_game->getLevelManager()->getTotalStars();
    int maxStars = m_game->getLevelManager()->getTotalLevelCount() * 3;
    renderer->drawTextCentered(
        "Stars: " + std::to_string(totalStars) + "/" + std::to_string(maxStars),
        Vec2(centerX, screenH * 0.17f),
        Color(255, 220, 125),
        20.0f * uiScale
    );
}

void LevelSelectState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

// SettingsState
SettingsState::SettingsState(Game* game) : GameState(game) {}

void SettingsState::enter() {
    m_animTime = 0.0f;
    
    auto* hud = m_game->getHUD();
    hud->clearButtons();
    hud->setVisible(true);
    
    Game* game = m_game;
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;
    float startY = m_game->getScreenHeight() * 0.3f;
    float buttonWidth = 280.0f * uiScale;
    float buttonHeight = 60.0f * uiScale;
    float spacing = 80.0f * uiScale;

    // Sound toggle
    std::string soundText = game->isSoundEnabled() ? "SOUND: ON" : "SOUND: OFF";
    hud->addButton(
        Rect(centerX - buttonWidth/2, startY, buttonWidth, buttonHeight),
        soundText,
        [game]() {
            game->setSoundEnabled(!game->isSoundEnabled());
            // Refresh the state to update button text
            game->changeState(GameStateType::Settings);
        }
    );

    // Music toggle
    std::string musicText = game->isMusicEnabled() ? "MUSIC: ON" : "MUSIC: OFF";
    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing, buttonWidth, buttonHeight),
        musicText,
        [game]() {
            bool newState = !game->isMusicEnabled();
            game->setMusicEnabled(newState);
            // Stop currently playing music if disabled
            if (!newState) {
                game->getAudioManager()->stopAllSounds();
            }
            game->changeState(GameStateType::Settings);
        }
    );

    // Reset progress
    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 2, buttonWidth, buttonHeight),
        "RESET PROGRESS",
        [game]() {
            // Reset would need confirmation in a real game
            game->getLevelManager()->initialize(game->getScreenWidth(), game->getScreenHeight());
            game->changeState(GameStateType::Settings);
        }
    );

    // Back button
    hud->addButton(
        Rect(centerX - buttonWidth/2, startY + spacing * 3.5f, buttonWidth, buttonHeight),
        "BACK",
        [game]() { 
            game->saveProgress();
            game->changeState(GameStateType::Menu); 
        }
    );
}

void SettingsState::exit() {
    m_game->getHUD()->clearButtons();
}

void SettingsState::update(float deltaTime) {
    m_animTime += deltaTime;
}

void SettingsState::render() {
    auto* renderer = m_game->getRenderer();
    float uiScale = getUiScale(m_game);
    float centerX = m_game->getScreenWidth() / 2.0f;

    // Gradient background
    Rect screenRect(0, 0, static_cast<float>(m_game->getScreenWidth()), 
                    static_cast<float>(m_game->getScreenHeight()));
    renderer->drawGradientRect(screenRect, 
                               Color(12, 18, 30), Color(24, 26, 40),
                               Color(8, 12, 22), Color(16, 18, 30));
    renderer->drawStarfield(m_animTime);
    renderer->drawGrid(56.0f, Color(90, 120, 145, 24));

    renderer->drawTextCentered(
        "SETTINGS",
        Vec2(centerX, m_game->getScreenHeight() * 0.15f),
        Color(225, 238, 255),
        42.0f * uiScale
    );
}

void SettingsState::handleInput(const TouchPoint& touch) {
    m_game->getHUD()->handleTouch(touch);
}

} // namespace GravityPaint
