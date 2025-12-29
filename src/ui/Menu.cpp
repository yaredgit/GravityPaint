#include "Vectoria/ui/Menu.h"
#include "Vectoria/graphics/Renderer.h"
#include "Vectoria/Constants.h"
#include <cmath>

namespace Vectoria {

Menu::Menu(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    m_position = Vec2(screenWidth / 2.0f, screenHeight / 2.0f);
}

void Menu::update(float deltaTime) {
    m_titlePulse += deltaTime * 2.0f;

    // Animation
    if (m_animating) {
        if (m_visible) {
            m_animProgress = std::min(1.0f, m_animProgress + deltaTime * 4.0f);
        } else {
            m_animProgress = std::max(0.0f, m_animProgress - deltaTime * 4.0f);
        }

        if ((m_visible && m_animProgress >= 1.0f) || (!m_visible && m_animProgress <= 0.0f)) {
            m_animating = false;
        }
    }

    // Update item animations
    for (size_t i = 0; i < m_items.size(); ++i) {
        float targetOffset = (i == static_cast<size_t>(m_selectedIndex)) ? 20.0f : 0.0f;
        m_items[i].animOffset += (targetOffset - m_items[i].animOffset) * deltaTime * 10.0f;
    }
}

void Menu::render(Renderer* renderer) {
    if (m_animProgress <= 0.0f) return;

    renderBackground(renderer);
    renderTitle(renderer);
    renderItems(renderer);
}

bool Menu::handleTouch(const TouchPoint& touch) {
    if (!m_visible || m_animating) return false;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (!m_items[i].enabled) continue;

        Rect bounds = getItemBounds(static_cast<int>(i));
        if (bounds.contains(touch.position)) {
            if (!touch.isActive) {
                // Touch released on item
                m_selectedIndex = static_cast<int>(i);
                if (m_items[i].action) {
                    m_items[i].action();
                }
                return true;
            } else {
                m_selectedIndex = static_cast<int>(i);
            }
        }
    }

    return false;
}

void Menu::addItem(const std::string& text, std::function<void()> action) {
    MenuItem item;
    item.text = text;
    item.action = action;
    m_items.push_back(item);
}

void Menu::clearItems() {
    m_items.clear();
    m_selectedIndex = 0;
}

void Menu::setSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        m_selectedIndex = index;
    }
}

void Menu::show() {
    m_visible = true;
    m_animating = true;
}

void Menu::hide() {
    m_visible = false;
    m_animating = true;
}

void Menu::renderBackground(Renderer* renderer) {
    float alpha = m_animProgress * 0.9f;
    Color bgColor = m_backgroundColor;
    bgColor.a = static_cast<uint8_t>(alpha * bgColor.a);

    renderer->drawRect(
        Rect(0, 0, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)),
        bgColor,
        true
    );
}

void Menu::renderTitle(Renderer* renderer) {
    float titleY = m_screenHeight * 0.2f;
    float scale = 1.0f + 0.05f * std::sin(m_titlePulse);
    float alpha = m_animProgress;

    Color titleColor = m_titleColor;
    titleColor.a = static_cast<uint8_t>(alpha * 255);

    renderer->drawTextCentered(
        m_title,
        Vec2(m_position.x, titleY),
        titleColor,
        48.0f * scale
    );

    if (!m_subtitle.empty()) {
        Color subtitleColor = Color(m_titleColor.r, m_titleColor.g, m_titleColor.b, 
                                    static_cast<uint8_t>(alpha * 180));
        renderer->drawTextCentered(
            m_subtitle,
            Vec2(m_position.x, titleY + 50),
            subtitleColor,
            24.0f
        );
    }
}

void Menu::renderItems(Renderer* renderer) {
    float startY = m_screenHeight * 0.45f;

    for (size_t i = 0; i < m_items.size(); ++i) {
        const MenuItem& item = m_items[i];
        float y = startY + i * m_itemSpacing;
        float x = m_position.x + item.animOffset;

        bool isSelected = (static_cast<int>(i) == m_selectedIndex);
        float alpha = m_animProgress;
        
        Color color = isSelected ? m_selectedColor : m_itemColor;
        if (!item.enabled) {
            color = Color(80, 80, 80);
        }
        color.a = static_cast<uint8_t>(alpha * 255);

        // Draw selection indicator
        if (isSelected) {
            renderer->drawRect(
                Rect(x - 150, y - 20, 300, 50),
                Color(m_selectedColor.r, m_selectedColor.g, m_selectedColor.b, 
                      static_cast<uint8_t>(alpha * 50)),
                true
            );
        }

        renderer->drawTextCentered(
            item.text,
            Vec2(x, y),
            color,
            isSelected ? 32.0f : 28.0f
        );
    }
}

Rect Menu::getItemBounds(int index) const {
    float startY = m_screenHeight * 0.45f;
    float y = startY + index * m_itemSpacing;
    
    return Rect(m_position.x - 150, y - 25, 300, 50);
}

// LevelSelectMenu implementation
LevelSelectMenu::LevelSelectMenu(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    m_levelStars.resize(m_levelCount, 0);
    
    float gridWidth = m_columns * (m_buttonSize + m_buttonSpacing) - m_buttonSpacing;
    m_gridOffset = Vec2((screenWidth - gridWidth) / 2, 200);
}

void LevelSelectMenu::update(float /*deltaTime*/) {
    // Animation updates if needed
}

void LevelSelectMenu::render(Renderer* renderer) {
    if (!m_visible) return;

    // Background
    renderer->drawRect(
        Rect(0, 0, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)),
        Color(20, 20, 40, 240),
        true
    );

    // Title
    renderer->drawTextCentered(
        "SELECT LEVEL",
        Vec2(m_screenWidth / 2.0f, 100),
        Color::white(),
        36.0f
    );

    // Level buttons
    for (int i = 0; i < m_levelCount; ++i) {
        renderLevelButton(renderer, i);
    }
}

bool LevelSelectMenu::handleTouch(const TouchPoint& touch) {
    if (!m_visible) return false;

    for (int i = 0; i < m_levelCount; ++i) {
        Rect bounds = getLevelButtonBounds(i);
        if (bounds.contains(touch.position)) {
            if (!touch.isActive && isLevelUnlocked(i + 1)) {
                m_selectedLevel = i + 1;
                if (m_onLevelSelected) {
                    m_onLevelSelected(m_selectedLevel);
                }
                return true;
            }
        }
    }

    return false;
}

void LevelSelectMenu::setLevelStars(int level, int stars) {
    if (level > 0 && level <= static_cast<int>(m_levelStars.size())) {
        m_levelStars[level - 1] = stars;
    }
}

void LevelSelectMenu::show() {
    m_visible = true;
}

void LevelSelectMenu::hide() {
    m_visible = false;
}

Rect LevelSelectMenu::getLevelButtonBounds(int level) const {
    int row = level / m_columns;
    int col = level % m_columns;

    float x = m_gridOffset.x + col * (m_buttonSize + m_buttonSpacing);
    float y = m_gridOffset.y + row * (m_buttonSize + m_buttonSpacing) - m_scrollOffset;

    return Rect(x, y, m_buttonSize, m_buttonSize);
}

void LevelSelectMenu::renderLevelButton(Renderer* renderer, int level) {
    Rect bounds = getLevelButtonBounds(level);
    int levelNum = level + 1;
    bool unlocked = levelNum <= m_unlockedLevels;

    // Button background
    Color bgColor = unlocked ? Color(60, 80, 100, 220) : Color(40, 40, 50, 200);
    renderer->drawRect(bounds, bgColor, true);

    // Border
    Color borderColor = (m_selectedLevel == levelNum) ? Color::cyan() : Color(100, 120, 150);
    renderer->drawRect(bounds, borderColor, false);

    // Level number
    Color textColor = unlocked ? Color::white() : Color(80, 80, 80);
    renderer->drawTextCentered(
        std::to_string(levelNum),
        Vec2(bounds.x + bounds.w / 2, bounds.y + bounds.h / 2 - 10),
        textColor,
        28.0f
    );

    // Stars
    if (unlocked) {
        int stars = m_levelStars[level];
        float starY = bounds.y + bounds.h - 25;
        float starSpacing = 20.0f;
        float startX = bounds.x + bounds.w / 2 - starSpacing;

        for (int s = 0; s < 3; ++s) {
            Color starColor = (s < stars) ? Color::yellow() : Color(60, 60, 70);
            renderer->drawCircle(
                Vec2(startX + s * starSpacing, starY),
                8.0f,
                starColor,
                true
            );
        }
    } else {
        // Lock icon (simplified)
        renderer->drawCircle(
            bounds.center(),
            15.0f,
            Color(80, 80, 80),
            false
        );
    }
}

bool LevelSelectMenu::isLevelUnlocked(int level) const {
    return level <= m_unlockedLevels;
}

} // namespace Vectoria
