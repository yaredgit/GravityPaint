#include "GravityPaint/physics/PhysicsObject.h"
#include "GravityPaint/Constants.h"
#include <cmath>
#include <algorithm>

namespace GravityPaint {

int PhysicsObject::s_nextId = 1;

PhysicsObject::PhysicsObject(b2World* world, ObjectType type, const Vec2& position, float size)
    : m_type(type)
    , m_size(size)
    , m_id(s_nextId++)
{
    createBody(world, position);

    // Set color based on type
    switch (type) {
        case ObjectType::Ball:
            m_color = Color::cyan();
            break;
        case ObjectType::Box:
            m_color = Color::orange();
            break;
        case ObjectType::Triangle:
            m_color = Color::green();
            break;
        case ObjectType::Star:
            m_color = Color::yellow();
            break;
        case ObjectType::Blob:
            m_color = Color::magenta();
            break;
    }
}

PhysicsObject::~PhysicsObject() {
    if (m_body && m_body->GetWorld()) {
        m_body->GetWorld()->DestroyBody(m_body);
    }
}

void PhysicsObject::createBody(b2World* world, const Vec2& position) {
    if (!world) return;

    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(position.x / PHYSICS_SCALE, position.y / PHYSICS_SCALE);
    bodyDef.linearDamping = 0.5f;
    bodyDef.angularDamping = 0.3f;

    m_body = world->CreateBody(&bodyDef);
    if (!m_body) return;
    
    b2BodyUserData& userData = m_body->GetUserData();
    userData.pointer = reinterpret_cast<uintptr_t>(this);

    float scaledSize = m_size * 20.0f / PHYSICS_SCALE;

    // Create shapes outside switch to avoid scope issues
    b2CircleShape circleShape;
    b2PolygonShape polyShape;
    
    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.6f;

    switch (m_type) {
        case ObjectType::Ball:
        case ObjectType::Blob:
            circleShape.m_radius = scaledSize;
            fixtureDef.shape = &circleShape;
            break;

        case ObjectType::Box:
            polyShape.SetAsBox(scaledSize, scaledSize);
            fixtureDef.shape = &polyShape;
            break;

        case ObjectType::Triangle: {
            b2Vec2 vertices[3];
            vertices[0].Set(0, -scaledSize);
            vertices[1].Set(-scaledSize, scaledSize);
            vertices[2].Set(scaledSize, scaledSize);
            polyShape.Set(vertices, 3);
            fixtureDef.shape = &polyShape;
            break;
        }

        case ObjectType::Star: {
            // Simplified star as pentagon
            b2Vec2 vertices[5];
            for (int i = 0; i < 5; ++i) {
                float angle = (i * 2.0f * 3.14159f / 5.0f) - 3.14159f / 2.0f;
                vertices[i].Set(std::cos(angle) * scaledSize, std::sin(angle) * scaledSize);
            }
            polyShape.Set(vertices, 5);
            fixtureDef.shape = &polyShape;
            break;
        }
    }

    m_body->CreateFixture(&fixtureDef);
}

void PhysicsObject::update(float deltaTime) {
    if (!m_active) return;

    // Decay energy over time
    m_energy = std::max(0.0f, m_energy - ENERGY_DECAY_RATE * deltaTime);

    // Update trail
    m_trailTimer += deltaTime;
    if (m_trailTimer >= 0.02f) {
        updateTrail();
        m_trailTimer = 0.0f;
    }
}

void PhysicsObject::updateTrail() {
    if (!m_body) return;

    Vec2 pos = getPosition();
    m_trail.push_back(pos);

    while (m_trail.size() > TRAIL_MAX_POINTS) {
        m_trail.erase(m_trail.begin());
    }
}

void PhysicsObject::applyForce(const Vec2& force) {
    if (m_body) {
        m_body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
    }
}

void PhysicsObject::applyImpulse(const Vec2& impulse) {
    if (m_body) {
        m_body->ApplyLinearImpulseToCenter(b2Vec2(impulse.x, impulse.y), true);
    }
}

void PhysicsObject::setVelocity(const Vec2& velocity) {
    if (m_body) {
        m_body->SetLinearVelocity(b2Vec2(velocity.x / PHYSICS_SCALE, velocity.y / PHYSICS_SCALE));
    }
}

Vec2 PhysicsObject::getPosition() const {
    if (!m_body) return Vec2(0, 0);
    b2Vec2 pos = m_body->GetPosition();
    return Vec2(pos.x * PHYSICS_SCALE, pos.y * PHYSICS_SCALE);
}

void PhysicsObject::setPosition(const Vec2& position) {
    if (m_body) {
        m_body->SetTransform(b2Vec2(position.x / PHYSICS_SCALE, position.y / PHYSICS_SCALE), m_body->GetAngle());
    }
}

Vec2 PhysicsObject::getVelocity() const {
    if (!m_body) return Vec2(0, 0);
    b2Vec2 vel = m_body->GetLinearVelocity();
    return Vec2(vel.x * PHYSICS_SCALE, vel.y * PHYSICS_SCALE);
}

float PhysicsObject::getAngle() const {
    if (!m_body) return 0.0f;
    return m_body->GetAngle();
}

void PhysicsObject::setAngle(float angle) {
    if (m_body) {
        m_body->SetTransform(m_body->GetPosition(), angle);
    }
}

float PhysicsObject::getAngularVelocity() const {
    if (!m_body) return 0.0f;
    return m_body->GetAngularVelocity();
}

float PhysicsObject::getMass() const {
    if (!m_body) return 1.0f;
    return m_body->GetMass();
}

void PhysicsObject::setEnergy(float energy) {
    m_energy = std::clamp(energy, 0.0f, MAX_OBJECT_ENERGY);
}

void PhysicsObject::addEnergy(float amount) {
    setEnergy(m_energy + amount);
}

void PhysicsObject::transferEnergy(PhysicsObject* other, float rate) {
    if (!other) return;

    float transfer = std::min(m_energy, rate);
    m_energy -= transfer;
    other->addEnergy(transfer);
}

Color PhysicsObject::getEnergyColor() const {
    // Gradient from blue (low) to white (medium) to yellow/red (high)
    float normalized = m_energy / MAX_OBJECT_ENERGY;

    if (normalized < 0.5f) {
        float t = normalized * 2.0f;
        return Color::lerp(Color::blue(), Color::white(), t);
    } else {
        float t = (normalized - 0.5f) * 2.0f;
        return Color::lerp(Color::white(), Color::orange(), t);
    }
}

void PhysicsObject::setActive(bool active) {
    m_active = active;
    if (m_body) {
        m_body->SetEnabled(active);
    }
}

} // namespace GravityPaint
