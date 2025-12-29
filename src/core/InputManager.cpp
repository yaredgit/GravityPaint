#include "Vectoria/core/InputManager.h"
#include "Vectoria/Constants.h"
#include <cstring>

namespace Vectoria {

InputManager::InputManager() {
    m_keyboardState = SDL_GetKeyboardState(nullptr);
    m_prevKeyboardState.resize(SDL_NUM_SCANCODES, 0);
    
    for (auto& touch : m_touchPoints) {
        touch = TouchPoint();
    }
}

void InputManager::processEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_FINGERDOWN:
            handleTouchDown(event);
            break;
        case SDL_FINGERUP:
            handleTouchUp(event);
            break;
        case SDL_FINGERMOTION:
            handleTouchMotion(event);
            break;
        case SDL_MOUSEBUTTONDOWN:
            handleMouseDown(event);
            break;
        case SDL_MOUSEBUTTONUP:
            handleMouseUp(event);
            break;
        case SDL_MOUSEMOTION:
            handleMouseMotion(event);
            break;
        case SDL_SENSORUPDATE:
            if (m_accelerometerEnabled && event.sensor.type == SDL_SENSOR_ACCEL) {
                m_accelerometer.x = event.sensor.data[0];
                m_accelerometer.y = event.sensor.data[1];
            }
            break;
        default:
            break;
    }
}

void InputManager::update(float /*deltaTime*/) {
    // Update previous mouse button states
    m_prevMouseButtons = m_mouseButtonsDown;
    
    // Calculate pressed/released states
    for (int i = 0; i < 5; ++i) {
        m_mouseButtonsPressed[i] = m_mouseButtonsDown[i] && !m_prevMouseButtons[i];
        m_mouseButtonsReleased[i] = !m_mouseButtonsDown[i] && m_prevMouseButtons[i];
    }

    // Update previous keyboard state
    std::memcpy(m_prevKeyboardState.data(), m_keyboardState, SDL_NUM_SCANCODES);

    // Detect pinch gesture if multiple touches
    if (m_activeTouchCount >= 2) {
        detectPinch();
    } else {
        m_isPinching = false;
        m_pinchScale = 1.0f;
    }

    // Reset swipe detection for next frame
    m_swipeDetected = false;
}

void InputManager::reset() {
    for (auto& touch : m_touchPoints) {
        touch = TouchPoint();
    }
    m_activeTouchCount = 0;
    m_swipeDetected = false;
    m_isPinching = false;
    
    for (int i = 0; i < 5; ++i) {
        m_mouseButtonsDown[i] = false;
        m_mouseButtonsPressed[i] = false;
        m_mouseButtonsReleased[i] = false;
    }
}

const TouchPoint& InputManager::getTouchPoint(int index) const {
    if (index >= 0 && index < MAX_TOUCH_POINTS) {
        return m_touchPoints[index];
    }
    static TouchPoint empty;
    return empty;
}

int InputManager::getActiveTouchCount() const {
    return m_activeTouchCount;
}

bool InputManager::isTouching() const {
    return m_activeTouchCount > 0;
}

void InputManager::clearSwipe() {
    m_swipeDetected = false;
    m_swipeStart = Vec2();
    m_swipeEnd = Vec2();
    m_swipeDirection = Vec2();
    m_swipeDistance = 0.0f;
}

void InputManager::enableAccelerometer(bool enable) {
    m_accelerometerEnabled = enable;
    
    if (enable && !m_accelerometerSensor) {
        int numSensors = SDL_NumSensors();
        for (int i = 0; i < numSensors; ++i) {
            if (SDL_SensorGetDeviceType(i) == SDL_SENSOR_ACCEL) {
                m_accelerometerSensor = SDL_SensorOpen(i);
                break;
            }
        }
    } else if (!enable && m_accelerometerSensor) {
        SDL_SensorClose(m_accelerometerSensor);
        m_accelerometerSensor = nullptr;
    }
}

bool InputManager::isMouseButtonDown(int button) const {
    if (button >= 1 && button <= 5) {
        return m_mouseButtonsDown[button - 1];
    }
    return false;
}

bool InputManager::isMouseButtonPressed(int button) const {
    if (button >= 1 && button <= 5) {
        return m_mouseButtonsPressed[button - 1];
    }
    return false;
}

bool InputManager::isMouseButtonReleased(int button) const {
    if (button >= 1 && button <= 5) {
        return m_mouseButtonsReleased[button - 1];
    }
    return false;
}

bool InputManager::isKeyDown(SDL_Scancode key) const {
    return m_keyboardState[key] != 0;
}

bool InputManager::isKeyPressed(SDL_Scancode key) const {
    return m_keyboardState[key] != 0 && m_prevKeyboardState[key] == 0;
}

bool InputManager::isKeyReleased(SDL_Scancode key) const {
    return m_keyboardState[key] == 0 && m_prevKeyboardState[key] != 0;
}

void InputManager::handleTouchDown(const SDL_Event& event) {
    SDL_TouchFingerEvent finger = event.tfinger;
    
    // Find empty slot or reuse finger ID slot
    int slot = -1;
    for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
        if (!m_touchPoints[i].isActive) {
            slot = i;
            break;
        }
    }
    
    if (slot >= 0) {
        // Get window size for coordinate conversion
        int windowWidth = DEFAULT_SCREEN_WIDTH;
        int windowHeight = DEFAULT_SCREEN_HEIGHT;
        
        Vec2 pos(finger.x * windowWidth, finger.y * windowHeight);
        
        m_touchPoints[slot].id = static_cast<int>(finger.fingerId);
        m_touchPoints[slot].position = pos;
        m_touchPoints[slot].previousPosition = pos;
        m_touchPoints[slot].startPosition = pos;
        m_touchPoints[slot].pressure = finger.pressure;
        m_touchPoints[slot].isActive = true;
        m_touchPoints[slot].timestamp = static_cast<float>(SDL_GetTicks()) / 1000.0f;
        
        m_activeTouchCount++;

        if (m_touchCallback) {
            m_touchCallback(m_touchPoints[slot]);
        }
    }
}

void InputManager::handleTouchUp(const SDL_Event& event) {
    SDL_TouchFingerEvent finger = event.tfinger;
    
    for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
        if (m_touchPoints[i].isActive && m_touchPoints[i].id == static_cast<int>(finger.fingerId)) {
            // Detect swipe before deactivating
            detectSwipe(m_touchPoints[i]);
            
            m_touchPoints[i].isActive = false;
            m_activeTouchCount = std::max(0, m_activeTouchCount - 1);

            if (m_touchCallback) {
                m_touchCallback(m_touchPoints[i]);
            }
            break;
        }
    }
}

void InputManager::handleTouchMotion(const SDL_Event& event) {
    SDL_TouchFingerEvent finger = event.tfinger;
    
    int windowWidth = DEFAULT_SCREEN_WIDTH;
    int windowHeight = DEFAULT_SCREEN_HEIGHT;
    
    for (int i = 0; i < MAX_TOUCH_POINTS; ++i) {
        if (m_touchPoints[i].isActive && m_touchPoints[i].id == static_cast<int>(finger.fingerId)) {
            m_touchPoints[i].previousPosition = m_touchPoints[i].position;
            m_touchPoints[i].position = Vec2(finger.x * windowWidth, finger.y * windowHeight);
            m_touchPoints[i].pressure = finger.pressure;
            
            if (m_touchCallback) {
                m_touchCallback(m_touchPoints[i]);
            }
            break;
        }
    }
}

void InputManager::handleMouseDown(const SDL_Event& event) {
    int button = event.button.button;
    if (button >= 1 && button <= 5) {
        m_mouseButtonsDown[button - 1] = true;
    }
    
    m_mousePosition = Vec2(static_cast<float>(event.button.x), static_cast<float>(event.button.y));

    // Simulate touch with mouse for desktop
    if (button == SDL_BUTTON_LEFT && m_activeTouchCount == 0) {
        m_touchPoints[0].id = 0;
        m_touchPoints[0].position = m_mousePosition;
        m_touchPoints[0].previousPosition = m_mousePosition;
        m_touchPoints[0].startPosition = m_mousePosition;
        m_touchPoints[0].isActive = true;
        m_touchPoints[0].timestamp = static_cast<float>(SDL_GetTicks()) / 1000.0f;
        m_activeTouchCount = 1;

        if (m_touchCallback) {
            m_touchCallback(m_touchPoints[0]);
        }
    }
}

void InputManager::handleMouseUp(const SDL_Event& event) {
    int button = event.button.button;
    if (button >= 1 && button <= 5) {
        m_mouseButtonsDown[button - 1] = false;
    }

    // End simulated touch
    if (button == SDL_BUTTON_LEFT && m_touchPoints[0].isActive) {
        detectSwipe(m_touchPoints[0]);
        m_touchPoints[0].isActive = false;
        m_activeTouchCount = 0;

        if (m_touchCallback) {
            m_touchCallback(m_touchPoints[0]);
        }
    }
}

void InputManager::handleMouseMotion(const SDL_Event& event) {
    m_mousePosition = Vec2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));

    // Update simulated touch
    if (m_touchPoints[0].isActive) {
        m_touchPoints[0].previousPosition = m_touchPoints[0].position;
        m_touchPoints[0].position = m_mousePosition;

        if (m_touchCallback) {
            m_touchCallback(m_touchPoints[0]);
        }
    }
}

void InputManager::detectSwipe(const TouchPoint& touch) {
    Vec2 delta = touch.position - touch.startPosition;
    float distance = delta.length();

    if (distance >= MIN_SWIPE_DISTANCE) {
        m_swipeDetected = true;
        m_swipeStart = touch.startPosition;
        m_swipeEnd = touch.position;
        m_swipeDirection = delta.normalized();
        m_swipeDistance = distance;

        if (m_swipeCallback) {
            m_swipeCallback(m_swipeStart, m_swipeEnd);
        }
    }
}

void InputManager::detectPinch() {
    if (m_activeTouchCount < 2) return;

    // Find two active touches
    int first = -1, second = -1;
    for (int i = 0; i < MAX_TOUCH_POINTS && second < 0; ++i) {
        if (m_touchPoints[i].isActive) {
            if (first < 0) first = i;
            else second = i;
        }
    }

    if (first >= 0 && second >= 0) {
        float currentDistance = (m_touchPoints[first].position - m_touchPoints[second].position).length();
        float previousDistance = (m_touchPoints[first].previousPosition - m_touchPoints[second].previousPosition).length();

        if (!m_isPinching) {
            m_isPinching = true;
            m_initialPinchDistance = currentDistance;
            m_pinchScale = 1.0f;
        } else if (m_initialPinchDistance > 0.01f) {
            m_pinchScale = currentDistance / m_initialPinchDistance;
        }

        // Calculate rotation
        Vec2 currentDir = (m_touchPoints[second].position - m_touchPoints[first].position).normalized();
        Vec2 previousDir = (m_touchPoints[second].previousPosition - m_touchPoints[first].previousPosition).normalized();
        m_rotationAngle = std::atan2(currentDir.cross(previousDir), currentDir.dot(previousDir));
    }
}

} // namespace Vectoria
