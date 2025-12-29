#pragma once

#include "GravityPaint/Types.h"

namespace GravityPaint {

class Camera {
public:
    Camera(int screenWidth, int screenHeight);
    ~Camera() = default;

    void update(float deltaTime);

    // Position
    Vec2 getPosition() const { return m_position; }
    void setPosition(const Vec2& position);
    void move(const Vec2& delta);

    // Zoom
    float getZoom() const { return m_zoom; }
    void setZoom(float zoom);
    void zoomBy(float delta);

    // Rotation
    float getRotation() const { return m_rotation; }
    void setRotation(float rotation);
    void rotate(float delta);

    // Smooth follow
    void setTarget(const Vec2& target);
    void setFollowSmoothing(float smoothing) { m_followSmoothing = smoothing; }
    void clearTarget() { m_hasTarget = false; }

    // Shake effect
    void shake(float intensity, float duration);
    bool isShaking() const { return m_shakeTimer > 0; }

    // Bounds
    void setBounds(const Rect& bounds);
    void clearBounds() { m_hasBounds = false; }

    // Transform
    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;
    Rect getViewRect() const;

    // Screen info
    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }
    void setScreenSize(int width, int height);

private:
    void clampToBounds();
    void updateShake(float deltaTime);

    Vec2 m_position;
    Vec2 m_shakeOffset;
    float m_zoom = 1.0f;
    float m_rotation = 0.0f;

    int m_screenWidth;
    int m_screenHeight;

    // Target following
    bool m_hasTarget = false;
    Vec2 m_target;
    float m_followSmoothing = 5.0f;

    // Shake
    float m_shakeIntensity = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeTimer = 0.0f;

    // Bounds
    bool m_hasBounds = false;
    Rect m_bounds;
};

} // namespace GravityPaint
