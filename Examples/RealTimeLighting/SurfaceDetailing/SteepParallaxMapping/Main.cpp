/**
 * @file    Main.cpp
 * @brief   In this example, in addition to normal mapping, steep parallax mapping technique is applied to the objects.
 * @author  Mustafa Yemural (myemural)
 * @date    07.01.2026
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
using namespace examples::real_time_lighting::surface_detailing::steep_parallax_mapping;

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
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesTexturePath,
                                               "Textures/Pebbles_027_BaseColor.jpg");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesImage, "pebblesImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesImageView, "pebblesImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesTexture, "pebblesTexture");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesNormalTexturePath,
                                               "Textures/Pebbles_027_Normal.jpg");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesNormalImage, "pebblesNormalImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesNormalImageView, "pebblesNormalImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesNormalTexture, "pebblesNormalTexture");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesHeightTexturePath,
                                               "Textures/Pebbles_027_Height.png");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesHeightImage, "pebblesHeightImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesHeightImageView, "pebblesHeightImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::PebblesHeightTexture, "pebblesHeightTexture");

    schema.RegisterImmutableParam<std::string>(AppConstants::CameraObject, "camera");
    schema.RegisterImmutableParam<std::string>(AppConstants::CubeObject, "cube");
    schema.RegisterImmutableParam<std::string>(AppConstants::PlaneObjectBottom, "planeBottom");
    schema.RegisterImmutableParam<std::string>(AppConstants::PlaneObjectBack, "planeBack");

    // Register Customizable Settings
    schema.RegisterParam<VkClearColorValue>(AppSettings::ClearColor);
    schema.RegisterParam<float>(AppSettings::MouseSensitivity);
    schema.RegisterParam<float>(AppSettings::CameraSpeed);
    schema.RegisterParam<glm::vec3>(AppSettings::LightDirection);
    schema.RegisterParam<glm::vec3>(AppSettings::LightColor);
    schema.RegisterParam<float>(AppSettings::AmbientStrength);
    schema.RegisterParam<float>(AppSettings::SpecularStrength);
    schema.RegisterParam<float>(AppSettings::Shininess);

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
        params.Set(AppSettings::CameraSpeed, 2.2f);
        params.Set(AppSettings::LightDirection, glm::vec3(-0.1f, -0.4f, -0.7f));
        params.Set(AppSettings::LightColor, glm::vec3(1.0f, 1.0f, 1.0f));
        params.Set(AppSettings::AmbientStrength, 0.05f);
        params.Set(AppSettings::SpecularStrength, 0.7f);
        params.Set(AppSettings::Shininess, 128.0f);
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
