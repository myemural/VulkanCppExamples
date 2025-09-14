/**
 * @file    Main.cpp
 * @brief   This examples draws 3 textured quads to the screen. Top quads have different textures and bottom quad has
 *          multiple textures that mixes of these two textures.
 * @author  Mustafa Yemural (myemural)
 * @date    01.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"
#include "Window.h"
#include "AppConfig.h"
#include "ShaderLoader.h"

using namespace common::utility;
using namespace common::window_wrapper;
using namespace common::vulkan_framework;
using namespace examples::fundamentals::images_and_samplers::using_multiple_textures;

inline ParameterSchema GetParameterSchema()
{
    ParameterSchema schema;
    SetCommonParamSchema(schema);

    // Register Constants
    schema.RegisterImmutableParam<std::uint32_t>(AppConstants::MaxFramesInFlight, 2);
    schema.RegisterImmutableParam<ShaderBaseType>(AppConstants::BaseShaderType, ShaderBaseType::GLSL);
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderFile, "multiple_textures.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderFile, "multiple_textures.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderKey, "vertMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderKey, "fragMain");

    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexBuffer, "mainVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainIndexBuffer, "mainIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::BricksStagingBuffer, "bricksStagingBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::WallStagingBuffer, "wallStagingBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainDescSetLayout, "mainDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::BricksTexturePath, "Textures/bricks.jpg");
    schema.RegisterImmutableParam<std::string>(AppConstants::WallTexturePath, "Textures/wall.jpg");

    // Register Customizable Settings
    schema.RegisterParam<VkClearColorValue>(AppSettings::ClearColor);

    return schema;
}

int main()
{
    ParameterServer params{GetParameterSchema()};

    // Initial window settings
    params.Set<std::uint32_t>(WindowParams::Width, 800);
    params.Set<std::uint32_t>(WindowParams::Height, 600);
    params.Set(WindowParams::Title, std::string(EXAMPLE_APPLICATION_NAME));

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
    params.Set<std::string>(VulkanParams::ApplicationName, params.Get<std::string>(WindowParams::Title));
    params.Set<std::vector<std::string> >(VulkanParams::InstanceLayers, {"VK_LAYER_KHRONOS_validation"});
    params.Set<std::vector<std::string> >(VulkanParams::InstanceExtensions, Window::GetVulkanInstanceExtensions());

    // Project customizable settings
    params.Set(AppSettings::ClearColor, VkClearColorValue{0.0f, 0.3f, 0.3f, 1.0f});

    // Init Vulkan application
    VulkanApplication app{std::move(params)};
    app.SetWindow(window);
    app.Run();

    return 0;
}
