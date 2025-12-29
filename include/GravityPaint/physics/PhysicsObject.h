#pragma once

#include "GravityPaint/Types.h"
#include <box2d/box2d.h>
#include <vector>

namespace GravityPaint {

class PhysicsObject {
public:
    PhysicsObject(b2World* world, ObjectType type, const Vec2& position, float size);
    ~PhysicsObject();

    void update(float deltaTime);
    void applyForce(const Vec2& force);
    void applyImpulse(const Vec2& impulse);
    void setVelocity(const Vec2& velocity);

    // Physics properties
    Vec2 getPosition() const;
    void setPosition(const Vec2& position);
    Vec2 getVelocity() const;
    float getAngle() const;
    void setAngle(float angle);
    float getAngularVelocity() const;

    // Object properties
    ObjectType getType() const { return m_type; }
    float getSize() const { return m_size; }
    float getMass() const;
    
    // Energy system
    float getEnergy() const { return m_energy; }
    void setEnergy(float energy);
    void addEnergy(float amount);
    void transferEnergy(PhysicsObject* other, float rate);
    Color getEnergyColor() const;

    // Visual
    Color getColor() const { return m_color; }
    void setColor(const Color& color) { m_color = color; }
    
    // Trail for visual effect
    const std::vector<Vec2>& getTrail() const { return m_trail; }
    void clearTrail() { m_trail.clear(); }

    // State
    bool isActive() const { return m_active; }
    void setActive(bool active);
    bool isInGoal() const { return m_inGoal; }
    void setInGoal(bool inGoal) { m_inGoal = inGoal; }
    bool isCollected() const { return m_collected; }
    void setCollected(bool collected) { m_collected = collected; }
    bool hasReachedGoal() const { return m_reachedGoal; }
    void setReachedGoal(bool reached) { m_reachedGoal = reached; }

    // Box2D access
    b2Body* getBody() const { return m_body; }

    // ID for tracking
    int getId() const { return m_id; }

private:
    void createBody(b2World* world, const Vec2& position);
    void updateTrail();

    b2Body* m_body = nullptr;
    ObjectType m_type;
    float m_size;
    int m_id;

    float m_energy = 50.0f;
    Color m_color = Color::white();
    
    std::vector<Vec2> m_trail;
    float m_trailTimer = 0.0f;

    bool m_active = true;
    bool m_inGoal = false;
    bool m_collected = false;
    bool m_reachedGoal = false;

    static int s_nextId;
};

} // namespace GravityPaint
