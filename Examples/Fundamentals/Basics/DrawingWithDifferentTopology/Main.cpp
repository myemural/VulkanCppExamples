/**
 * @file    Main.cpp
 * @brief   This example draws two quads to the screen to demonstrate different topology usage and
 *          "primitiveRestartIndex" usage. This example also contains selecting different polygon mode
 *          and line width (if polygon mode is line) for rasterization states in pipeline.
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include <thread>

#include "VulkanApplication.h"
#include "Window.h"

using namespace examples::fundamentals::basics::drawing_with_different_topology;

int main()
{
    // Window settings
    constexpr uint32_t windowWidth = 800;
    constexpr uint32_t windowHeight = 600;
    constexpr auto windowTitle = EXAMPLE_APPLICATION_NAME;

    // Create a window
    const auto window = std::make_shared<common::window_wrapper::Window>(windowTitle);
    if (!window->Init(windowWidth, windowHeight, false, 1)) {
        std::cerr << "Failed to initialize window." << std::endl;
        return -1;
    }

    common::vulkan_framework::ApplicationCreateConfig config;
    config.ApplicationName = windowTitle;
    config.InstanceLayers = {"VK_LAYER_KHRONOS_validation"};
    config.InstanceExtensions = common::window_wrapper::Window::GetVulkanInstanceExtensions();

    // Customize the example settings
    ApplicationSettings settings;
    settings.ClearColor = {0.0f, 0.6f, 0.2f, 1.0f};
    settings.PrimitiveRestartEnabled = true;
    settings.PolygonMode = VK_POLYGON_MODE_FILL;
    settings.LineWidth = 1.0f;

    // Init Vulkan application
    VulkanApplication app{config, settings};
    app.SetWindow(window);
    app.Run();

    return 0;
}