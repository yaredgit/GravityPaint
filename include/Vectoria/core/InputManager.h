#pragma once

#include "Vectoria/Types.h"
#include <SDL.h>
#include <array>
#include <vector>

namespace Vectoria {

constexpr int MAX_TOUCH_POINTS = 10;

class InputManager {
public:
    InputManager();
    ~InputManager() = default;

    void processEvent(const SDL_Event& event);
    void update(float deltaTime);
    void reset();

    // Touch input
    const TouchPoint& getTouchPoint(int index) const;
    int getActiveTouchCount() const;
    bool isTouching() const;
    
    // Swipe detection
    bool isSwipeDetected() const { return m_swipeDetected; }
    Vec2 getSwipeStart() const { return m_swipeStart; }
    Vec2 getSwipeEnd() const { return m_swipeEnd; }
    Vec2 getSwipeDirection() const { return m_swipeDirection; }
    float getSwipeDistance() const { return m_swipeDistance; }
    void clearSwipe();

    // Accelerometer (for tilt control)
    void enableAccelerometer(bool enable);
    Vec2 getAccelerometerData() const { return m_accelerometer; }
    bool isAccelerometerEnabled() const { return m_accelerometerEnabled; }

    // Mouse input (for desktop)
    Vec2 getMousePosition() const { return m_mousePosition; }
    bool isMouseButtonDown(int button) const;
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;

    // Keyboard input (for desktop debugging)
    bool isKeyDown(SDL_Scancode key) const;
    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyReleased(SDL_Scancode key) const;

    // Gesture recognition
    bool isPinching() const { return m_isPinching; }
    float getPinchScale() const { return m_pinchScale; }
    float getRotationAngle() const { return m_rotationAngle; }

    // Callbacks
    void setTouchCallback(InputCallback callback) { m_touchCallback = callback; }
    void setSwipeCallback(std::function<void(Vec2, Vec2)> callback) { m_swipeCallback = callback; }

private:
    void handleTouchDown(const SDL_Event& event);
    void handleTouchUp(const SDL_Event& event);
    void handleTouchMotion(const SDL_Event& event);
    void handleMouseDown(const SDL_Event& event);
    void handleMouseUp(const SDL_Event& event);
    void handleMouseMotion(const SDL_Event& event);
    void detectSwipe(const TouchPoint& touch);
    void detectPinch();

    std::array<TouchPoint, MAX_TOUCH_POINTS> m_touchPoints;
    int m_activeTouchCount = 0;

    // Swipe state
    bool m_swipeDetected = false;
    Vec2 m_swipeStart;
    Vec2 m_swipeEnd;
    Vec2 m_swipeDirection;
    float m_swipeDistance = 0.0f;

    // Mouse state
    Vec2 m_mousePosition;
    std::array<bool, 5> m_mouseButtonsDown = {};
    std::array<bool, 5> m_mouseButtonsPressed = {};
    std::array<bool, 5> m_mouseButtonsReleased = {};
    std::array<bool, 5> m_prevMouseButtons = {};

    // Keyboard state
    const uint8_t* m_keyboardState = nullptr;
    std::vector<uint8_t> m_prevKeyboardState;

    // Accelerometer
    bool m_accelerometerEnabled = false;
    Vec2 m_accelerometer;
    SDL_Sensor* m_accelerometerSensor = nullptr;

    // Gesture state
    bool m_isPinching = false;
    float m_pinchScale = 1.0f;
    float m_rotationAngle = 0.0f;
    float m_initialPinchDistance = 0.0f;

    // Callbacks
    InputCallback m_touchCallback;
    std::function<void(Vec2, Vec2)> m_swipeCallback;
};

} // namespace Vectoria
