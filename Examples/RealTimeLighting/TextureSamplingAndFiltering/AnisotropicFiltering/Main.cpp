/**
 * @file    Main.cpp
 * @brief   In this example, the diffuse texture coordinates of the objects drawn on the screen are scaled, and
 *          anisotropic filtering is activated. Additionally, the maximum anisotropy level supported by the device is
 *          used.
 * @author  Mustafa Yemural (myemural)
 * @date    28.12.2025
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
using namespace examples::real_time_lighting::texture_sampling_and_filtering::anisotropic_filtering;

inline ParameterSchema CreateParameterSchema()
{
    ParameterSchema schema;
    SetCommonParamSchema(schema);

    // Register Constants

    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderFile, "blinn_phong_directional.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::SceneObjectsFragmentShaderFile,
                                               "blinn_phong_directional.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderKey, "vertMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::SceneObjectsFragmentShaderKey, "fragScene");

    schema.RegisterImmutableParam<std::string>(AppConstants::LightUniformBuffer, "lightUniformBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImage, "depthImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::DepthImageView, "depthImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainSampler, "mainSampler");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainDescSet, "mainDescSet");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainDescSetLayout, "mainDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::FloorTexturePath,
                                               "Textures/Linoleum_Floor_001_basecolor.png");
    schema.RegisterImmutableParam<std::string>(AppConstants::FloorImage, "floorImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::FloorImageView, "floorImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::FloorTexture, "floorTexture");

    schema.RegisterImmutableParam<std::string>(AppConstants::CameraObject, "camera");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeObject, "cube");
    schema.RegisterImmutableParam<std::string>(AppConstants::PlaneObject, "plane");

    // Register Customizable Settings
    schema.RegisterParam<VkClearColorValue>(AppSettings::ClearColor);
    schema.RegisterParam<float>(AppSettings::MouseSensitivity);
    schema.RegisterParam<float>(AppSettings::CameraSpeed);
    schema.RegisterParam<glm::vec3>(AppSettings::LightDirection);
    schema.RegisterParam<glm::vec3>(AppSettings::LightColor);
    schema.RegisterParam<float>(AppSettings::AmbientStrength);
    schema.RegisterParam<float>(AppSettings::SpecularStrength);
    schema.RegisterParam<float>(AppSettings::Shininess);
    schema.RegisterParam<float>(AppSettings::MinMipmapLevel);

    return schema;
}

bool SetParams(ParameterServer& params)
{
    try {
        // Initial window settings
        params.Set<std::uint32_t>(WindowParams::Width, 1280);
        params.Set<std::uint32_t>(WindowParams::Height, 720);
        params.Set(WindowParams::Title, std::string(EXAMPLE_APPLICATION_NAME));

        // Vulkan settings
        params.Set<std::string>(VulkanParams::ApplicationName, params.Get<std::string>(WindowParams::Title));
        params.Set<std::vector<std::string>>(VulkanParams::InstanceLayers, {"VK_LAYER_KHRONOS_validation"});

        // Project customizable settings
        params.Set(AppSettings::ClearColor, VkClearColorValue{0.175f, 0.175f, 0.175f, 1.0f});
        params.Set(AppSettings::MouseSensitivity, 3.0f);
        params.Set(AppSettings::CameraSpeed, 3.0f);
        params.Set(AppSettings::LightDirection, glm::vec3(-0.2f, -1.0f, -0.3f));
        params.Set(AppSettings::LightColor, glm::vec3(1.0f, 1.0f, 1.0f));
        params.Set(AppSettings::AmbientStrength, 0.05f);
        params.Set(AppSettings::SpecularStrength, 0.5f);
        params.Set(AppSettings::Shininess, 64.0f);
        params.Set(AppSettings::MinMipmapLevel, 0.0f);
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
