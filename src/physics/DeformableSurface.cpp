#include "Vectoria/physics/DeformableSurface.h"
#include "Vectoria/Constants.h"
#include <cmath>

namespace Vectoria {

DeformableSurface::DeformableSurface(const Vec2& position, float width, float height, int resolutionX, int resolutionY)
    : m_position(position)
    , m_width(width)
    , m_height(height)
    , m_resolutionX(resolutionX)
    , m_resolutionY(resolutionY)
{
    createMesh();
    createSprings();
}

void DeformableSurface::createMesh() {
    m_nodes.clear();
    
    float cellWidth = m_width / (m_resolutionX - 1);
    float cellHeight = m_height / (m_resolutionY - 1);

    for (int y = 0; y < m_resolutionY; ++y) {
        for (int x = 0; x < m_resolutionX; ++x) {
            SurfaceNode node;
            node.position = Vec2(
                m_position.x - m_width / 2 + x * cellWidth,
                m_position.y - m_height / 2 + y * cellHeight
            );
            node.restPosition = node.position;
            node.velocity = Vec2(0, 0);
            node.mass = 1.0f;
            
            // Fix edge nodes
            node.isFixed = (y == 0 || y == m_resolutionY - 1 || x == 0 || x == m_resolutionX - 1);
            
            m_nodes.push_back(node);
        }
    }
}

void DeformableSurface::createSprings() {
    m_springs.clear();

    float cellWidth = m_width / (m_resolutionX - 1);
    float cellHeight = m_height / (m_resolutionY - 1);

    for (int y = 0; y < m_resolutionY; ++y) {
        for (int x = 0; x < m_resolutionX; ++x) {
            int index = y * m_resolutionX + x;

            // Horizontal spring
            if (x < m_resolutionX - 1) {
                SurfaceSpring spring;
                spring.nodeA = index;
                spring.nodeB = index + 1;
                spring.restLength = cellWidth;
                spring.stiffness = m_defaultStiffness;
                spring.damping = m_defaultDamping;
                m_springs.push_back(spring);
            }

            // Vertical spring
            if (y < m_resolutionY - 1) {
                SurfaceSpring spring;
                spring.nodeA = index;
                spring.nodeB = index + m_resolutionX;
                spring.restLength = cellHeight;
                spring.stiffness = m_defaultStiffness;
                spring.damping = m_defaultDamping;
                m_springs.push_back(spring);
            }

            // Diagonal springs for stability
            if (x < m_resolutionX - 1 && y < m_resolutionY - 1) {
                float diagLength = std::sqrt(cellWidth * cellWidth + cellHeight * cellHeight);
                
                SurfaceSpring spring1;
                spring1.nodeA = index;
                spring1.nodeB = index + m_resolutionX + 1;
                spring1.restLength = diagLength;
                spring1.stiffness = m_defaultStiffness * 0.5f;
                spring1.damping = m_defaultDamping * 0.5f;
                m_springs.push_back(spring1);

                SurfaceSpring spring2;
                spring2.nodeA = index + 1;
                spring2.nodeB = index + m_resolutionX;
                spring2.restLength = diagLength;
                spring2.stiffness = m_defaultStiffness * 0.5f;
                spring2.damping = m_defaultDamping * 0.5f;
                m_springs.push_back(spring2);
            }
        }
    }
}

void DeformableSurface::update(float deltaTime) {
    // Multiple physics substeps for stability
    int substeps = 4;
    float subDt = deltaTime / substeps;

    for (int s = 0; s < substeps; ++s) {
        updatePhysics(subDt);
        applyConstraints();
    }
}

void DeformableSurface::updatePhysics(float deltaTime) {
    // Apply spring forces
    for (const auto& spring : m_springs) {
        SurfaceNode& nodeA = m_nodes[spring.nodeA];
        SurfaceNode& nodeB = m_nodes[spring.nodeB];

        Vec2 delta = nodeB.position - nodeA.position;
        float currentLength = delta.length();
        
        if (currentLength < 0.0001f) continue;

        Vec2 direction = delta / currentLength;
        float displacement = currentLength - spring.restLength;

        // Spring force (Hooke's law)
        Vec2 springForce = direction * displacement * spring.stiffness;

        // Damping force
        Vec2 relativeVelocity = nodeB.velocity - nodeA.velocity;
        Vec2 dampingForce = direction * relativeVelocity.dot(direction) * spring.damping;

        Vec2 totalForce = springForce + dampingForce;

        if (!nodeA.isFixed) {
            nodeA.velocity += totalForce * (deltaTime / nodeA.mass);
        }
        if (!nodeB.isFixed) {
            nodeB.velocity -= totalForce * (deltaTime / nodeB.mass);
        }
    }

    // Apply return-to-rest force
    for (auto& node : m_nodes) {
        if (!node.isFixed) {
            Vec2 toRest = node.restPosition - node.position;
            node.velocity += toRest * (m_elasticity * deltaTime);
            node.velocity *= 0.98f; // Damping
        }
    }

    // Integrate velocities
    for (auto& node : m_nodes) {
        if (!node.isFixed) {
            node.position += node.velocity * deltaTime;
        }
    }
}

void DeformableSurface::applyConstraints() {
    // Keep fixed nodes in place
    for (auto& node : m_nodes) {
        if (node.isFixed) {
            node.position = node.restPosition;
            node.velocity = Vec2(0, 0);
        }
    }
}

void DeformableSurface::applyImpact(const Vec2& point, float force) {
    float impactRadius = 50.0f;
    float impactRadiusSq = impactRadius * impactRadius;

    for (auto& node : m_nodes) {
        if (node.isFixed) continue;

        Vec2 toNode = node.position - point;
        float distSq = toNode.lengthSquared();

        if (distSq < impactRadiusSq && distSq > 0.0001f) {
            float dist = std::sqrt(distSq);
            float falloff = 1.0f - (dist / impactRadius);
            falloff = falloff * falloff;

            Vec2 impulse = toNode.normalized() * force * falloff * 0.5f;
            node.velocity += impulse;
        }
    }
}

void DeformableSurface::reset() {
    for (auto& node : m_nodes) {
        node.position = node.restPosition;
        node.velocity = Vec2(0, 0);
    }
}

void DeformableSurface::attachToWorld(b2World* world) {
    m_world = world;
    // Could create Box2D bodies for each node for full integration
}

void DeformableSurface::detachFromWorld() {
    for (auto* body : m_bodies) {
        if (m_world) {
            m_world->DestroyBody(body);
        }
    }
    m_bodies.clear();
    m_world = nullptr;
}

float DeformableSurface::getDeformationAt(const Vec2& point) const {
    // Find nearest node and return its deformation
    float minDist = std::numeric_limits<float>::max();
    float deformation = 0.0f;

    for (const auto& node : m_nodes) {
        float dist = (node.position - point).length();
        if (dist < minDist) {
            minDist = dist;
            deformation = (node.position - node.restPosition).length();
        }
    }

    return deformation;
}

float DeformableSurface::getTotalDeformation() const {
    float total = 0.0f;
    for (const auto& node : m_nodes) {
        total += (node.position - node.restPosition).length();
    }
    return total;
}

void DeformableSurface::setStiffness(float stiffness) {
    m_defaultStiffness = stiffness;
    for (auto& spring : m_springs) {
        spring.stiffness = stiffness;
    }
}

void DeformableSurface::setDamping(float damping) {
    m_defaultDamping = damping;
    for (auto& spring : m_springs) {
        spring.damping = damping;
    }
}

bool DeformableSurface::isPointNear(const Vec2& point, float threshold) const {
    for (const auto& node : m_nodes) {
        if ((node.position - point).length() < threshold) {
            return true;
        }
    }
    return false;
}

Vec2 DeformableSurface::getNearestPoint(const Vec2& point) const {
    float minDist = std::numeric_limits<float>::max();
    Vec2 nearest = point;

    for (const auto& node : m_nodes) {
        float dist = (node.position - point).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = node.position;
        }
    }

    return nearest;
}

} // namespace Vectoria
