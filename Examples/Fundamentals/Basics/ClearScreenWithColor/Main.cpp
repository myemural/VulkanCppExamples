/**
 * @file    Main.cpp
 * @brief   This example clears screen with a single color to demonstrate basic Vulkan program workflow.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "AppConfig.h"
#include "ParameterServer.h"
#include "VulkanApplication.h"
#include "Window.h"

using namespace common::utility;
using namespace common::window_wrapper;
using namespace examples::fundamentals::basics::clear_screen_with_color;

int main()
{
    common::utility::ParameterServer params;

    // Initial window settings
    params.Set<std::uint32_t>(WindowParams::Width, 800, true);
    params.Set<std::uint32_t>(WindowParams::Height, 600, true);
    params.Set(WindowParams::Title, std::string(EXAMPLE_APPLICATION_NAME), true);
    params.Set(WindowParams::Resizable, false, true);
    params.Set(WindowParams::SampleCount, 1U, true);

    // Create a window
    const auto window = std::make_shared<Window>(params.Get<std::string>(WindowParams::Title));
    if (!window->Init(params.Get<std::uint32_t>(WindowParams::Width),
                      params.Get<std::uint32_t>(WindowParams::Height),
                      params.Get<bool>(WindowParams::Resizable),
                      params.Get<unsigned int>(WindowParams::SampleCount))) {
        std::cerr << "Failed to initialize window." << std::endl;
        return -1;
    }

    // Vulkan settings
    common::vulkan_framework::ApplicationCreateConfig createConfig;
    createConfig.ApplicationName = params.Get<std::string>(WindowParams::Title);
    createConfig.InstanceLayers = {"VK_LAYER_KHRONOS_validation"};
    createConfig.InstanceExtensions = Window::GetVulkanInstanceExtensions();
    params.Set(VulkanParams::AppCreateConfig, createConfig, true);

    // Project constants
    params.Set<std::uint32_t>(ProjectParams::MaxFramesInFlight, 2, true);

    // Project customizable settings
    params.Set(ProjectParams::ClearColor, VkClearColorValue{0.0f, 0.6f, 0.2f, 1.0f});

    // Init Vulkan application
    VulkanApplication app{std::move(params)};
    app.SetWindow(window);
    app.Run();

    return 0;
}
