#pragma once

#include "Vectoria/Types.h"
#include "Vectoria/Constants.h"
#include <box2d/box2d.h>
#include <vector>
#include <memory>

namespace Vectoria {

class PhysicsObject;
class GravityField;
class DeformableSurface;

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    void setCollisionCallback(CollisionCallback callback) { m_callback = callback; }

private:
    CollisionCallback m_callback;
};

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void reset();

    // Object management
    PhysicsObject* createObject(ObjectType type, const Vec2& position, float size = 1.0f);
    void destroyObject(PhysicsObject* object);
    void clearObjects();

    // Gravity fields
    GravityField* createGravityField(const Vec2& position, const Vec2& direction, float strength, float radius);
    void removeGravityField(GravityField* field);
    void applyGravityFromStrokes(const std::vector<GravityStroke>& strokes);
    void clearGravityFields();

    // Deformable surfaces
    DeformableSurface* createDeformableSurface(const Vec2& position, float width, float height);
    void removeDeformableSurface(DeformableSurface* surface);

    // World boundaries
    void createBoundaries(float width, float height);
    void destroyBoundaries();

    // Static obstacles
    b2Body* createStaticBox(const Vec2& position, const Vec2& size);
    b2Body* createStaticCircle(const Vec2& position, float radius);
    void destroyStaticBody(b2Body* body);

    // Goal zones
    void createGoalZone(const Vec2& position, const Vec2& size);
    bool isObjectInGoal(PhysicsObject* object) const;
    std::vector<PhysicsObject*> getObjectsInGoal() const;

    // Queries
    PhysicsObject* getObjectAtPoint(const Vec2& point);
    std::vector<PhysicsObject*> getObjectsInArea(const Rect& area);

    // Accessors
    b2World* getBox2DWorld() const { return m_world.get(); }
    const std::vector<std::unique_ptr<PhysicsObject>>& getObjects() const { return m_objects; }
    Vec2 getGlobalGravity() const { return m_globalGravity; }
    void setGlobalGravity(const Vec2& gravity);

    // Collision callbacks
    void setCollisionCallback(CollisionCallback callback);

    // Debug
    void setDebugDraw(bool enable) { m_debugDraw = enable; }
    bool isDebugDrawEnabled() const { return m_debugDraw; }

    // Coordinate conversion
    static Vec2 toPixels(const b2Vec2& meters);
    static b2Vec2 toMeters(const Vec2& pixels);

private:
    void applyGravityFields();
    void updateDeformableSurfaces(float deltaTime);

    std::unique_ptr<b2World> m_world;
    std::unique_ptr<ContactListener> m_contactListener;

    std::vector<std::unique_ptr<PhysicsObject>> m_objects;
    std::vector<std::unique_ptr<GravityField>> m_gravityFields;
    std::vector<std::unique_ptr<DeformableSurface>> m_deformableSurfaces;
    std::vector<b2Body*> m_boundaryBodies;
    std::vector<b2Body*> m_staticBodies;

    Rect m_goalZone;
    bool m_hasGoalZone = false;

    Vec2 m_globalGravity = Vec2(DEFAULT_GRAVITY_X, DEFAULT_GRAVITY_Y);
    bool m_debugDraw = false;

    float m_accumulator = 0.0f;
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
};

} // namespace Vectoria
