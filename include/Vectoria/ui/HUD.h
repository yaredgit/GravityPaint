#pragma once

#include "Vectoria/Types.h"
#include <string>
#include <vector>
#include <functional>

namespace Vectoria {

class Renderer;

struct UIButton {
    Rect bounds;
    std::string text;
    Color normalColor;
    Color hoverColor;
    Color pressedColor;
    std::function<void()> onClick;
    bool isHovered = false;
    bool isPressed = false;
    bool isVisible = true;
};

class HUD {
public:
    HUD(int screenWidth, int screenHeight);
    ~HUD() = default;

    void update(float deltaTime);
    void render(Renderer* renderer);

    // Touch handling
    bool handleTouch(const TouchPoint& touch);

    // Score display
    void setScore(int score);
    void setHighScore(int highScore);
    void setCombo(int combo);
    void addScorePopup(const Vec2& position, int points);

    // Level info
    void setLevelNumber(int level);
    void setLevelTime(float time);
    void setTimeLimit(float limit);
    void setObjective(const std::string& objective);
    void setProgress(float progress);  // 0-1

    // Gravity vector display
    void setCurrentGravityAngle(float angle);
    void setGravityStrength(float strength);
    void setStrokeCount(int current, int max);

    // Stars/rating
    void setStars(int stars, int maxStars = 3);

    // Messages
    void showMessage(const std::string& message, float duration = 2.0f);
    void showTutorialHint(const std::string& hint);
    void hideTutorialHint();

    // Buttons
    UIButton* addButton(const Rect& bounds, const std::string& text, std::function<void()> onClick);
    void removeButton(UIButton* button);
    void clearButtons();
    void setPauseButtonVisible(bool visible);

    // State
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    void reset();

private:
    void renderScore(Renderer* renderer);
    void renderLevelInfo(Renderer* renderer);
    void renderGravityIndicator(Renderer* renderer);
    void renderProgress(Renderer* renderer);
    void renderStars(Renderer* renderer);
    void renderMessage(Renderer* renderer);
    void renderButtons(Renderer* renderer);
    void renderScorePopups(Renderer* renderer);
    void updateScorePopups(float deltaTime);

    int m_screenWidth;
    int m_screenHeight;

    // Score
    int m_score = 0;
    int m_displayedScore = 0;
    int m_highScore = 0;
    int m_combo = 0;
    float m_comboAnimTime = 0.0f;

    // Level
    int m_levelNumber = 1;
    float m_levelTime = 0.0f;
    float m_timeLimit = 0.0f;
    std::string m_objective;
    float m_progress = 0.0f;

    // Gravity
    float m_gravityAngle = 0.0f;
    float m_gravityStrength = 0.0f;
    int m_strokeCount = 0;
    int m_maxStrokes = MAX_ACTIVE_STROKES;

    // Stars
    int m_stars = 0;
    int m_maxStars = 3;

    // Message
    std::string m_message;
    float m_messageTimer = 0.0f;
    float m_messageDuration = 0.0f;

    // Tutorial
    std::string m_tutorialHint;
    bool m_showTutorialHint = false;

    // Buttons
    std::vector<std::unique_ptr<UIButton>> m_buttons;
    UIButton* m_pauseButton = nullptr;

    // Score popups
    struct ScorePopup {
        Vec2 position;
        int points;
        float lifetime;
        float alpha;
    };
    std::vector<ScorePopup> m_scorePopups;

    bool m_visible = true;
};

} // namespace Vectoria
