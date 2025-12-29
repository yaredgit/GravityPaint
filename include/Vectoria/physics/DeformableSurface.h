#pragma once

#include "Vectoria/Types.h"
#include <box2d/box2d.h>
#include <vector>

namespace Vectoria {

struct SurfaceNode {
    Vec2 position;
    Vec2 restPosition;
    Vec2 velocity;
    float mass = 1.0f;
    bool isFixed = false;
};

struct SurfaceSpring {
    int nodeA;
    int nodeB;
    float restLength;
    float stiffness;
    float damping;
};

class DeformableSurface {
public:
    DeformableSurface(const Vec2& position, float width, float height, int resolutionX = 10, int resolutionY = 5);
    ~DeformableSurface() = default;

    void update(float deltaTime);
    void applyImpact(const Vec2& point, float force);
    void reset();

    // Physics integration
    void attachToWorld(b2World* world);
    void detachFromWorld();

    // Accessors
    Vec2 getPosition() const { return m_position; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

    const std::vector<SurfaceNode>& getNodes() const { return m_nodes; }
    const std::vector<SurfaceSpring>& getSprings() const { return m_springs; }

    // Visual
    Color getColor() const { return m_color; }
    void setColor(const Color& color) { m_color = color; }

    // Get deformation at a point (for scoring)
    float getDeformationAt(const Vec2& point) const;
    float getTotalDeformation() const;

    // Surface properties
    void setStiffness(float stiffness);
    void setDamping(float damping);
    void setElasticity(float elasticity) { m_elasticity = elasticity; }
    float getElasticity() const { return m_elasticity; }

    // Check if a point is near the surface
    bool isPointNear(const Vec2& point, float threshold = 20.0f) const;
    Vec2 getNearestPoint(const Vec2& point) const;

private:
    void createMesh();
    void createSprings();
    void updatePhysics(float deltaTime);
    void applyConstraints();
    
    Vec2 m_position;
    float m_width;
    float m_height;
    int m_resolutionX;
    int m_resolutionY;

    std::vector<SurfaceNode> m_nodes;
    std::vector<SurfaceSpring> m_springs;
    std::vector<b2Body*> m_bodies;

    b2World* m_world = nullptr;

    Color m_color = Color::cyan();
    float m_elasticity = 0.8f;
    float m_defaultStiffness = 500.0f;
    float m_defaultDamping = 10.0f;
};

} // namespace Vectoria
