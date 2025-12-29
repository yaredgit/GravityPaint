#include "GravityPaint/physics/GravityField.h"
#include <cmath>

namespace GravityPaint {

GravityField::GravityField(const Vec2& position, const Vec2& direction, float strength, float radius)
    : m_position(position)
    , m_direction(direction.normalized())
    , m_strength(strength)
    , m_radius(radius)
{
    // Set color based on direction
    float angle = std::atan2(m_direction.y, m_direction.x);
    float hue = (angle + 3.14159f) / (2.0f * 3.14159f); // 0-1

    // Simple HSV to RGB conversion for hue
    float r, g, b;
    int h_i = static_cast<int>(hue * 6);
    float f = hue * 6 - h_i;
    float q = 1 - f;
    
    switch (h_i % 6) {
        case 0: r = 1; g = f; b = 0; break;
        case 1: r = q; g = 1; b = 0; break;
        case 2: r = 0; g = 1; b = f; break;
        case 3: r = 0; g = q; b = 1; break;
        case 4: r = f; g = 0; b = 1; break;
        default: r = 1; g = 0; b = q; break;
    }

    m_color = Color::fromFloat(r * 0.8f + 0.2f, g * 0.8f + 0.2f, b * 0.8f + 0.2f);
}

void GravityField::update(float deltaTime) {
    m_lifetime += deltaTime;
    m_pulsePhase += deltaTime * 3.0f;
    
    if (m_pulsePhase > 6.28318f) {
        m_pulsePhase -= 6.28318f;
    }
}

Vec2 GravityField::calculateForce(const Vec2& objectPosition) const {
    if (!m_active) return Vec2(0, 0);

    Vec2 toObject = objectPosition - m_position;
    float distance = toObject.length();

    if (distance > m_radius || distance < 0.001f) {
        return Vec2(0, 0);
    }

    // Force falls off with distance
    float falloff = 1.0f - (distance / m_radius);
    falloff = falloff * falloff; // Quadratic falloff for smoother feel

    Vec2 force;

    switch (m_zoneType) {
        case ZoneType::Normal:
            force = m_direction * m_strength * falloff;
            break;

        case ZoneType::Boost:
            force = m_direction * m_strength * falloff * 2.0f;
            break;

        case ZoneType::Slow:
            force = m_direction * m_strength * falloff * 0.3f;
            break;

        case ZoneType::Reverse:
            force = m_direction * -m_strength * falloff;
            break;

        case ZoneType::Attract: {
            // Pull toward center
            Vec2 toCenter = (m_position - objectPosition).normalized();
            force = toCenter * m_strength * falloff;
            break;
        }

        case ZoneType::Repel: {
            // Push away from center
            Vec2 fromCenter = toObject.normalized();
            force = fromCenter * m_strength * falloff;
            break;
        }

        case ZoneType::Zero:
            // Counteract gravity - return negative of expected gravity
            force = Vec2(0, -9.8f * falloff);
            break;
    }

    return force;
}

bool GravityField::isPointInRange(const Vec2& point) const {
    return (point - m_position).lengthSquared() <= m_radius * m_radius;
}

// GravityZone implementation
GravityZone::GravityZone(const Vec2& position, float width, float height, ZoneType type)
    : GravityField(position, Vec2(0, 1), 9.8f, std::max(width, height))
    , m_width(width)
    , m_height(height)
{
    setZoneType(type);

    // Set color based on zone type
    switch (type) {
        case ZoneType::Boost:
            setColor(Color::green());
            break;
        case ZoneType::Slow:
            setColor(Color::blue());
            break;
        case ZoneType::Reverse:
            setColor(Color::purple());
            break;
        case ZoneType::Attract:
            setColor(Color::orange());
            break;
        case ZoneType::Repel:
            setColor(Color::red());
            break;
        case ZoneType::Zero:
            setColor(Color::cyan());
            break;
        default:
            setColor(Color(100, 100, 150));
            break;
    }
}

bool GravityZone::isPointInZone(const Vec2& point) const {
    Rect bounds = getBounds();
    return bounds.contains(point);
}

Vec2 GravityZone::calculateZoneForce(const Vec2& objectPosition, const Vec2& objectVelocity) const {
    if (!isPointInZone(objectPosition)) {
        return Vec2(0, 0);
    }

    Vec2 force = calculateForce(objectPosition);

    // Apply velocity-based modifiers for some zone types
    if (getZoneType() == ZoneType::Slow) {
        // Add drag force
        force -= objectVelocity * 0.5f;
    }

    return force;
}

Rect GravityZone::getBounds() const {
    Vec2 pos = getPosition();
    return Rect(pos.x - m_width / 2, pos.y - m_height / 2, m_width, m_height);
}

} // namespace GravityPaint
