/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "InputDispatcher.h"

#include "GLFW/glfw3.h"

namespace common::window_wrapper
{
void InputDispatcher::AddKeyListener(KeyCallback callback) { keyListeners_.push_back(std::move(callback)); }

void InputDispatcher::AddMouseMoveListener(MouseMoveCallback callback)
{
    mouseMoveListeners_.push_back(std::move(callback));
}

void InputDispatcher::AddMouseButtonListener(MouseButtonCallback callback)
{
    mouseButtonListeners_.push_back(std::move(callback));
}

void InputDispatcher::AddMouseScrollListener(MouseScrollCallback callback)
{
    mouseScrollListeners_.push_back(std::move(callback));
}

bool InputDispatcher::IsKeyPressed(const int key) const
{
    if (key < 0 || key >= MAX_KEYBOARD_KEYS) {
        return false;
    }
    return keyboardKeys_[key];
}

bool InputDispatcher::IsMouseButtonPressed(const int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return false;
    }
    return mouseButtons_[button];
}

void InputDispatcher::OnKeyEvent(const int key, const int action)
{
    if (key < 0 || key >= MAX_KEYBOARD_KEYS) {
        return;
    }

    // Update state array
    keyboardKeys_[key] = action != GLFW_RELEASE;

    // Dispatch event
    const KeyEvent e(key, action);
    for (auto& callback: keyListeners_) {
        callback(e);
    }
}

void InputDispatcher::OnMouseButtonEvent(const int button, const int action)
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return;
    }

    // Update state array
    mouseButtons_[button] = action != GLFW_RELEASE;

    const MouseButtonEvent e(button, action);
    for (auto& callback: mouseButtonListeners_) {
        callback(e);
    }
}

void InputDispatcher::OnMouseMoveEvent(const double x, const double y) const
{
    const MouseMoveEvent e(x, y);
    for (auto& callback: mouseMoveListeners_) {
        callback(e);
    }
}

void InputDispatcher::OnMouseScrollEvent(const double dx, const double dy) const
{
    const MouseScrollEvent e(dx, dy);
    for (auto& callback: mouseScrollListeners_) {
        callback(e);
    }
}
} // namespace common::window_wrapper
