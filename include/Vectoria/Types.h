#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace Vectoria {

// Vector2 for 2D mathematics
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }

    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
    
    Vec2 normalized() const {
        float len = length();
        if (len > 0.0001f) return *this / len;
        return Vec2(0, 0);
    }

    float dot(const Vec2& other) const { return x * other.x + y * other.y; }
    float cross(const Vec2& other) const { return x * other.y - y * other.x; }

    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return a + (b - a) * t;
    }
};

// Color with alpha
struct Color {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    Color() = default;
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}

    static Color fromFloat(float r, float g, float b, float a = 1.0f) {
        return Color(
            static_cast<uint8_t>(r * 255),
            static_cast<uint8_t>(g * 255),
            static_cast<uint8_t>(b * 255),
            static_cast<uint8_t>(a * 255)
        );
    }

    static Color lerp(const Color& a, const Color& b, float t) {
        return Color(
            static_cast<uint8_t>(a.r + (b.r - a.r) * t),
            static_cast<uint8_t>(a.g + (b.g - a.g) * t),
            static_cast<uint8_t>(a.b + (b.b - a.b) * t),
            static_cast<uint8_t>(a.a + (b.a - a.a) * t)
        );
    }

    // Predefined colors
    static Color white() { return Color(255, 255, 255); }
    static Color black() { return Color(0, 0, 0); }
    static Color red() { return Color(255, 100, 100); }
    static Color green() { return Color(100, 255, 100); }
    static Color blue() { return Color(100, 150, 255); }
    static Color yellow() { return Color(255, 255, 100); }
    static Color cyan() { return Color(100, 255, 255); }
    static Color magenta() { return Color(255, 100, 255); }
    static Color orange() { return Color(255, 180, 80); }
    static Color purple() { return Color(180, 100, 255); }
};

// Rectangle
struct Rect {
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;

    Rect() = default;
    Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

    bool contains(const Vec2& point) const {
        return point.x >= x && point.x <= x + w &&
               point.y >= y && point.y <= y + h;
    }

    bool intersects(const Rect& other) const {
        return !(x + w < other.x || other.x + other.w < x ||
                 y + h < other.y || other.y + other.h < y);
    }

    Vec2 center() const { return Vec2(x + w / 2, y + h / 2); }
};

// Touch/Input state
struct TouchPoint {
    int id = 0;
    Vec2 position;
    Vec2 previousPosition;
    Vec2 startPosition;
    bool isActive = false;
    float pressure = 1.0f;
    float timestamp = 0.0f;
};

// Gravity vector with decay
struct GravityStroke {
    std::vector<Vec2> points;
    Vec2 direction;
    float strength = 1.0f;
    float lifetime = 0.0f;
    float maxLifetime = 2.0f;
    Color color;
    bool isActive = true;

    float getAlpha() const {
        return std::max(0.0f, 1.0f - (lifetime / maxLifetime));
    }
};

// Object types in the game
enum class ObjectType {
    Ball,
    Box,
    Triangle,
    Star,
    Blob
};

// Zone types for gravity influence
enum class ZoneType {
    Normal,
    Boost,
    Slow,
    Reverse,
    Attract,
    Repel,
    Zero
};

// Game states
enum class GameStateType {
    Menu,
    Playing,
    Paused,
    LevelComplete,
    GameOver,
    Tutorial
};

// Level objective types
enum class ObjectiveType {
    ReachGoal,
    CollectItems,
    TimeChallenge,
    ChainReaction,
    MinimizeStrokes,
    MaximizeEnergy
};

// Callback types
using UpdateCallback = std::function<void(float deltaTime)>;
using CollisionCallback = std::function<void(class PhysicsObject*, class PhysicsObject*)>;
using InputCallback = std::function<void(const TouchPoint&)>;

} // namespace Vectoria
