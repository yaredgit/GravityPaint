#pragma once

#include "GravityPaint/Types.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <map>

namespace GravityPaint {

class PhysicsObject;
class GravityField;
class DeformableSurface;
class ParticleSystem;
class Camera;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool initialize(SDL_Window* window, int width, int height);
    void shutdown();

    void beginFrame();
    void endFrame();
    void clear(const Color& color = Color::black());

    // Primitive drawing
    void drawPoint(const Vec2& position, const Color& color, float size = 1.0f);
    void drawLine(const Vec2& start, const Vec2& end, const Color& color, float thickness = 1.0f);
    void drawRect(const Rect& rect, const Color& color, bool filled = true);
    void drawCircle(const Vec2& center, float radius, const Color& color, bool filled = true, int segments = 32);
    void drawTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Color& color, bool filled = true);
    void drawPolygon(const std::vector<Vec2>& points, const Color& color, bool filled = true);

    // Advanced drawing
    void drawGradientRect(const Rect& rect, const Color& topLeft, const Color& topRight, 
                          const Color& bottomLeft, const Color& bottomRight);
    void drawBezierCurve(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, 
                         const Color& color, int segments = 20);
    void drawDashedLine(const Vec2& start, const Vec2& end, const Color& color, 
                        float dashLength = 10.0f, float gapLength = 5.0f);

    // Game-specific drawing
    void drawPhysicsObject(const PhysicsObject* object);
    void drawGravityField(const GravityField* field);
    void drawGravityStroke(const GravityStroke& stroke);
    void drawDeformableSurface(const DeformableSurface* surface);
    void drawGoalZone(const Rect& zone);
    void drawTrail(const std::vector<Vec2>& trail, const Color& color);
    void drawEnergyBar(const Vec2& position, float energy, float maxEnergy);

    // Vector visualization
    void drawVector(const Vec2& origin, const Vec2& direction, float magnitude, const Color& color);
    void drawVectorField(const std::vector<GravityField*>& fields, float gridSize = 50.0f);

    // Text rendering (simplified - would need proper font support)
    void drawText(const std::string& text, const Vec2& position, const Color& color, float size = 24.0f);
    void drawTextCentered(const std::string& text, const Vec2& position, const Color& color, float size = 24.0f);

    // Texture rendering
    void drawTexture(SDL_Texture* texture, const Rect& destRect, float angle = 0.0f, 
                     const Color& tint = Color::white());
    void drawTexture(SDL_Texture* texture, const Rect& srcRect, const Rect& destRect, 
                     float angle = 0.0f, const Color& tint = Color::white());

    // Camera
    void setCamera(Camera* camera) { m_camera = camera; }
    Camera* getCamera() const { return m_camera; }
    Vec2 worldToScreen(const Vec2& worldPos) const;
    Vec2 screenToWorld(const Vec2& screenPos) const;

    // Accessors
    SDL_Renderer* getSDLRenderer() const { return m_renderer; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Effects
    void setAlpha(uint8_t alpha) { m_currentAlpha = alpha; }
    void setBlendMode(SDL_BlendMode mode);

    // Background effects
    void drawStarfield(float time);
    void drawGrid(float cellSize, const Color& color);

private:
    void drawFilledCircle(const Vec2& center, float radius, const Color& color, int segments);
    void drawCircleOutline(const Vec2& center, float radius, const Color& color, int segments);
    TTF_Font* getFont(float size);

    SDL_Renderer* m_renderer = nullptr;
    Camera* m_camera = nullptr;
    
    int m_width;
    int m_height;
    uint8_t m_currentAlpha = 255;

    // Starfield cache
    std::vector<Vec2> m_stars;
    bool m_starsInitialized = false;

    // Font cache
    std::map<int, TTF_Font*> m_fonts;
    std::string m_fontPath;
};

} // namespace GravityPaint
