#include "Vectoria/ui/HUD.h"
#include "Vectoria/graphics/Renderer.h"
#include "Vectoria/Constants.h"
#include <cmath>
#include <sstream>

namespace Vectoria {

HUD::HUD(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    // Create pause button
    float buttonSize = 50.0f;
    m_pauseButton = addButton(
        Rect(m_screenWidth - buttonSize - HUD_PADDING, HUD_PADDING, buttonSize, buttonSize),
        "||",
        nullptr  // Set externally
    );
}

void HUD::update(float deltaTime) {
    // Animate score display
    if (m_displayedScore < m_score) {
        int diff = m_score - m_displayedScore;
        m_displayedScore += std::max(1, diff / 10);
        if (m_displayedScore > m_score) m_displayedScore = m_score;
    }

    // Update combo animation
    if (m_combo > 0) {
        m_comboAnimTime += deltaTime;
    }

    // Update message timer
    if (m_messageTimer > 0) {
        m_messageTimer -= deltaTime;
    }

    // Update score popups
    updateScorePopups(deltaTime);
}

void HUD::render(Renderer* renderer) {
    if (!m_visible) return;

    renderScore(renderer);
    renderLevelInfo(renderer);
    renderGravityIndicator(renderer);
    renderProgress(renderer);
    renderMessage(renderer);
    renderButtons(renderer);
    renderScorePopups(renderer);

    if (m_showTutorialHint) {
        renderer->drawTextCentered(
            m_tutorialHint,
            Vec2(m_screenWidth / 2.0f, m_screenHeight - 100),
            Color::cyan(),
            24.0f
        );
    }
}

bool HUD::handleTouch(const TouchPoint& touch) {
    if (!m_visible) return false;

    for (auto& button : m_buttons) {
        if (!button->isVisible) continue;

        if (button->bounds.contains(touch.position)) {
            if (touch.isActive) {
                button->isPressed = true;
                button->isHovered = true;
            } else if (button->isPressed) {
                button->isPressed = false;
                if (button->onClick) {
                    button->onClick();
                }
                return true;
            }
        } else {
            button->isPressed = false;
            button->isHovered = false;
        }
    }

    return false;
}

void HUD::setScore(int score) {
    m_score = score;
}

void HUD::setHighScore(int highScore) {
    m_highScore = highScore;
}

void HUD::setCombo(int combo) {
    if (combo > m_combo) {
        m_comboAnimTime = 0;
    }
    m_combo = combo;
}

void HUD::addScorePopup(const Vec2& position, int points) {
    ScorePopup popup;
    popup.position = position;
    popup.points = points;
    popup.lifetime = 0;
    popup.alpha = 1.0f;
    m_scorePopups.push_back(popup);
}

void HUD::setLevelNumber(int level) {
    m_levelNumber = level;
}

void HUD::setLevelTime(float time) {
    m_levelTime = time;
}

void HUD::setTimeLimit(float limit) {
    m_timeLimit = limit;
}

void HUD::setObjective(const std::string& objective) {
    m_objective = objective;
}

void HUD::setProgress(float progress) {
    m_progress = std::clamp(progress, 0.0f, 1.0f);
}

void HUD::setCurrentGravityAngle(float angle) {
    m_gravityAngle = angle;
}

void HUD::setGravityStrength(float strength) {
    m_gravityStrength = strength;
}

void HUD::setStrokeCount(int current, int max) {
    m_strokeCount = current;
    m_maxStrokes = max;
}

void HUD::setStars(int stars, int maxStars) {
    m_stars = stars;
    m_maxStars = maxStars;
}

void HUD::showMessage(const std::string& message, float duration) {
    m_message = message;
    m_messageDuration = duration;
    m_messageTimer = duration;
}

void HUD::showTutorialHint(const std::string& hint) {
    m_tutorialHint = hint;
    m_showTutorialHint = true;
}

void HUD::hideTutorialHint() {
    m_showTutorialHint = false;
}

UIButton* HUD::addButton(const Rect& bounds, const std::string& text, std::function<void()> onClick) {
    auto button = std::make_unique<UIButton>();
    button->bounds = bounds;
    button->text = text;
    button->onClick = onClick;
    button->normalColor = Color(60, 60, 80, 200);
    button->hoverColor = Color(80, 80, 120, 220);
    button->pressedColor = Color(100, 100, 150, 240);

    UIButton* ptr = button.get();
    m_buttons.push_back(std::move(button));
    return ptr;
}

void HUD::removeButton(UIButton* button) {
    for (auto it = m_buttons.begin(); it != m_buttons.end(); ++it) {
        if (it->get() == button) {
            m_buttons.erase(it);
            return;
        }
    }
}

void HUD::clearButtons() {
    m_buttons.clear();
    m_pauseButton = nullptr;
}

void HUD::setPauseButtonVisible(bool visible) {
    if (m_pauseButton) {
        m_pauseButton->isVisible = visible;
    }
}

void HUD::reset() {
    m_score = 0;
    m_displayedScore = 0;
    m_combo = 0;
    m_comboAnimTime = 0;
    m_levelTime = 0;
    m_progress = 0;
    m_strokeCount = 0;
    m_scorePopups.clear();
    m_messageTimer = 0;
    m_showTutorialHint = false;
}

void HUD::renderScore(Renderer* renderer) {
    // Score display
    std::stringstream ss;
    ss << "SCORE: " << m_displayedScore;
    renderer->drawText(ss.str(), Vec2(HUD_PADDING, HUD_PADDING), Color::white(), 28.0f);

    // High score
    if (m_highScore > 0) {
        ss.str("");
        ss << "BEST: " << m_highScore;
        renderer->drawText(ss.str(), Vec2(HUD_PADDING, HUD_PADDING + 35), Color(150, 150, 150), 20.0f);
    }

    // Combo display
    if (m_combo > 1) {
        float scale = 1.0f + 0.3f * std::sin(m_comboAnimTime * 10.0f);
        ss.str("");
        ss << "x" << m_combo;
        renderer->drawText(
            ss.str(),
            Vec2(HUD_PADDING, HUD_PADDING + 70),
            Color::yellow(),
            32.0f * scale
        );
    }
}

void HUD::renderLevelInfo(Renderer* renderer) {
    float centerX = m_screenWidth / 2.0f;

    // Level number
    std::stringstream ss;
    ss << "LEVEL " << m_levelNumber;
    renderer->drawTextCentered(ss.str(), Vec2(centerX, HUD_PADDING + 15), Color::white(), 24.0f);

    // Time
    float remainingTime = std::max(0.0f, m_timeLimit - m_levelTime);
    int seconds = static_cast<int>(remainingTime);
    int tenths = static_cast<int>((remainingTime - seconds) * 10);
    
    ss.str("");
    ss << seconds << "." << tenths;
    
    Color timeColor = (remainingTime < 10.0f) ? Color::red() : Color::white();
    renderer->drawTextCentered(ss.str(), Vec2(centerX, HUD_PADDING + 45), timeColor, 32.0f);

    // Stroke count
    ss.str("");
    ss << "STROKES: " << m_strokeCount << "/" << m_maxStrokes;
    renderer->drawTextCentered(ss.str(), Vec2(centerX, HUD_PADDING + 80), Color(180, 180, 200), 18.0f);
}

void HUD::renderGravityIndicator(Renderer* renderer) {
    // Gravity direction indicator (compass-like)
    Vec2 indicatorCenter(m_screenWidth - 80, 150);
    float indicatorRadius = 40.0f;

    // Background circle
    renderer->drawCircle(indicatorCenter, indicatorRadius, Color(40, 40, 60, 180), true);
    renderer->drawCircle(indicatorCenter, indicatorRadius, Color(100, 100, 140), false);

    // Direction arrow
    float angleRad = m_gravityAngle * 3.14159f / 180.0f;
    Vec2 direction(std::cos(angleRad), std::sin(angleRad));
    renderer->drawVector(indicatorCenter, direction, indicatorRadius * 0.8f, Color::cyan());

    // Label
    renderer->drawTextCentered("GRAVITY", Vec2(indicatorCenter.x, indicatorCenter.y + indicatorRadius + 15), 
                               Color(150, 150, 150), 14.0f);
}

void HUD::renderProgress(Renderer* renderer) {
    float barWidth = m_screenWidth - HUD_PADDING * 2;
    float barHeight = 8.0f;
    float barY = m_screenHeight - HUD_PADDING - barHeight;

    // Background
    renderer->drawRect(
        Rect(HUD_PADDING, barY, barWidth, barHeight),
        Color(40, 40, 60, 180),
        true
    );

    // Progress fill
    renderer->drawRect(
        Rect(HUD_PADDING, barY, barWidth * m_progress, barHeight),
        Color::green(),
        true
    );

    // Border
    renderer->drawRect(
        Rect(HUD_PADDING, barY, barWidth, barHeight),
        Color(100, 100, 140),
        false
    );

    // Objective text
    if (!m_objective.empty()) {
        renderer->drawTextCentered(
            m_objective,
            Vec2(m_screenWidth / 2.0f, barY - 25),
            Color(180, 180, 200),
            18.0f
        );
    }
}

void HUD::renderStars(Renderer* renderer) {
    float starSize = 25.0f;
    float spacing = 35.0f;
    float startX = m_screenWidth / 2.0f - (m_maxStars - 1) * spacing / 2.0f;
    float y = 200.0f;

    for (int i = 0; i < m_maxStars; ++i) {
        float x = startX + i * spacing;
        Color starColor = (i < m_stars) ? Color::yellow() : Color(60, 60, 80);
        renderer->drawCircle(Vec2(x, y), starSize, starColor, true);
    }
}

void HUD::renderMessage(Renderer* renderer) {
    if (m_messageTimer <= 0 || m_message.empty()) return;

    float alpha = std::min(m_messageTimer / 0.5f, 1.0f); // Fade out in last 0.5s
    Color messageColor = Color(255, 255, 255, static_cast<uint8_t>(alpha * 255));

    renderer->drawTextCentered(
        m_message,
        Vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f),
        messageColor,
        36.0f
    );
}

void HUD::renderButtons(Renderer* renderer) {
    for (const auto& button : m_buttons) {
        if (!button->isVisible) continue;

        Color color = button->normalColor;
        if (button->isPressed) {
            color = button->pressedColor;
        } else if (button->isHovered) {
            color = button->hoverColor;
        }

        renderer->drawRect(button->bounds, color, true);
        renderer->drawRect(button->bounds, Color(150, 150, 180), false);

        Vec2 textPos = button->bounds.center();
        renderer->drawTextCentered(button->text, textPos, Color::white(), 24.0f);
    }
}

void HUD::renderScorePopups(Renderer* renderer) {
    for (const auto& popup : m_scorePopups) {
        Color color = (popup.points > 0) ? Color::green() : Color::red();
        color.a = static_cast<uint8_t>(popup.alpha * 255);

        std::string text = (popup.points > 0 ? "+" : "") + std::to_string(popup.points);
        renderer->drawTextCentered(text, popup.position, color, 24.0f);
    }
}

void HUD::updateScorePopups(float deltaTime) {
    for (auto it = m_scorePopups.begin(); it != m_scorePopups.end();) {
        it->lifetime += deltaTime;
        it->position.y -= 50.0f * deltaTime; // Float upward
        it->alpha = std::max(0.0f, 1.0f - it->lifetime / 1.0f);

        if (it->lifetime >= 1.0f) {
            it = m_scorePopups.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace Vectoria
