/**
 * @file    Main.cpp
 * @brief   In this example, a plane with a cloud texture moving into the background is drawn with one render pass.
 *          Then, a second render pass draws the cubes using a perspective view.
 * @author  Mustafa Yemural (myemural)
 * @date    28.09.2025
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
using namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes;

inline ParameterSchema CreateParameterSchema()
{
    ParameterSchema schema;
    SetCommonParamSchema(schema);

    // Register Constants
    schema.RegisterImmutableParam<std::uint32_t>(AppConstants::MaxFramesInFlight, 2);
    schema.RegisterImmutableParam<ShaderBaseType>(AppConstants::BaseShaderType, ShaderBaseType::GLSL);
    schema.RegisterImmutableParam<std::string>(AppConstants::BackgroundVertexShaderFile, "background.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::BackgroundFragmentShaderFile, "background.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeVertexShaderFile, "cube.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeFragmentShaderFile, "cube.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::BackgroundVertexShaderKey, "vertBackground");
    schema.RegisterImmutableParam<std::string>(AppConstants::BackgroundFragmentShaderKey, "fragBackground");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeVertexShaderKey, "vertCube");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeFragmentShaderKey, "fragCube");

    schema.RegisterImmutableParam<std::string>(AppConstants::CubeVertexBuffer, "cubeVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeIndexBuffer, "cubeIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::PlaneVertexBuffer, "planeVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::PlaneIndexBuffer, "planeIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::TimeSpeedUniformBuffer, "timeSpeedUniformBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateImageStagingBuffer, "crateImageStagingBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CloudImageStagingBuffer, "cloudImageStagingBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateImage, "crateImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateImageView, "crateImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::CloudImage, "cloudImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::CloudImageView, "cloudImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImage, "depthImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImageView, "depthImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainSampler, "mainSampler");
    schema.RegisterImmutableParam<std::string>(AppConstants::BackgroundDescSetLayout, "backgroundDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeDescSetLayout, "cubeDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateTexturePath, "Textures/crate1_diffuse.png");
    schema.RegisterImmutableParam<std::string>(AppConstants::CloudTexturePath, "Textures/cloud.png");


    // Register Customizable Settings
    schema.RegisterParam<VkClearColorValue>(AppSettings::ClearColor);
    schema.RegisterParam<float>(AppSettings::MouseSensitivity);
    schema.RegisterParam<float>(AppSettings::CameraSpeed);
    schema.RegisterParam<float>(AppSettings::CloudSpeed);

    return schema;
}

bool SetParams(ParameterServer &params)
{
    try {
        // Initial window settings
        params.Set<std::uint32_t>(WindowParams::Width, 800);
        params.Set<std::uint32_t>(WindowParams::Height, 600);
        params.Set(WindowParams::Title, std::string(EXAMPLE_APPLICATION_NAME));

        // Vulkan settings
        params.Set<std::string>(VulkanParams::ApplicationName, params.Get<std::string>(WindowParams::Title));
        params.Set<std::vector<std::string> >(VulkanParams::InstanceLayers, {"VK_LAYER_KHRONOS_validation"});

        // Project customizable settings
        params.Set(AppSettings::ClearColor, VkClearColorValue{0.0f, 0.3f, 0.3f, 1.0f});
        params.Set(AppSettings::MouseSensitivity, 3.0f);
        params.Set(AppSettings::CameraSpeed, 3.0f);
        params.Set(AppSettings::CloudSpeed, 0.2f);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

int main()
{
    ParameterServer params{CreateParameterSchema()};
    if (!SetParams(params)) {
        std::cerr << "Failed to set parameters!" << std::endl;
        return -1;
    }

    // Create a window
    const auto window = std::make_shared<Window>(params.Get<std::string>(WindowParams::Title));
    if (!window->Init(params.Get<std::uint32_t>(WindowParams::Width),
                      params.Get<std::uint32_t>(WindowParams::Height),
                      params.Get<bool>(WindowParams::Resizable),
                      params.Get<unsigned int>(WindowParams::SampleCount))) {
        std::cerr << "Failed to initialize window." << std::endl;
        return -1;
    }
    params.Set<std::vector<std::string> >(VulkanParams::InstanceExtensions, Window::GetVulkanInstanceExtensions());

    // Init Vulkan application
    VulkanApplication app{std::move(params)};
    app.SetWindow(window);
    app.Run();

    return 0;
}
