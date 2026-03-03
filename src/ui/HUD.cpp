#include "GravityPaint/ui/HUD.h"
#include "GravityPaint/graphics/Renderer.h"
#include "GravityPaint/Constants.h"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace GravityPaint {

HUD::HUD(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    setScreenSize(screenWidth, screenHeight);
    m_pauseButton = addButton(Rect(), "||", nullptr);
    updatePauseButtonLayout();
}

void HUD::setScreenSize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    float shortSide = static_cast<float>(std::min(m_screenWidth, m_screenHeight));
    m_uiScale = std::clamp(shortSide / 900.0f, 0.72f, 1.25f);
    updatePauseButtonLayout();
}

void HUD::updatePauseButtonLayout() {
    if (!m_pauseButton) {
        return;
    }

    float buttonSize = scaled(50.0f);
    float pad = scaled(HUD_PADDING);
    m_pauseButton->bounds = Rect(m_screenWidth - buttonSize - pad, pad, buttonSize, buttonSize);
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
    renderLives(renderer);
    renderGravityIndicator(renderer);
    renderProgress(renderer);
    renderMessage(renderer);
    renderButtons(renderer);
    renderScorePopups(renderer);

    if (m_showTutorialHint) {
        float panelWidth = std::min(static_cast<float>(m_screenWidth) - scaled(40.0f), scaled(560.0f));
        float panelHeight = scaled(48.0f);
        float panelX = (m_screenWidth - panelWidth) * 0.5f;
        float panelY = m_screenHeight - scaled(130.0f);
        Rect hintPanel(
            panelX,
            panelY,
            panelWidth,
            panelHeight
        );
        renderer->drawGradientRect(
            hintPanel,
            Color(20, 40, 70, 185),
            Color(30, 65, 100, 190),
            Color(10, 20, 35, 180),
            Color(20, 35, 55, 185)
        );
        renderer->drawRect(hintPanel, Color(120, 235, 255, 210), false);
        renderer->drawTextCentered(
            m_tutorialHint,
            Vec2(m_screenWidth / 2.0f, panelY + panelHeight * 0.6f),
            Color(180, 250, 255),
            scaled(22.0f)
        );
    }
}

bool HUD::handleTouch(const TouchPoint& touch) {
    if (!m_visible) {
        return false;
    }

    // Store callback to execute after loop (avoid iterator invalidation)
    std::function<void()> pendingCallback = nullptr;

    for (auto& button : m_buttons) {
        if (!button->isVisible) continue;

        bool inside = button->bounds.contains(touch.position);
        
        if (touch.isActive) {
            // Mouse/touch is down
            if (inside) {
                button->isPressed = true;
                button->isHovered = true;
            }
        } else {
            // Mouse/touch released - check if this was a click
            if (inside && button->isPressed && button->onClick) {
                pendingCallback = button->onClick;
            }
            button->isPressed = false;
            button->isHovered = inside;
        }
    }

    // Execute callback after loop is done
    if (pendingCallback) {
        // Play click sound
        if (m_clickSoundCallback) {
            m_clickSoundCallback();
        }
        pendingCallback();
        return true;
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

void HUD::setLives(int lives, int maxLives) {
    m_lives = lives;
    m_maxLives = maxLives;
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
    button->normalColor = Color(52, 64, 90, 200);
    button->hoverColor = Color(72, 94, 128, 225);
    button->pressedColor = Color(95, 116, 150, 240);

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
    float pad = scaled(HUD_PADDING);
    Rect panel(pad - scaled(10.0f), pad - scaled(8.0f), scaled(260.0f), scaled(112.0f));
    renderer->drawGradientRect(
        panel,
        Color(24, 40, 68, 185),
        Color(34, 65, 92, 195),
        Color(12, 20, 32, 175),
        Color(20, 35, 56, 185)
    );
    renderer->drawRect(panel, Color(176, 210, 255, 180), false);

    // Score display
    std::stringstream ss;
    ss << "SCORE: " << m_displayedScore;
    renderer->drawText(ss.str(), Vec2(pad + scaled(2.0f), pad + scaled(1.0f)), Color(200, 250, 255), scaled(28.0f));

    // High score
    if (m_highScore > 0) {
        ss.str("");
        ss << "BEST: " << m_highScore;
        renderer->drawText(ss.str(), Vec2(pad + scaled(2.0f), pad + scaled(35.0f)), Color(140, 185, 200), scaled(20.0f));
    }

    // Combo display
    if (m_combo > 1) {
        float scale = 1.0f + 0.3f * std::sin(m_comboAnimTime * 10.0f);
        ss.str("");
        ss << "x" << m_combo;
        renderer->drawText(
            ss.str(),
            Vec2(pad + scaled(2.0f), pad + scaled(70.0f)),
            Color(255, 220, 120),
            scaled(32.0f) * scale
        );
    }
}

void HUD::renderLevelInfo(Renderer* renderer) {
    float centerX = m_screenWidth / 2.0f;
    float pad = scaled(HUD_PADDING);
    Rect panel(centerX - scaled(190.0f), pad - scaled(10.0f), scaled(380.0f), scaled(110.0f));
    renderer->drawGradientRect(
        panel,
        Color(28, 44, 70, 175),
        Color(24, 58, 86, 180),
        Color(12, 20, 34, 165),
        Color(18, 32, 50, 170)
    );
    renderer->drawRect(panel, Color(170, 202, 245, 165), false);

    // Level number
    std::stringstream ss;
    ss << "LEVEL " << m_levelNumber;
    renderer->drawTextCentered(ss.str(), Vec2(centerX, pad + scaled(15.0f)), Color(200, 245, 255), scaled(24.0f));

    // Time
    float remainingTime = std::max(0.0f, m_timeLimit - m_levelTime);
    int seconds = static_cast<int>(remainingTime);
    int tenths = static_cast<int>((remainingTime - seconds) * 10);
    
    ss.str("");
    ss << seconds << "." << tenths;
    
    Color timeColor = (remainingTime < 10.0f) ? Color(255, 110, 110) : Color(205, 245, 255);
    renderer->drawTextCentered(ss.str(), Vec2(centerX, pad + scaled(45.0f)), timeColor, scaled(32.0f));

    // Stroke count
    ss.str("");
    ss << "STROKES: " << m_strokeCount << "/" << m_maxStrokes;
    renderer->drawTextCentered(ss.str(), Vec2(centerX, pad + scaled(80.0f)), Color(160, 205, 220), scaled(18.0f));
}

void HUD::renderLives(Renderer* renderer) {
    // Draw hearts/lives in top right area (left of pause button)
    float pauseButtonWidth = scaled(60.0f); // Space for pause button
    float lifeSpacing = scaled(30.0f);
    float startX = m_screenWidth - scaled(HUD_PADDING) - pauseButtonWidth - (m_maxLives * (lifeSpacing + scaled(5.0f)));
    float y = scaled(HUD_PADDING) + scaled(15.0f);
    
    for (int i = 0; i < m_maxLives; ++i) {
        float x = startX + i * lifeSpacing;
        Color heartColor = (i < m_lives) ? Color(255, 110, 150) : Color(50, 65, 82, 170);
        
        // Simple heart shape using circles
        renderer->drawCircle(Vec2(x, y + scaled(5.0f)), scaled(8.0f), heartColor, true);
        renderer->drawCircle(Vec2(x + scaled(10.0f), y + scaled(5.0f)), scaled(8.0f), heartColor, true);
        // Triangle bottom (approximate with filled rectangle for simplicity)
        renderer->drawTriangle(
            Vec2(x - scaled(7.0f), y + scaled(5.0f)),
            Vec2(x + scaled(17.0f), y + scaled(5.0f)),
            Vec2(x + scaled(5.0f), y + scaled(22.0f)),
            heartColor, true
        );
    }
}

void HUD::renderGravityIndicator(Renderer* renderer) {
    // Gravity direction indicator (compass-like)
    Vec2 indicatorCenter(m_screenWidth - scaled(80.0f), scaled(150.0f));
    float indicatorRadius = scaled(40.0f);

    // Background and glow rings
    renderer->drawCircle(indicatorCenter, indicatorRadius + 8.0f, Color(140, 170, 245, 38), true);
    renderer->drawCircle(indicatorCenter, indicatorRadius, Color(18, 35, 52, 190), true);
    renderer->drawCircle(indicatorCenter, indicatorRadius, Color(170, 210, 255, 185), false);
    renderer->drawCircle(indicatorCenter, indicatorRadius * 0.7f, Color(125, 165, 220, 130), false);

    // Direction arrow
    float angleRad = m_gravityAngle * 3.14159f / 180.0f;
    Vec2 direction(std::cos(angleRad), std::sin(angleRad));
    renderer->drawVector(indicatorCenter, direction, indicatorRadius * 0.8f, Color(210, 230, 255));

    // Label
    renderer->drawTextCentered("GRAVITY", Vec2(indicatorCenter.x, indicatorCenter.y + indicatorRadius + scaled(15.0f)), 
                               Color(135, 190, 205), scaled(14.0f));
}

void HUD::renderProgress(Renderer* renderer) {
    float pad = scaled(HUD_PADDING);
    float barWidth = m_screenWidth - pad * 2;
    float barHeight = scaled(10.0f);
    float barY = m_screenHeight - pad - barHeight;

    Rect barRect(pad, barY, barWidth, barHeight);
    renderer->drawGradientRect(
        barRect,
        Color(20, 30, 45, 200),
        Color(30, 45, 65, 205),
        Color(10, 18, 28, 195),
        Color(14, 24, 35, 198)
    );

    // Progress fill
    Rect fillRect(pad, barY, barWidth * m_progress, barHeight);
    renderer->drawGradientRect(
        fillRect,
        Color(70, 200, 230, 220),
        Color(140, 255, 255, 230),
        Color(20, 125, 165, 215),
        Color(70, 200, 235, 220)
    );

    // Border
    renderer->drawRect(
        barRect,
        Color(110, 220, 255),
        false
    );

    // Objective text
    if (!m_objective.empty()) {
        renderer->drawTextCentered(
            m_objective,
            Vec2(m_screenWidth / 2.0f, barY - scaled(25.0f)),
            Color(170, 215, 230),
            scaled(18.0f)
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
    Color messageColor = Color(220, 250, 255, static_cast<uint8_t>(alpha * 255));
    float msgWidth = std::min(static_cast<float>(m_screenWidth) - scaled(60.0f), scaled(520.0f));
    float msgHeight = scaled(72.0f);
    float msgX = (m_screenWidth - msgWidth) * 0.5f;
    float msgY = m_screenHeight * 0.5f - msgHeight * 0.5f;
    Rect msgRect(
        msgX,
        msgY,
        msgWidth,
        msgHeight
    );
    renderer->drawGradientRect(
        msgRect,
        Color(26, 40, 62, static_cast<uint8_t>(alpha * 190)),
        Color(30, 65, 90, static_cast<uint8_t>(alpha * 195)),
        Color(10, 18, 30, static_cast<uint8_t>(alpha * 180)),
        Color(20, 35, 52, static_cast<uint8_t>(alpha * 185))
    );
    renderer->drawRect(msgRect, Color(140, 235, 255, static_cast<uint8_t>(alpha * 220)), false);

    renderer->drawTextCentered(
        m_message,
        Vec2(m_screenWidth / 2.0f, msgY + msgHeight * 0.52f),
        messageColor,
        scaled(36.0f)
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

        Rect glowRect(button->bounds.x - 3, button->bounds.y - 3, button->bounds.w + 6, button->bounds.h + 6);
        renderer->drawRect(glowRect, Color(160, 195, 255, button->isPressed ? 55 : 35), true);
        renderer->drawGradientRect(
            button->bounds,
            Color(
                static_cast<uint8_t>(std::min(255, color.r + 20)),
                static_cast<uint8_t>(std::min(255, color.g + 20)),
                static_cast<uint8_t>(std::min(255, color.b + 25)),
                color.a
            ),
            Color(
                static_cast<uint8_t>(std::min(255, color.r + 35)),
                static_cast<uint8_t>(std::min(255, color.g + 40)),
                static_cast<uint8_t>(std::min(255, color.b + 45)),
                color.a
            ),
            Color(
                static_cast<uint8_t>(color.r * 0.65f),
                static_cast<uint8_t>(color.g * 0.65f),
                static_cast<uint8_t>(color.b * 0.7f),
                color.a
            ),
            Color(
                static_cast<uint8_t>(color.r * 0.8f),
                static_cast<uint8_t>(color.g * 0.8f),
                static_cast<uint8_t>(color.b * 0.85f),
                color.a
            )
        );
        renderer->drawRect(button->bounds, Color(205, 224, 255), false);

        Vec2 textPos = button->bounds.center();
        float buttonTextSize = std::clamp(button->bounds.h * 0.42f, scaled(18.0f), scaled(30.0f));
        renderer->drawTextCentered(button->text, textPos, Color(225, 250, 255), buttonTextSize);
    }
}

void HUD::renderScorePopups(Renderer* renderer) {
    for (const auto& popup : m_scorePopups) {
        Color color = (popup.points > 0) ? Color(120, 255, 190) : Color(255, 120, 120);
        color.a = static_cast<uint8_t>(popup.alpha * 255);

        std::string text = (popup.points > 0 ? "+" : "") + std::to_string(popup.points);
        renderer->drawTextCentered(text, popup.position, color, scaled(24.0f));
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

} // namespace GravityPaint
