/**
 * @file    Main.cpp
 * @brief   This example draws multicolored triangles to the screen to demonstrate vertex attributes working mechanism.
 * @author  Mustafa Yemural (myemural)
 * @date    20.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include <thread>

#include "VulkanApplication.h"
#include "Window.h"

using namespace examples::fundamentals::basics::drawing_multicolor_triangles;

int main()
{
    // Window settings
    constexpr uint32_t windowWidth = 800;
    constexpr uint32_t windowHeight = 600;
    constexpr auto windowTitle = EXAMPLE_APPLICATION_NAME;

    // Create a window
    const auto window = std::make_shared<Window>(windowTitle);
    if (!window->Init(windowWidth, windowHeight, false, 1)) {
        std::cerr << "Failed to initialize window." << std::endl;
        return -1;
    }

    common::vulkan_framework::ApplicationCreateConfig config;
    config.ApplicationName = windowTitle;
    config.InstanceLayers = {"VK_LAYER_KHRONOS_validation"};
    config.InstanceExtensions = Window::GetVulkanInstanceExtensions();

    // Customize the example settings
    ApplicationSettings settings;
    settings.ClearColor = {0.0f, 0.6f, 0.2f, 1.0f};

    // Init Vulkan application
    VulkanApplication app{config, settings};
    app.SetWindow(window);
    app.Run();

    return 0;
}