#pragma once

#include "Vectoria/Types.h"
#include "Vectoria/Constants.h"
#include <vector>
#include <random>

namespace Vectoria {

struct Particle {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Color color;
    Color startColor;
    Color endColor;
    float size;
    float startSize;
    float endSize;
    float lifetime;
    float maxLifetime;
    float rotation;
    float rotationSpeed;
    bool active = false;
};

enum class EmitterShape {
    Point,
    Circle,
    Rectangle,
    Line
};

struct EmitterConfig {
    Vec2 position;
    EmitterShape shape = EmitterShape::Point;
    float shapeRadius = 10.0f;
    Vec2 shapeSize = Vec2(100, 100);

    // Emission
    float emissionRate = 50.0f;  // particles per second
    int burstCount = 0;          // for burst emission
    bool continuous = true;

    // Particle properties
    Vec2 velocityMin = Vec2(-50, -50);
    Vec2 velocityMax = Vec2(50, 50);
    Vec2 acceleration = Vec2(0, 50);  // gravity-like
    
    float lifetimeMin = 0.5f;
    float lifetimeMax = 1.5f;
    
    float sizeStart = 10.0f;
    float sizeEnd = 2.0f;
    
    Color colorStart = Color::white();
    Color colorEnd = Color(255, 255, 255, 0);
    
    float rotationMin = 0.0f;
    float rotationMax = 360.0f;
    float rotationSpeedMin = -90.0f;
    float rotationSpeedMax = 90.0f;

    // Variation
    bool randomizeColor = false;
    std::vector<Color> colorPalette;
};

class ParticleEmitter {
public:
    ParticleEmitter(const EmitterConfig& config);
    ~ParticleEmitter() = default;

    void update(float deltaTime);
    void emit(int count = 1);
    void burst(int count);
    void stop();
    void reset();

    void setPosition(const Vec2& position) { m_config.position = position; }
    Vec2 getPosition() const { return m_config.position; }
    
    void setConfig(const EmitterConfig& config) { m_config = config; }
    const EmitterConfig& getConfig() const { return m_config; }

    const std::vector<Particle>& getParticles() const { return m_particles; }
    int getActiveCount() const { return m_activeCount; }
    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

private:
    void emitParticle();
    Vec2 getEmissionPoint();
    
    EmitterConfig m_config;
    std::vector<Particle> m_particles;
    int m_activeCount = 0;
    bool m_active = true;
    float m_emissionAccumulator = 0.0f;

    std::mt19937 m_rng;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem() = default;

    void update(float deltaTime);
    void render(class Renderer* renderer);
    void clear();

    // Emitter management
    ParticleEmitter* createEmitter(const EmitterConfig& config);
    void removeEmitter(ParticleEmitter* emitter);

    // Preset effects
    void spawnCollisionEffect(const Vec2& position, const Color& color);
    void spawnGoalEffect(const Vec2& position);
    void spawnTrailEffect(const Vec2& position, const Vec2& velocity, const Color& color);
    void spawnGravityFieldEffect(const Vec2& position, float radius);
    void spawnEnergyTransferEffect(const Vec2& from, const Vec2& to, const Color& color);
    void spawnExplosionEffect(const Vec2& position, const Color& color, int particleCount = 50);

    int getTotalParticleCount() const;

private:
    std::vector<std::unique_ptr<ParticleEmitter>> m_emitters;
    std::vector<std::unique_ptr<ParticleEmitter>> m_tempEmitters;  // one-shot effects
};

} // namespace Vectoria
