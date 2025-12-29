#pragma once

#include "GravityPaint/Types.h"

namespace GravityPaint {

class GravityField {
public:
    GravityField(const Vec2& position, const Vec2& direction, float strength, float radius);
    ~GravityField() = default;

    void update(float deltaTime);
    Vec2 calculateForce(const Vec2& objectPosition) const;
    bool isPointInRange(const Vec2& point) const;

    // Accessors
    Vec2 getPosition() const { return m_position; }
    void setPosition(const Vec2& position) { m_position = position; }

    Vec2 getDirection() const { return m_direction; }
    void setDirection(const Vec2& direction) { m_direction = direction.normalized(); }

    float getStrength() const { return m_strength; }
    void setStrength(float strength) { m_strength = strength; }

    float getRadius() const { return m_radius; }
    void setRadius(float radius) { m_radius = radius; }

    ZoneType getZoneType() const { return m_zoneType; }
    void setZoneType(ZoneType type) { m_zoneType = type; }

    Color getColor() const { return m_color; }
    void setColor(const Color& color) { m_color = color; }

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    float getLifetime() const { return m_lifetime; }
    float getMaxLifetime() const { return m_maxLifetime; }
    void setMaxLifetime(float lifetime) { m_maxLifetime = lifetime; }
    bool isExpired() const { return m_lifetime >= m_maxLifetime && m_maxLifetime > 0; }

    // Pulse effect for visualization
    float getPulsePhase() const { return m_pulsePhase; }

private:
    Vec2 m_position;
    Vec2 m_direction;
    float m_strength;
    float m_radius;
    ZoneType m_zoneType = ZoneType::Normal;
    Color m_color = Color::blue();
    bool m_active = true;
    float m_lifetime = 0.0f;
    float m_maxLifetime = 0.0f;  // 0 = permanent
    float m_pulsePhase = 0.0f;
};

// Specialized gravity zone types
class GravityZone : public GravityField {
public:
    GravityZone(const Vec2& position, float width, float height, ZoneType type);
    
    bool isPointInZone(const Vec2& point) const;
    Vec2 calculateZoneForce(const Vec2& objectPosition, const Vec2& objectVelocity) const;

    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    Rect getBounds() const;

private:
    float m_width;
    float m_height;
};

} // namespace GravityPaint
