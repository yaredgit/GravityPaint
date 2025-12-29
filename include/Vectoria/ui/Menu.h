#pragma once

#include "Vectoria/Types.h"
#include <string>
#include <vector>
#include <functional>

namespace Vectoria {

class Renderer;

struct MenuItem {
    std::string text;
    std::function<void()> action;
    bool enabled = true;
    bool selected = false;
    float animOffset = 0.0f;
};

class Menu {
public:
    Menu(int screenWidth, int screenHeight);
    ~Menu() = default;

    void update(float deltaTime);
    void render(Renderer* renderer);
    bool handleTouch(const TouchPoint& touch);

    // Menu items
    void addItem(const std::string& text, std::function<void()> action);
    void clearItems();
    void setSelectedIndex(int index);
    int getSelectedIndex() const { return m_selectedIndex; }

    // Title
    void setTitle(const std::string& title) { m_title = title; }
    void setSubtitle(const std::string& subtitle) { m_subtitle = subtitle; }

    // Appearance
    void setBackgroundColor(const Color& color) { m_backgroundColor = color; }
    void setTitleColor(const Color& color) { m_titleColor = color; }
    void setItemColor(const Color& color) { m_itemColor = color; }
    void setSelectedColor(const Color& color) { m_selectedColor = color; }

    // Animation
    void show();
    void hide();
    bool isVisible() const { return m_visible; }
    bool isAnimating() const { return m_animating; }

    // Layout
    void setPosition(const Vec2& position) { m_position = position; }
    void setItemSpacing(float spacing) { m_itemSpacing = spacing; }

private:
    void renderBackground(Renderer* renderer);
    void renderTitle(Renderer* renderer);
    void renderItems(Renderer* renderer);
    Rect getItemBounds(int index) const;

    int m_screenWidth;
    int m_screenHeight;

    std::string m_title;
    std::string m_subtitle;
    std::vector<MenuItem> m_items;
    int m_selectedIndex = 0;

    Vec2 m_position;
    float m_itemSpacing = 80.0f;

    Color m_backgroundColor = Color(20, 20, 40, 230);
    Color m_titleColor = Color::white();
    Color m_itemColor = Color(180, 180, 200);
    Color m_selectedColor = Color::cyan();

    bool m_visible = false;
    bool m_animating = false;
    float m_animProgress = 0.0f;
    float m_titlePulse = 0.0f;
};

// Level select menu with grid layout
class LevelSelectMenu {
public:
    LevelSelectMenu(int screenWidth, int screenHeight);
    ~LevelSelectMenu() = default;

    void update(float deltaTime);
    void render(Renderer* renderer);
    bool handleTouch(const TouchPoint& touch);

    void setLevelCount(int count) { m_levelCount = count; }
    void setUnlockedLevels(int count) { m_unlockedLevels = count; }
    void setLevelStars(int level, int stars);
    void setOnLevelSelected(std::function<void(int)> callback) { m_onLevelSelected = callback; }

    void show();
    void hide();
    bool isVisible() const { return m_visible; }

private:
    Rect getLevelButtonBounds(int level) const;
    void renderLevelButton(Renderer* renderer, int level);

    int m_screenWidth;
    int m_screenHeight;

    int m_levelCount = 50;
    int m_unlockedLevels = 1;
    std::vector<int> m_levelStars;

    int m_columns = 5;
    float m_buttonSize = 120.0f;
    float m_buttonSpacing = 20.0f;
    Vec2 m_gridOffset;

    int m_selectedLevel = -1;
    int m_scrollOffset = 0;
    
    std::function<void(int)> m_onLevelSelected;
    bool m_visible = false;
};

} // namespace Vectoria
