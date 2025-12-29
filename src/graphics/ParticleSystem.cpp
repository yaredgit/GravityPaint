#include "GravityPaint/graphics/ParticleSystem.h"
#include "GravityPaint/graphics/Renderer.h"
#include "GravityPaint/Constants.h"

namespace GravityPaint {

ParticleEmitter::ParticleEmitter(const EmitterConfig& config)
    : m_config(config)
    , m_rng(std::random_device{}())
{
    m_particles.resize(static_cast<size_t>(config.emissionRate * config.lifetimeMax * 2));
}

void ParticleEmitter::update(float deltaTime) {
    if (!m_active) return;

    // Emit new particles
    if (m_config.continuous) {
        m_emissionAccumulator += m_config.emissionRate * deltaTime;
        while (m_emissionAccumulator >= 1.0f) {
            emitParticle();
            m_emissionAccumulator -= 1.0f;
        }
    }

    // Update existing particles
    m_activeCount = 0;
    for (auto& particle : m_particles) {
        if (!particle.active) continue;

        particle.lifetime += deltaTime;
        if (particle.lifetime >= particle.maxLifetime) {
            particle.active = false;
            continue;
        }

        float t = particle.lifetime / particle.maxLifetime;

        // Update physics
        particle.velocity += particle.acceleration * deltaTime;
        particle.position += particle.velocity * deltaTime;
        particle.rotation += particle.rotationSpeed * deltaTime;

        // Interpolate visual properties
        particle.color = Color::lerp(particle.startColor, particle.endColor, t);
        particle.size = particle.startSize + (particle.endSize - particle.startSize) * t;

        m_activeCount++;
    }
}

void ParticleEmitter::emit(int count) {
    for (int i = 0; i < count; ++i) {
        emitParticle();
    }
}

void ParticleEmitter::burst(int count) {
    emit(count);
}

void ParticleEmitter::stop() {
    m_active = false;
}

void ParticleEmitter::reset() {
    for (auto& particle : m_particles) {
        particle.active = false;
    }
    m_activeCount = 0;
    m_emissionAccumulator = 0;
    m_active = true;
}

void ParticleEmitter::emitParticle() {
    // Find inactive particle
    Particle* particle = nullptr;
    for (auto& p : m_particles) {
        if (!p.active) {
            particle = &p;
            break;
        }
    }

    if (!particle) return; // Pool exhausted

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 6.28318f);

    particle->active = true;
    particle->position = getEmissionPoint();
    
    // Random velocity within bounds
    particle->velocity = Vec2(
        m_config.velocityMin.x + (m_config.velocityMax.x - m_config.velocityMin.x) * dist01(m_rng),
        m_config.velocityMin.y + (m_config.velocityMax.y - m_config.velocityMin.y) * dist01(m_rng)
    );
    
    particle->acceleration = m_config.acceleration;
    
    particle->lifetime = 0;
    particle->maxLifetime = m_config.lifetimeMin + (m_config.lifetimeMax - m_config.lifetimeMin) * dist01(m_rng);
    
    particle->startSize = m_config.sizeStart;
    particle->endSize = m_config.sizeEnd;
    particle->size = particle->startSize;

    if (m_config.randomizeColor && !m_config.colorPalette.empty()) {
        std::uniform_int_distribution<size_t> colorDist(0, m_config.colorPalette.size() - 1);
        particle->startColor = m_config.colorPalette[colorDist(m_rng)];
        particle->endColor = particle->startColor;
        particle->endColor.a = 0;
    } else {
        particle->startColor = m_config.colorStart;
        particle->endColor = m_config.colorEnd;
    }
    particle->color = particle->startColor;

    particle->rotation = m_config.rotationMin + (m_config.rotationMax - m_config.rotationMin) * dist01(m_rng);
    particle->rotationSpeed = m_config.rotationSpeedMin + (m_config.rotationSpeedMax - m_config.rotationSpeedMin) * dist01(m_rng);

    m_activeCount++;
}

Vec2 ParticleEmitter::getEmissionPoint() {
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 6.28318f);

    switch (m_config.shape) {
        case EmitterShape::Point:
            return m_config.position;

        case EmitterShape::Circle: {
            float angle = distAngle(m_rng);
            float radius = m_config.shapeRadius * std::sqrt(dist01(m_rng));
            return m_config.position + Vec2(std::cos(angle), std::sin(angle)) * radius;
        }

        case EmitterShape::Rectangle:
            return Vec2(
                m_config.position.x + (dist01(m_rng) - 0.5f) * m_config.shapeSize.x,
                m_config.position.y + (dist01(m_rng) - 0.5f) * m_config.shapeSize.y
            );

        case EmitterShape::Line:
            return Vec2(
                m_config.position.x + (dist01(m_rng) - 0.5f) * m_config.shapeSize.x,
                m_config.position.y
            );
    }

    return m_config.position;
}

// ParticleSystem implementation
ParticleSystem::ParticleSystem() = default;

void ParticleSystem::update(float deltaTime) {
    // Update permanent emitters
    for (auto& emitter : m_emitters) {
        emitter->update(deltaTime);
    }

    // Update and clean up temporary emitters
    for (auto it = m_tempEmitters.begin(); it != m_tempEmitters.end();) {
        (*it)->update(deltaTime);
        
        if ((*it)->getActiveCount() == 0 && !(*it)->isActive()) {
            it = m_tempEmitters.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::render(Renderer* renderer) {
    auto renderEmitter = [renderer](ParticleEmitter* emitter) {
        for (const auto& particle : emitter->getParticles()) {
            if (!particle.active) continue;

            renderer->drawCircle(
                particle.position,
                particle.size,
                particle.color,
                true,
                8
            );
        }
    };

    for (auto& emitter : m_emitters) {
        renderEmitter(emitter.get());
    }

    for (auto& emitter : m_tempEmitters) {
        renderEmitter(emitter.get());
    }
}

void ParticleSystem::clear() {
    m_emitters.clear();
    m_tempEmitters.clear();
}

ParticleEmitter* ParticleSystem::createEmitter(const EmitterConfig& config) {
    auto emitter = std::make_unique<ParticleEmitter>(config);
    ParticleEmitter* ptr = emitter.get();
    m_emitters.push_back(std::move(emitter));
    return ptr;
}

void ParticleSystem::removeEmitter(ParticleEmitter* emitter) {
    for (auto it = m_emitters.begin(); it != m_emitters.end(); ++it) {
        if (it->get() == emitter) {
            m_emitters.erase(it);
            return;
        }
    }
}

void ParticleSystem::spawnCollisionEffect(const Vec2& position, const Color& color) {
    EmitterConfig config;
    config.position = position;
    config.shape = EmitterShape::Point;
    config.emissionRate = 0;
    config.burstCount = 15;
    config.continuous = false;
    config.velocityMin = Vec2(-100, -100);
    config.velocityMax = Vec2(100, 100);
    config.acceleration = Vec2(0, 100);
    config.lifetimeMin = 0.3f;
    config.lifetimeMax = 0.6f;
    config.sizeStart = 8.0f;
    config.sizeEnd = 2.0f;
    config.colorStart = color;
    config.colorEnd = Color(color.r, color.g, color.b, 0);

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(15);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

void ParticleSystem::spawnGoalEffect(const Vec2& position) {
    EmitterConfig config;
    config.position = position;
    config.shape = EmitterShape::Circle;
    config.shapeRadius = 30.0f;
    config.emissionRate = 0;
    config.continuous = false;
    config.velocityMin = Vec2(-50, -150);
    config.velocityMax = Vec2(50, -50);
    config.acceleration = Vec2(0, -50);
    config.lifetimeMin = 0.5f;
    config.lifetimeMax = 1.0f;
    config.sizeStart = 12.0f;
    config.sizeEnd = 4.0f;
    config.colorStart = Color::green();
    config.colorEnd = Color(100, 255, 100, 0);

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(30);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

void ParticleSystem::spawnTrailEffect(const Vec2& position, const Vec2& velocity, const Color& color) {
    EmitterConfig config;
    config.position = position;
    config.shape = EmitterShape::Point;
    config.emissionRate = 0;
    config.continuous = false;
    config.velocityMin = velocity * -0.3f + Vec2(-20, -20);
    config.velocityMax = velocity * -0.1f + Vec2(20, 20);
    config.acceleration = Vec2(0, 0);
    config.lifetimeMin = 0.1f;
    config.lifetimeMax = 0.3f;
    config.sizeStart = 6.0f;
    config.sizeEnd = 1.0f;
    config.colorStart = Color(color.r, color.g, color.b, 150);
    config.colorEnd = Color(color.r, color.g, color.b, 0);

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(3);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

void ParticleSystem::spawnGravityFieldEffect(const Vec2& position, float radius) {
    EmitterConfig config;
    config.position = position;
    config.shape = EmitterShape::Circle;
    config.shapeRadius = radius;
    config.emissionRate = 0;
    config.continuous = false;
    config.velocityMin = Vec2(-10, -10);
    config.velocityMax = Vec2(10, 10);
    config.acceleration = Vec2(0, 0);
    config.lifetimeMin = 0.3f;
    config.lifetimeMax = 0.5f;
    config.sizeStart = 4.0f;
    config.sizeEnd = 1.0f;
    config.colorStart = Color::cyan();
    config.colorEnd = Color(0, 255, 255, 0);

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(20);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

void ParticleSystem::spawnEnergyTransferEffect(const Vec2& from, const Vec2& to, const Color& color) {
    Vec2 direction = (to - from).normalized();
    float distance = (to - from).length();

    EmitterConfig config;
    config.position = from;
    config.shape = EmitterShape::Point;
    config.emissionRate = 0;
    config.continuous = false;
    config.velocityMin = direction * distance * 2.0f;
    config.velocityMax = direction * distance * 3.0f;
    config.acceleration = Vec2(0, 0);
    config.lifetimeMin = 0.2f;
    config.lifetimeMax = 0.4f;
    config.sizeStart = 6.0f;
    config.sizeEnd = 2.0f;
    config.colorStart = color;
    config.colorEnd = Color(color.r, color.g, color.b, 0);

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(10);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

void ParticleSystem::spawnExplosionEffect(const Vec2& position, const Color& color, int particleCount) {
    EmitterConfig config;
    config.position = position;
    config.shape = EmitterShape::Point;
    config.emissionRate = 0;
    config.continuous = false;
    config.velocityMin = Vec2(-200, -200);
    config.velocityMax = Vec2(200, 200);
    config.acceleration = Vec2(0, 150);
    config.lifetimeMin = 0.5f;
    config.lifetimeMax = 1.2f;
    config.sizeStart = 15.0f;
    config.sizeEnd = 3.0f;
    config.colorStart = color;
    config.colorEnd = Color(color.r, color.g, color.b, 0);
    config.rotationSpeedMin = -360.0f;
    config.rotationSpeedMax = 360.0f;

    auto emitter = std::make_unique<ParticleEmitter>(config);
    emitter->burst(particleCount);
    emitter->stop();
    m_tempEmitters.push_back(std::move(emitter));
}

int ParticleSystem::getTotalParticleCount() const {
    int total = 0;
    for (const auto& emitter : m_emitters) {
        total += emitter->getActiveCount();
    }
    for (const auto& emitter : m_tempEmitters) {
        total += emitter->getActiveCount();
    }
    return total;
}

} // namespace GravityPaint
