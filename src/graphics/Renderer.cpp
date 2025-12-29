#include "GravityPaint/graphics/Renderer.h"
#include "GravityPaint/graphics/Camera.h"
#include "GravityPaint/physics/PhysicsObject.h"
#include "GravityPaint/physics/GravityField.h"
#include "GravityPaint/physics/DeformableSurface.h"
#include "GravityPaint/Constants.h"
#include <cmath>
#include <random>

namespace GravityPaint {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::initialize(SDL_Window* window, int width, int height) {
    m_width = width;
    m_height = height;

    m_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
    }

    // Try to load a font
    m_fontPath = "assets/fonts/default.ttf";

    return true;
}

void Renderer::shutdown() {
    // Clean up fonts
    for (auto& pair : m_fonts) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    m_fonts.clear();
    TTF_Quit();

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
}

void Renderer::beginFrame() {
    // Nothing special needed
}

void Renderer::endFrame() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::clear(const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::drawPoint(const Vec2& position, const Color& color, float size) {
    Vec2 screenPos = worldToScreen(position);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    if (size <= 1.0f) {
        SDL_RenderDrawPointF(m_renderer, screenPos.x, screenPos.y);
    } else {
        drawCircle(position, size / 2, color, true, 8);
    }
}

void Renderer::drawLine(const Vec2& start, const Vec2& end, const Color& color, float thickness) {
    Vec2 screenStart = worldToScreen(start);
    Vec2 screenEnd = worldToScreen(end);
    
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    if (thickness <= 1.0f) {
        SDL_RenderDrawLineF(m_renderer, screenStart.x, screenStart.y, screenEnd.x, screenEnd.y);
    } else {
        // Draw thick line using multiple parallel lines
        Vec2 dir = (screenEnd - screenStart).normalized();
        Vec2 perp(-dir.y, dir.x);
        
        for (float offset = -thickness / 2; offset <= thickness / 2; offset += 1.0f) {
            Vec2 offsetVec = perp * offset;
            SDL_RenderDrawLineF(m_renderer,
                screenStart.x + offsetVec.x, screenStart.y + offsetVec.y,
                screenEnd.x + offsetVec.x, screenEnd.y + offsetVec.y);
        }
    }
}

void Renderer::drawRect(const Rect& rect, const Color& color, bool filled) {
    Vec2 topLeft = worldToScreen(Vec2(rect.x, rect.y));
    
    SDL_FRect sdlRect;
    sdlRect.x = topLeft.x;
    sdlRect.y = topLeft.y;
    sdlRect.w = rect.w;
    sdlRect.h = rect.h;

    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    if (filled) {
        SDL_RenderFillRectF(m_renderer, &sdlRect);
    } else {
        SDL_RenderDrawRectF(m_renderer, &sdlRect);
    }
}

void Renderer::drawCircle(const Vec2& center, float radius, const Color& color, bool filled, int segments) {
    if (filled) {
        drawFilledCircle(center, radius, color, segments);
    } else {
        drawCircleOutline(center, radius, color, segments);
    }
}

void Renderer::drawFilledCircle(const Vec2& center, float radius, const Color& color, int segments) {
    Vec2 screenCenter = worldToScreen(center);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    // Draw filled circle using horizontal lines
    for (int y = -static_cast<int>(radius); y <= static_cast<int>(radius); y++) {
        float halfWidth = std::sqrt(radius * radius - y * y);
        SDL_RenderDrawLineF(m_renderer,
            screenCenter.x - halfWidth, screenCenter.y + y,
            screenCenter.x + halfWidth, screenCenter.y + y);
    }
}

void Renderer::drawCircleOutline(const Vec2& center, float radius, const Color& color, int segments) {
    Vec2 screenCenter = worldToScreen(center);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    float angleStep = 2.0f * 3.14159f / segments;
    float prevX = screenCenter.x + radius;
    float prevY = screenCenter.y;

    for (int i = 1; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = screenCenter.x + std::cos(angle) * radius;
        float y = screenCenter.y + std::sin(angle) * radius;
        SDL_RenderDrawLineF(m_renderer, prevX, prevY, x, y);
        prevX = x;
        prevY = y;
    }
}

void Renderer::drawTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& color, bool filled) {
    std::vector<Vec2> points = {p1, p2, p3};
    drawPolygon(points, color, filled);
}

void Renderer::drawPolygon(const std::vector<Vec2>& points, const Color& color, bool filled) {
    if (points.size() < 3) return;

    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    if (filled) {
        // Simple triangle fan for convex polygons
        Vec2 center = worldToScreen(points[0]);
        for (size_t i = 1; i < points.size() - 1; ++i) {
            Vec2 p1 = worldToScreen(points[i]);
            Vec2 p2 = worldToScreen(points[i + 1]);
            
            // Draw filled triangle
            SDL_Vertex vertices[3];
            vertices[0].position = {center.x, center.y};
            vertices[0].color = {color.r, color.g, color.b, color.a};
            vertices[1].position = {p1.x, p1.y};
            vertices[1].color = {color.r, color.g, color.b, color.a};
            vertices[2].position = {p2.x, p2.y};
            vertices[2].color = {color.r, color.g, color.b, color.a};
            
            SDL_RenderGeometry(m_renderer, nullptr, vertices, 3, nullptr, 0);
        }
    } else {
        // Draw outline
        for (size_t i = 0; i < points.size(); ++i) {
            Vec2 start = worldToScreen(points[i]);
            Vec2 end = worldToScreen(points[(i + 1) % points.size()]);
            SDL_RenderDrawLineF(m_renderer, start.x, start.y, end.x, end.y);
        }
    }
}

void Renderer::drawGradientRect(const Rect& rect, const Color& topLeft, const Color& topRight,
                                 const Color& bottomLeft, const Color& bottomRight) {
    Vec2 tl = worldToScreen(Vec2(rect.x, rect.y));
    Vec2 tr = worldToScreen(Vec2(rect.x + rect.w, rect.y));
    Vec2 bl = worldToScreen(Vec2(rect.x, rect.y + rect.h));
    Vec2 br = worldToScreen(Vec2(rect.x + rect.w, rect.y + rect.h));

    SDL_Vertex vertices[4];
    vertices[0].position = {tl.x, tl.y};
    vertices[0].color = {topLeft.r, topLeft.g, topLeft.b, topLeft.a};
    vertices[1].position = {tr.x, tr.y};
    vertices[1].color = {topRight.r, topRight.g, topRight.b, topRight.a};
    vertices[2].position = {br.x, br.y};
    vertices[2].color = {bottomRight.r, bottomRight.g, bottomRight.b, bottomRight.a};
    vertices[3].position = {bl.x, bl.y};
    vertices[3].color = {bottomLeft.r, bottomLeft.g, bottomLeft.b, bottomLeft.a};

    int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(m_renderer, nullptr, vertices, 4, indices, 6);
}

void Renderer::drawBezierCurve(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3,
                                const Color& color, int segments) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    Vec2 prev = worldToScreen(p0);
    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        float u = 1.0f - t;
        
        Vec2 point = p0 * (u * u * u) + p1 * (3 * u * u * t) + p2 * (3 * u * t * t) + p3 * (t * t * t);
        Vec2 screenPoint = worldToScreen(point);
        
        SDL_RenderDrawLineF(m_renderer, prev.x, prev.y, screenPoint.x, screenPoint.y);
        prev = screenPoint;
    }
}

void Renderer::drawDashedLine(const Vec2& start, const Vec2& end, const Color& color,
                               float dashLength, float gapLength) {
    Vec2 dir = (end - start);
    float totalLength = dir.length();
    dir = dir.normalized();

    float current = 0;
    bool drawing = true;

    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    while (current < totalLength) {
        float segmentLength = drawing ? dashLength : gapLength;
        float nextPos = std::min(current + segmentLength, totalLength);

        if (drawing) {
            Vec2 segStart = worldToScreen(start + dir * current);
            Vec2 segEnd = worldToScreen(start + dir * nextPos);
            SDL_RenderDrawLineF(m_renderer, segStart.x, segStart.y, segEnd.x, segEnd.y);
        }

        current = nextPos;
        drawing = !drawing;
    }
}

void Renderer::drawPhysicsObject(const PhysicsObject* object) {
    if (!object || !object->isActive()) return;

    Vec2 pos = object->getPosition();
    float size = object->getSize() * 20.0f;
    float angle = object->getAngle();
    Color color = object->getColor();

    // Draw trail first (behind object)
    drawTrail(object->getTrail(), Color(color.r, color.g, color.b, 100));

    // Draw energy glow
    float energyRatio = object->getEnergy() / MAX_OBJECT_ENERGY;
    if (energyRatio > 0.3f) {
        Color glowColor = object->getEnergyColor();
        glowColor.a = static_cast<uint8_t>(energyRatio * 100);
        drawCircle(pos, size * 1.5f, glowColor, true);
    }

    // Draw object based on type
    switch (object->getType()) {
        case ObjectType::Ball:
        case ObjectType::Blob:
            drawCircle(pos, size, color, true);
            drawCircle(pos, size, Color::white(), false);
            break;

        case ObjectType::Box: {
            // Rotated box
            std::vector<Vec2> vertices(4);
            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);
            
            vertices[0] = pos + Vec2(-size * cos_a + size * sin_a, -size * sin_a - size * cos_a);
            vertices[1] = pos + Vec2(size * cos_a + size * sin_a, size * sin_a - size * cos_a);
            vertices[2] = pos + Vec2(size * cos_a - size * sin_a, size * sin_a + size * cos_a);
            vertices[3] = pos + Vec2(-size * cos_a - size * sin_a, -size * sin_a + size * cos_a);
            
            drawPolygon(vertices, color, true);
            drawPolygon(vertices, Color::white(), false);
            break;
        }

        case ObjectType::Triangle: {
            std::vector<Vec2> vertices(3);
            for (int i = 0; i < 3; ++i) {
                float a = angle + i * 2.0f * 3.14159f / 3.0f - 3.14159f / 2.0f;
                vertices[i] = pos + Vec2(std::cos(a), std::sin(a)) * size;
            }
            drawPolygon(vertices, color, true);
            drawPolygon(vertices, Color::white(), false);
            break;
        }

        case ObjectType::Star: {
            std::vector<Vec2> vertices(10);
            for (int i = 0; i < 10; ++i) {
                float a = angle + i * 3.14159f / 5.0f - 3.14159f / 2.0f;
                float r = (i % 2 == 0) ? size : size * 0.5f;
                vertices[i] = pos + Vec2(std::cos(a), std::sin(a)) * r;
            }
            drawPolygon(vertices, color, true);
            drawPolygon(vertices, Color::white(), false);
            break;
        }
    }

    // Draw small energy indicator
    drawEnergyBar(pos + Vec2(0, -size - 10), object->getEnergy(), MAX_OBJECT_ENERGY);
}

void Renderer::drawGravityField(const GravityField* field) {
    if (!field || !field->isActive()) return;

    Vec2 pos = field->getPosition();
    float radius = field->getRadius();
    Color color = field->getColor();

    // Pulsing alpha
    float pulse = 0.5f + 0.3f * std::sin(field->getPulsePhase());
    color.a = static_cast<uint8_t>(pulse * 150);

    // Draw range circle
    drawCircle(pos, radius, color, false);
    drawCircle(pos, radius * 0.7f, Color(color.r, color.g, color.b, color.a / 2), false);

    // Draw direction arrow
    drawVector(pos, field->getDirection(), field->getStrength() * 5.0f, color);
}

void Renderer::drawGravityStroke(const GravityStroke& stroke) {
    if (stroke.points.size() < 2) return;

    float alpha = stroke.getAlpha();
    Color color = stroke.color;
    color.a = static_cast<uint8_t>(alpha * 255);

    // Draw stroke path with gradient
    for (size_t i = 1; i < stroke.points.size(); ++i) {
        float t = static_cast<float>(i) / stroke.points.size();
        Color segColor = color;
        segColor.a = static_cast<uint8_t>(alpha * (1.0f - t * 0.5f) * 255);
        
        float thickness = 3.0f * (1.0f - t * 0.5f);
        drawLine(stroke.points[i - 1], stroke.points[i], segColor, thickness);
    }

    // Draw arrow at end showing direction
    if (stroke.points.size() >= 2) {
        Vec2 end = stroke.points.back();
        drawVector(end, stroke.direction, 30.0f * alpha, color);
    }

    // Draw glow at stroke center
    if (!stroke.points.empty()) {
        Vec2 center = stroke.points[stroke.points.size() / 2];
        Color glowColor = color;
        glowColor.a = static_cast<uint8_t>(alpha * 100);
        drawCircle(center, 20.0f * alpha, glowColor, true);
    }
}

void Renderer::drawDeformableSurface(const DeformableSurface* surface) {
    if (!surface) return;

    const auto& nodes = surface->getNodes();
    const auto& springs = surface->getSprings();
    Color color = surface->getColor();

    // Draw springs
    for (const auto& spring : springs) {
        Vec2 posA = nodes[spring.nodeA].position;
        Vec2 posB = nodes[spring.nodeB].position;
        drawLine(posA, posB, color, 2.0f);
    }

    // Draw nodes
    for (const auto& node : nodes) {
        Color nodeColor = node.isFixed ? Color::red() : color;
        drawCircle(node.position, 4.0f, nodeColor, true);
    }
}

void Renderer::drawGoalZone(const Rect& zone) {
    // Animated goal zone
    static float time = 0;
    time += 0.016f;

    // Background
    Color bgColor = Color::green();
    bgColor.a = static_cast<uint8_t>(50 + 30 * std::sin(time * 3.0f));
    drawRect(zone, bgColor, true);

    // Pulsing border
    Color borderColor = Color::green();
    borderColor.a = static_cast<uint8_t>(150 + 100 * std::sin(time * 5.0f));
    drawRect(zone, borderColor, false);

    // Inner glow lines
    float inset = 10.0f + 5.0f * std::sin(time * 2.0f);
    Rect innerZone(zone.x + inset, zone.y + inset, zone.w - inset * 2, zone.h - inset * 2);
    drawRect(innerZone, Color(100, 255, 100, 80), false);
}

void Renderer::drawTrail(const std::vector<Vec2>& trail, const Color& color) {
    if (trail.size() < 2) return;

    for (size_t i = 1; i < trail.size(); ++i) {
        float alpha = static_cast<float>(i) / trail.size();
        Color segColor = color;
        segColor.a = static_cast<uint8_t>(alpha * color.a);
        
        drawLine(trail[i - 1], trail[i], segColor, 2.0f * alpha);
    }
}

void Renderer::drawEnergyBar(const Vec2& position, float energy, float maxEnergy) {
    float width = 30.0f;
    float height = 4.0f;
    float ratio = energy / maxEnergy;

    Vec2 screenPos = worldToScreen(position);

    // Background
    SDL_FRect bgRect = {screenPos.x - width / 2, screenPos.y, width, height};
    SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 150);
    SDL_RenderFillRectF(m_renderer, &bgRect);

    // Energy fill
    Color energyColor = ratio > 0.5f ? Color::green() : (ratio > 0.25f ? Color::yellow() : Color::red());
    SDL_FRect fillRect = {screenPos.x - width / 2, screenPos.y, width * ratio, height};
    SDL_SetRenderDrawColor(m_renderer, energyColor.r, energyColor.g, energyColor.b, 200);
    SDL_RenderFillRectF(m_renderer, &fillRect);
}

void Renderer::drawVector(const Vec2& origin, const Vec2& direction, float magnitude, const Color& color) {
    Vec2 end = origin + direction * magnitude;
    drawLine(origin, end, color, 2.0f);

    // Arrow head
    float arrowSize = std::min(magnitude * 0.3f, 15.0f);
    Vec2 perpendicular(-direction.y, direction.x);
    
    Vec2 arrowLeft = end - direction * arrowSize + perpendicular * arrowSize * 0.5f;
    Vec2 arrowRight = end - direction * arrowSize - perpendicular * arrowSize * 0.5f;
    
    drawLine(end, arrowLeft, color, 2.0f);
    drawLine(end, arrowRight, color, 2.0f);
}

TTF_Font* Renderer::getFont(float size) {
    int sizeInt = static_cast<int>(size);
    auto it = m_fonts.find(sizeInt);
    if (it != m_fonts.end()) {
        return it->second;
    }
    
    TTF_Font* font = TTF_OpenFont(m_fontPath.c_str(), sizeInt);
    if (!font) {
        SDL_Log("Failed to load font %s at size %d: %s", m_fontPath.c_str(), sizeInt, TTF_GetError());
        return nullptr;
    }
    
    m_fonts[sizeInt] = font;
    return font;
}

void Renderer::drawText(const std::string& text, const Vec2& position, const Color& color, float size) {
    if (text.empty()) return;
    
    TTF_Font* font = getFont(size);
    if (!font) {
        // Fallback to rectangles if font not available
        Vec2 screenPos = worldToScreen(position);
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
        float charWidth = size * 0.6f;
        float x = screenPos.x;
        for (char c : text) {
            if (c != ' ') {
                SDL_FRect charRect = {x, screenPos.y, charWidth * 0.8f, size};
                SDL_RenderDrawRectF(m_renderer, &charRect);
            }
            x += charWidth;
        }
        return;
    }
    
    Vec2 screenPos = worldToScreen(position);
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        SDL_FRect destRect = {screenPos.x, screenPos.y, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        SDL_RenderCopyF(m_renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void Renderer::drawTextCentered(const std::string& text, const Vec2& position, const Color& color, float size) {
    if (text.empty()) return;
    
    TTF_Font* font = getFont(size);
    if (!font) {
        float totalWidth = text.length() * size * 0.6f;
        Vec2 startPos(position.x - totalWidth / 2, position.y - size / 2);
        drawText(text, startPos, color, size);
        return;
    }
    
    Vec2 screenPos = worldToScreen(position);
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        SDL_FRect destRect = {
            screenPos.x - surface->w / 2.0f,
            screenPos.y - surface->h / 2.0f,
            static_cast<float>(surface->w),
            static_cast<float>(surface->h)
        };
        SDL_RenderCopyF(m_renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void Renderer::drawTexture(SDL_Texture* texture, const Rect& destRect, float angle, const Color& tint) {
    if (!texture) return;

    SDL_FRect dest = {destRect.x, destRect.y, destRect.w, destRect.h};
    SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(texture, tint.a);
    SDL_RenderCopyExF(m_renderer, texture, nullptr, &dest, angle * 180.0f / 3.14159f, nullptr, SDL_FLIP_NONE);
}

void Renderer::drawTexture(SDL_Texture* texture, const Rect& srcRect, const Rect& destRect,
                           float angle, const Color& tint) {
    if (!texture) return;

    SDL_Rect src = {static_cast<int>(srcRect.x), static_cast<int>(srcRect.y),
                    static_cast<int>(srcRect.w), static_cast<int>(srcRect.h)};
    SDL_FRect dest = {destRect.x, destRect.y, destRect.w, destRect.h};
    
    SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(texture, tint.a);
    SDL_RenderCopyExF(m_renderer, texture, &src, &dest, angle * 180.0f / 3.14159f, nullptr, SDL_FLIP_NONE);
}

Vec2 Renderer::worldToScreen(const Vec2& worldPos) const {
    if (m_camera) {
        return m_camera->worldToScreen(worldPos);
    }
    return worldPos;
}

Vec2 Renderer::screenToWorld(const Vec2& screenPos) const {
    if (m_camera) {
        return m_camera->screenToWorld(screenPos);
    }
    return screenPos;
}

void Renderer::setBlendMode(SDL_BlendMode mode) {
    SDL_SetRenderDrawBlendMode(m_renderer, mode);
}

void Renderer::drawStarfield(float time) {
    if (!m_starsInitialized) {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> distX(0, static_cast<float>(m_width));
        std::uniform_real_distribution<float> distY(0, static_cast<float>(m_height));
        
        m_stars.resize(100);
        for (auto& star : m_stars) {
            star = Vec2(distX(rng), distY(rng));
        }
        m_starsInitialized = true;
    }

    for (size_t i = 0; i < m_stars.size(); ++i) {
        float twinkle = 0.5f + 0.5f * std::sin(time * 3.0f + i * 0.5f);
        uint8_t brightness = static_cast<uint8_t>(100 + 155 * twinkle);
        
        SDL_SetRenderDrawColor(m_renderer, brightness, brightness, brightness, 255);
        SDL_RenderDrawPointF(m_renderer, m_stars[i].x, m_stars[i].y);
    }
}

void Renderer::drawGrid(float cellSize, const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);

    for (float x = 0; x < m_width; x += cellSize) {
        SDL_RenderDrawLineF(m_renderer, x, 0, x, static_cast<float>(m_height));
    }
    for (float y = 0; y < m_height; y += cellSize) {
        SDL_RenderDrawLineF(m_renderer, 0, y, static_cast<float>(m_width), y);
    }
}

} // namespace GravityPaint
