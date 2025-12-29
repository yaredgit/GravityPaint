#include "Vectoria/physics/PhysicsWorld.h"
#include "Vectoria/physics/GravityField.h"
#include "Vectoria/physics/PhysicsObject.h"
#include "Vectoria/physics/DeformableSurface.h"
#include "Vectoria/Constants.h"

namespace Vectoria {

void ContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    void* userDataA = reinterpret_cast<void*>(fixtureA->GetBody()->GetUserData().pointer);
    void* userDataB = reinterpret_cast<void*>(fixtureB->GetBody()->GetUserData().pointer);

    if (m_callback && userDataA && userDataB) {
        m_callback(static_cast<PhysicsObject*>(userDataA), static_cast<PhysicsObject*>(userDataB));
    }
}

void ContactListener::EndContact(b2Contact* /*contact*/) {}

void ContactListener::PreSolve(b2Contact* /*contact*/, const b2Manifold* /*oldManifold*/) {}

void ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    // Energy transfer on collision
    float totalImpulse = 0;
    for (int i = 0; i < impulse->count; ++i) {
        totalImpulse += impulse->normalImpulses[i];
    }

    if (totalImpulse > 1.0f) {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();

        void* userDataA = reinterpret_cast<void*>(fixtureA->GetBody()->GetUserData().pointer);
        void* userDataB = reinterpret_cast<void*>(fixtureB->GetBody()->GetUserData().pointer);

        if (userDataA && userDataB) {
            PhysicsObject* objA = static_cast<PhysicsObject*>(userDataA);
            PhysicsObject* objB = static_cast<PhysicsObject*>(userDataB);

            // Transfer energy based on impact
            float transfer = std::min(totalImpulse * 0.1f, 10.0f);
            if (objA->getEnergy() > objB->getEnergy()) {
                objA->transferEnergy(objB, transfer * ENERGY_TRANSFER_RATE);
            } else {
                objB->transferEnergy(objA, transfer * ENERGY_TRANSFER_RATE);
            }
        }
    }
}

PhysicsWorld::PhysicsWorld() = default;

PhysicsWorld::~PhysicsWorld() {
    shutdown();
}

bool PhysicsWorld::initialize() {
    b2Vec2 gravity(m_globalGravity.x, m_globalGravity.y);
    m_world = std::make_unique<b2World>(gravity);

    m_contactListener = std::make_unique<ContactListener>();
    m_world->SetContactListener(m_contactListener.get());

    return true;
}

void PhysicsWorld::shutdown() {
    clearObjects();
    clearGravityFields();
    m_deformableSurfaces.clear();
    destroyBoundaries();

    for (auto* body : m_staticBodies) {
        if (m_world) {
            m_world->DestroyBody(body);
        }
    }
    m_staticBodies.clear();

    m_contactListener.reset();
    m_world.reset();
}

void PhysicsWorld::update(float deltaTime) {
    if (!m_world) return;

    // Apply custom gravity fields
    applyGravityFields();

    // Fixed timestep physics
    m_accumulator += deltaTime;
    while (m_accumulator >= FIXED_TIMESTEP) {
        m_world->Step(FIXED_TIMESTEP, PHYSICS_VELOCITY_ITERATIONS, PHYSICS_POSITION_ITERATIONS);
        m_accumulator -= FIXED_TIMESTEP;
    }

    // Update objects
    for (auto& obj : m_objects) {
        if (obj->isActive()) {
            obj->update(deltaTime);

            // Check if in goal zone
            if (m_hasGoalZone) {
                Vec2 pos = obj->getPosition();
                if (m_goalZone.contains(pos)) {
                    obj->setInGoal(true);
                }
            }
        }
    }

    // Update deformable surfaces
    updateDeformableSurfaces(deltaTime);

    // Update gravity fields
    for (auto it = m_gravityFields.begin(); it != m_gravityFields.end();) {
        (*it)->update(deltaTime);
        if ((*it)->isExpired()) {
            it = m_gravityFields.erase(it);
        } else {
            ++it;
        }
    }
}

void PhysicsWorld::reset() {
    clearObjects();
    clearGravityFields();

    for (auto& surface : m_deformableSurfaces) {
        surface->reset();
    }

    m_accumulator = 0.0f;
}

PhysicsObject* PhysicsWorld::createObject(ObjectType type, const Vec2& position, float size) {
    auto obj = std::make_unique<PhysicsObject>(m_world.get(), type, position, size);
    PhysicsObject* ptr = obj.get();
    m_objects.push_back(std::move(obj));
    return ptr;
}

void PhysicsWorld::destroyObject(PhysicsObject* object) {
    for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (it->get() == object) {
            m_objects.erase(it);
            return;
        }
    }
}

void PhysicsWorld::clearObjects() {
    m_objects.clear();
}

GravityField* PhysicsWorld::createGravityField(const Vec2& position, const Vec2& direction, float strength, float radius) {
    auto field = std::make_unique<GravityField>(position, direction, strength, radius);
    GravityField* ptr = field.get();
    m_gravityFields.push_back(std::move(field));
    return ptr;
}

void PhysicsWorld::removeGravityField(GravityField* field) {
    for (auto it = m_gravityFields.begin(); it != m_gravityFields.end(); ++it) {
        if (it->get() == field) {
            m_gravityFields.erase(it);
            return;
        }
    }
}

void PhysicsWorld::applyGravityFromStrokes(const std::vector<GravityStroke>& strokes) {
    // Clear temporary gravity fields
    clearGravityFields();

    // Create gravity fields from active strokes
    for (const auto& stroke : strokes) {
        if (stroke.isActive && stroke.points.size() >= 2) {
            // Use stroke midpoint as position
            Vec2 midpoint = stroke.points[stroke.points.size() / 2];
            float alpha = stroke.getAlpha();
            
            auto field = createGravityField(
                midpoint,
                stroke.direction,
                stroke.strength * alpha,
                GRAVITY_STROKE_RADIUS
            );
            field->setMaxLifetime(0); // Managed externally
            field->setColor(stroke.color);
        }
    }
}

void PhysicsWorld::clearGravityFields() {
    m_gravityFields.clear();
}

DeformableSurface* PhysicsWorld::createDeformableSurface(const Vec2& position, float width, float height) {
    auto surface = std::make_unique<DeformableSurface>(position, width, height);
    surface->attachToWorld(m_world.get());
    DeformableSurface* ptr = surface.get();
    m_deformableSurfaces.push_back(std::move(surface));
    return ptr;
}

void PhysicsWorld::removeDeformableSurface(DeformableSurface* surface) {
    for (auto it = m_deformableSurfaces.begin(); it != m_deformableSurfaces.end(); ++it) {
        if (it->get() == surface) {
            (*it)->detachFromWorld();
            m_deformableSurfaces.erase(it);
            return;
        }
    }
}

void PhysicsWorld::createBoundaries(float width, float height) {
    destroyBoundaries();

    if (!m_world) return;

    float thickness = 50.0f;
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;

    // Bottom
    bodyDef.position = toMeters(Vec2(width / 2, height + thickness / 2));
    b2Body* bottom = m_world->CreateBody(&bodyDef);
    b2PolygonShape box;
    box.SetAsBox(width / 2 / PHYSICS_SCALE, thickness / 2 / PHYSICS_SCALE);
    bottom->CreateFixture(&box, 0.0f);
    m_boundaryBodies.push_back(bottom);

    // Top
    bodyDef.position = toMeters(Vec2(width / 2, -thickness / 2));
    b2Body* top = m_world->CreateBody(&bodyDef);
    top->CreateFixture(&box, 0.0f);
    m_boundaryBodies.push_back(top);

    // Left
    bodyDef.position = toMeters(Vec2(-thickness / 2, height / 2));
    b2Body* left = m_world->CreateBody(&bodyDef);
    box.SetAsBox(thickness / 2 / PHYSICS_SCALE, height / 2 / PHYSICS_SCALE);
    left->CreateFixture(&box, 0.0f);
    m_boundaryBodies.push_back(left);

    // Right
    bodyDef.position = toMeters(Vec2(width + thickness / 2, height / 2));
    b2Body* right = m_world->CreateBody(&bodyDef);
    right->CreateFixture(&box, 0.0f);
    m_boundaryBodies.push_back(right);
}

void PhysicsWorld::destroyBoundaries() {
    if (!m_world) return;

    for (auto* body : m_boundaryBodies) {
        m_world->DestroyBody(body);
    }
    m_boundaryBodies.clear();
}

b2Body* PhysicsWorld::createStaticBox(const Vec2& position, const Vec2& size) {
    if (!m_world) return nullptr;

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position = toMeters(position);

    b2Body* body = m_world->CreateBody(&bodyDef);

    b2PolygonShape box;
    box.SetAsBox(size.x / 2 / PHYSICS_SCALE, size.y / 2 / PHYSICS_SCALE);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &box;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    body->CreateFixture(&fixtureDef);
    m_staticBodies.push_back(body);

    return body;
}

b2Body* PhysicsWorld::createStaticCircle(const Vec2& position, float radius) {
    if (!m_world) return nullptr;

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position = toMeters(position);

    b2Body* body = m_world->CreateBody(&bodyDef);

    b2CircleShape circle;
    circle.m_radius = radius / PHYSICS_SCALE;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    body->CreateFixture(&fixtureDef);
    m_staticBodies.push_back(body);

    return body;
}

void PhysicsWorld::destroyStaticBody(b2Body* body) {
    if (!m_world || !body) return;

    for (auto it = m_staticBodies.begin(); it != m_staticBodies.end(); ++it) {
        if (*it == body) {
            m_world->DestroyBody(body);
            m_staticBodies.erase(it);
            return;
        }
    }
}

void PhysicsWorld::createGoalZone(const Vec2& position, const Vec2& size) {
    m_goalZone = Rect(position.x - size.x / 2, position.y - size.y / 2, size.x, size.y);
    m_hasGoalZone = true;
}

bool PhysicsWorld::isObjectInGoal(PhysicsObject* object) const {
    if (!m_hasGoalZone || !object) return false;
    return m_goalZone.contains(object->getPosition());
}

std::vector<PhysicsObject*> PhysicsWorld::getObjectsInGoal() const {
    std::vector<PhysicsObject*> result;
    if (!m_hasGoalZone) return result;

    for (const auto& obj : m_objects) {
        if (obj->isInGoal()) {
            result.push_back(obj.get());
        }
    }
    return result;
}

PhysicsObject* PhysicsWorld::getObjectAtPoint(const Vec2& point) {
    b2Vec2 p = toMeters(point);
    
    for (const auto& obj : m_objects) {
        if (obj->getBody()) {
            for (b2Fixture* f = obj->getBody()->GetFixtureList(); f; f = f->GetNext()) {
                if (f->TestPoint(p)) {
                    return obj.get();
                }
            }
        }
    }
    return nullptr;
}

std::vector<PhysicsObject*> PhysicsWorld::getObjectsInArea(const Rect& area) {
    std::vector<PhysicsObject*> result;

    for (const auto& obj : m_objects) {
        Vec2 pos = obj->getPosition();
        if (area.contains(pos)) {
            result.push_back(obj.get());
        }
    }
    return result;
}

void PhysicsWorld::setGlobalGravity(const Vec2& gravity) {
    m_globalGravity = gravity;
    if (m_world) {
        m_world->SetGravity(b2Vec2(gravity.x, gravity.y));
    }
}

void PhysicsWorld::setCollisionCallback(CollisionCallback callback) {
    if (m_contactListener) {
        m_contactListener->setCollisionCallback(callback);
    }
}

Vec2 PhysicsWorld::toPixels(const b2Vec2& meters) {
    return Vec2(meters.x * PHYSICS_SCALE, meters.y * PHYSICS_SCALE);
}

b2Vec2 PhysicsWorld::toMeters(const Vec2& pixels) {
    return b2Vec2(pixels.x / PHYSICS_SCALE, pixels.y / PHYSICS_SCALE);
}

void PhysicsWorld::applyGravityFields() {
    for (const auto& obj : m_objects) {
        if (!obj->isActive() || !obj->getBody()) continue;

        Vec2 objPos = obj->getPosition();
        Vec2 totalForce(0, 0);

        for (const auto& field : m_gravityFields) {
            if (field->isActive() && field->isPointInRange(objPos)) {
                totalForce += field->calculateForce(objPos);
            }
        }

        if (totalForce.lengthSquared() > 0.01f) {
            obj->applyForce(totalForce * obj->getMass());
        }
    }
}

void PhysicsWorld::updateDeformableSurfaces(float deltaTime) {
    for (auto& surface : m_deformableSurfaces) {
        surface->update(deltaTime);

        // Check for collisions with physics objects
        for (const auto& obj : m_objects) {
            if (obj->isActive()) {
                Vec2 pos = obj->getPosition();
                Vec2 vel = obj->getVelocity();
                float impact = vel.length();

                if (surface->isPointNear(pos) && impact > 5.0f) {
                    surface->applyImpact(pos, impact);
                }
            }
        }
    }
}

} // namespace Vectoria
