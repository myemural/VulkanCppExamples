/**
 * @file    InputDispatcher.h
 * @brief   This class has an observer-like pattern and is used to dispatch incoming inputs to the relevant listeners
 *          and to keep the state of the keyboard and mouse buttons.
 * @author  Mustafa Yemural (myemural)
 * @date    21.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "InputEvent.h"

namespace common::window_wrapper
{
class InputDispatcher
{
public:
    static constexpr int MAX_KEYBOARD_KEYS = 1024;
    static constexpr int MAX_MOUSE_BUTTONS = 16;

    void AddKeyListener(KeyCallback callback);

    void AddMouseMoveListener(MouseMoveCallback callback);

    void AddMouseButtonListener(MouseButtonCallback callback);

    void AddMouseScrollListener(MouseScrollCallback callback);

    [[nodiscard]] bool IsKeyPressed(int key) const;

    [[nodiscard]] bool IsMouseButtonPressed(int button) const;

    void OnKeyEvent(int key, int action);

    void OnMouseButtonEvent(int button, int action);

    void OnMouseMoveEvent(double x, double y) const;

    void OnMouseScrollEvent(double dx, double dy) const;

private:
    bool keyboardKeys_[MAX_KEYBOARD_KEYS] = {false};
    bool mouseButtons_[MAX_MOUSE_BUTTONS] = {false};

    std::vector<KeyCallback> keyListeners_;
    std::vector<MouseMoveCallback> mouseMoveListeners_;
    std::vector<MouseButtonCallback> mouseButtonListeners_;
    std::vector<MouseScrollCallback> mouseScrollListeners_;
};
} // namespace common::window_wrapper
