/**
 * @file    Main.cpp
 * @brief   In this example, a checkerboard texture is first generated with a compute shader and then applied to the
 *          spheres in the scene. The user can also specify the size and tile size of the texture to be generated.
 * @author  Mustafa Yemural (myemural)
 * @date    9.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ShaderLoader.h"
#include "VulkanApplication.h"
#include "Window.h"

using namespace common::utility;
using namespace common::window_wrapper;
using namespace common::vulkan_framework;
using namespace examples::fundamentals::compute_shaders::checkerboard_generator;

inline ParameterSchema CreateParameterSchema()
{
    ParameterSchema schema;
    SetCommonParamSchema(schema);

    // Register Constants
    schema.RegisterImmutableParam<std::uint32_t>(AppConstants::MaxFramesInFlight, 2);

    schema.RegisterImmutableParam<ShaderBaseType>(AppConstants::BaseShaderType, ShaderBaseType::GLSL);
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderFile, "drawing_cube.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderFile, "drawing_cube.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::CheckerboardComputeShaderFile,
                                               "checkerboard_generator.comp.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderKey, "vertMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderKey, "fragMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::CheckerboardComputeShaderKey, "compCheckerboard");

    schema.RegisterImmutableParam<std::string>(AppConstants::CubeVertexBuffer, "cubeVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeIndexBuffer, "cubeIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::SphereVertexBuffer, "sphereVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::SphereIndexBuffer, "sphereIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::CheckerboardStorageImage, "checkerboardStorageImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::CheckerboardStorageImageView,
                                               "checkerboardStorageImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateImage, "crateImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateImageView, "crateImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImage, "depthImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImageView, "depthImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainSampler, "mainSampler");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeDescSet, "cubeDescSet");
    schema.RegisterImmutableParam<std::string>(AppConstants::SphereDescSet, "sphereDescSet");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainDescSetLayout, "mainDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::ComputeDescSetLayout, "computeDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::CrateTexturePath, "Textures/crate1_diffuse.png");

    // Register Customizable Settings
    schema.RegisterParam<VkClearColorValue>(AppSettings::ClearColor);
    schema.RegisterParam<float>(AppSettings::MouseSensitivity);
    schema.RegisterParam<float>(AppSettings::CameraSpeed);
    schema.RegisterParam<std::uint32_t>(AppSettings::CheckerboardTextureSize);
    schema.RegisterParam<int>(AppSettings::TileSize);

    return schema;
}

bool SetParams(ParameterServer& params)
{
    try {
        // Initial window settings
        params.Set<std::uint32_t>(WindowParams::Width, 800);
        params.Set<std::uint32_t>(WindowParams::Height, 600);
        params.Set(WindowParams::Title, std::string(EXAMPLE_APPLICATION_NAME));

        // Vulkan settings
        params.Set<std::string>(VulkanParams::ApplicationName, params.Get<std::string>(WindowParams::Title));
        params.Set<std::vector<std::string>>(VulkanParams::InstanceLayers, {"VK_LAYER_KHRONOS_validation"});

        // Project customizable settings
        params.Set(AppSettings::ClearColor, VkClearColorValue{0.0f, 0.3f, 0.3f, 1.0f});
        params.Set(AppSettings::MouseSensitivity, 3.0f);
        params.Set(AppSettings::CameraSpeed, 3.0f);
        params.Set<std::uint32_t>(AppSettings::CheckerboardTextureSize, 1024);
        params.Set(AppSettings::TileSize, 32);
    } catch (const std::exception& e) {
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
    if (!window->Init(params.Get<std::uint32_t>(WindowParams::Width), params.Get<std::uint32_t>(WindowParams::Height),
                      params.Get<bool>(WindowParams::Resizable))) {
        std::cerr << "Failed to initialize window." << std::endl;
        return -1;
    }
    params.Set<std::vector<std::string>>(VulkanParams::InstanceExtensions, Window::GetVulkanInstanceExtensions());

    // Init Vulkan application
    VulkanApplication app{std::move(params)};
    app.SetWindow(window);
    app.Run();

    return 0;
}
