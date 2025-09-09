/**
 * @file    Main.cpp
 * @brief   This examples draws a cube to the screen and rotates it constantly. But this time also has a camera that
 *          controlling with WASD keys and mouse. With this camera you can move around the scene as you wish.
 * @author  Mustafa Yemural (myemural)
 * @date    09.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"
#include "VulkanApplicationBase.h"
#include "Window.h"

using namespace common::vulkan_framework;
using namespace examples::fundamentals::drawing_3d::basic_camera_control;

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

    ApplicationCreateConfig config;
    config.ApplicationName = windowTitle;
    config.InstanceLayers = {"VK_LAYER_KHRONOS_validation"};
    config.InstanceExtensions = common::window_wrapper::Window::GetVulkanInstanceExtensions();

    // Customize the example settings
    ApplicationSettings settings;
    settings.ClearColor = {0.0f, 0.3f, 0.3f, 1.0f};
    settings.MouseSensitivity = 3.0f;
    settings.CameraSpeed = 3.0f;

    // Init Vulkan application
    VulkanApplication app{config, settings};
    app.SetWindow(window);
    app.Run();

    return 0;
}