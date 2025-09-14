/**
 * @file    Main.cpp
 * @brief   This example creates a simple window.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include <iostream>

#include "Window.h"

int main()
{
    constexpr uint32_t windowWidth = 800;
    constexpr uint32_t windowHeight = 600;
    constexpr auto windowTitle = EXAMPLE_APPLICATION_NAME;

    const auto window = std::make_shared<common::window_wrapper::Window>(windowTitle);
    window->Init(windowWidth, windowHeight, false, 1);

    while (!window->CheckWindowCloseFlag()) {
        window->OnUpdate();
    }

    return 0;
}
