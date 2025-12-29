#include "GravityPaint/graphics/Camera.h"
#include <cmath>
#include <random>
#include <algorithm>

namespace GravityPaint {

Camera::Camera(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    m_position = Vec2(screenWidth / 2.0f, screenHeight / 2.0f);
}

void Camera::update(float deltaTime) {
    // Follow target smoothly
    if (m_hasTarget) {
        Vec2 diff = m_target - m_position;
        m_position += diff * (m_followSmoothing * deltaTime);
    }

    // Update shake
    if (m_shakeTimer > 0) {
        updateShake(deltaTime);
    }

    // Clamp to bounds
    if (m_hasBounds) {
        clampToBounds();
    }
}

void Camera::setPosition(const Vec2& position) {
    m_position = position;
    if (m_hasBounds) {
        clampToBounds();
    }
}

void Camera::move(const Vec2& delta) {
    m_position += delta;
    if (m_hasBounds) {
        clampToBounds();
    }
}

void Camera::setZoom(float zoom) {
    m_zoom = std::clamp(zoom, 0.1f, 10.0f);
}

void Camera::zoomBy(float delta) {
    setZoom(m_zoom + delta);
}

void Camera::setRotation(float rotation) {
    m_rotation = rotation;
    // Normalize to 0-2PI
    while (m_rotation < 0) m_rotation += 6.28318f;
    while (m_rotation >= 6.28318f) m_rotation -= 6.28318f;
}

void Camera::rotate(float delta) {
    setRotation(m_rotation + delta);
}

void Camera::setTarget(const Vec2& target) {
    m_target = target;
    m_hasTarget = true;
}

void Camera::shake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeDuration = duration;
    m_shakeTimer = duration;
}

void Camera::setBounds(const Rect& bounds) {
    m_bounds = bounds;
    m_hasBounds = true;
    clampToBounds();
}

Vec2 Camera::worldToScreen(const Vec2& worldPos) const {
    Vec2 relative = worldPos - m_position;

    // Apply rotation
    if (m_rotation != 0) {
        float cos_r = std::cos(-m_rotation);
        float sin_r = std::sin(-m_rotation);
        relative = Vec2(
            relative.x * cos_r - relative.y * sin_r,
            relative.x * sin_r + relative.y * cos_r
        );
    }

    // Apply zoom
    relative *= m_zoom;

    // Center on screen
    Vec2 screenPos = Vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f) + relative;

    // Add shake offset
    screenPos += m_shakeOffset;

    return screenPos;
}

Vec2 Camera::screenToWorld(const Vec2& screenPos) const {
    // Remove shake offset
    Vec2 pos = screenPos - m_shakeOffset;

    // Uncenter
    Vec2 relative = pos - Vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f);

    // Remove zoom
    relative /= m_zoom;

    // Remove rotation
    if (m_rotation != 0) {
        float cos_r = std::cos(m_rotation);
        float sin_r = std::sin(m_rotation);
        relative = Vec2(
            relative.x * cos_r - relative.y * sin_r,
            relative.x * sin_r + relative.y * cos_r
        );
    }

    return m_position + relative;
}

Rect Camera::getViewRect() const {
    float halfWidth = (m_screenWidth / 2.0f) / m_zoom;
    float halfHeight = (m_screenHeight / 2.0f) / m_zoom;

    return Rect(
        m_position.x - halfWidth,
        m_position.y - halfHeight,
        halfWidth * 2,
        halfHeight * 2
    );
}

void Camera::setScreenSize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
}

void Camera::clampToBounds() {
    float halfWidth = (m_screenWidth / 2.0f) / m_zoom;
    float halfHeight = (m_screenHeight / 2.0f) / m_zoom;

    m_position.x = std::clamp(m_position.x, m_bounds.x + halfWidth, m_bounds.x + m_bounds.w - halfWidth);
    m_position.y = std::clamp(m_position.y, m_bounds.y + halfHeight, m_bounds.y + m_bounds.h - halfHeight);
}

void Camera::updateShake(float deltaTime) {
    m_shakeTimer -= deltaTime;

    if (m_shakeTimer <= 0) {
        m_shakeTimer = 0;
        m_shakeOffset = Vec2(0, 0);
        return;
    }

    // Calculate shake intensity based on remaining time
    float progress = m_shakeTimer / m_shakeDuration;
    float currentIntensity = m_shakeIntensity * progress;

    // Random offset
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    m_shakeOffset = Vec2(dist(rng), dist(rng)) * currentIntensity;
}

} // namespace GravityPaint
