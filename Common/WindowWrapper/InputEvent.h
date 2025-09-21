/**
 * @file    InputEvent.h
 * @brief   This file keeps input event and its callback function types.
 * @author  Mustafa Yemural (myemural)
 * @date    21.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <functional>

namespace common::window_wrapper
{

enum class EventType {
    Key,
    MouseButton,
    MouseMove,
    MouseScroll
};

class InputEvent {
public:
    virtual ~InputEvent() = default;
    [[nodiscard]] virtual EventType GetType() const = 0;
};

// Key Event
class KeyEvent final : public InputEvent {
public:
    KeyEvent(const int key, const int action) : Key(key), Action(action) {}
    [[nodiscard]] EventType GetType() const override { return EventType::Key; }

    int Key;
    int Action;
};

// Mouse Button Event
class MouseButtonEvent final : public InputEvent {
public:
    MouseButtonEvent(const int button, const int action) : Button(button), Action(action) {}
    [[nodiscard]] EventType GetType() const override { return EventType::MouseButton; }

    int Button;
    int Action;
};

// Mouse Move Event
class MouseMoveEvent final : public InputEvent {
public:
    MouseMoveEvent(const double x, const double y) : X(x), Y(y) {}
    [[nodiscard]] EventType GetType() const override { return EventType::MouseMove; }

    double X;
    double Y;
};

// Mouse Scroll Event
class MouseScrollEvent : public InputEvent {
public:
    MouseScrollEvent(const double deltaX, const double deltaY) : DeltaX(deltaX), DeltaY(deltaY) {}
    [[nodiscard]] EventType GetType() const override { return EventType::MouseScroll; }

    double DeltaX;
    double DeltaY;
};

using KeyCallback = std::function<void(const KeyEvent&)>;
using MouseButtonCallback = std::function<void(const MouseButtonEvent&)>;
using MouseMoveCallback = std::function<void(const MouseMoveEvent&)>;
using MouseScrollCallback = std::function<void(const MouseScrollEvent&)>;

} // namespace common::window_wrapper