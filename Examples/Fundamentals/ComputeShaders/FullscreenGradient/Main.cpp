/**
 * @file    Main.cpp
 * @brief   In this example, a full-screen quad is drawn on the screen, and the gradient output from the compute shader
 *          is placed on it. This gradient moves over time.
 * @author  Mustafa Yemural (myemural)
 * @date    7.11.2025
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
using namespace examples::fundamentals::compute_shaders::fullscreen_gradient;

inline ParameterSchema CreateParameterSchema()
{
    ParameterSchema schema;
    SetCommonParamSchema(schema);

    // Register Constants

    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderFile, "passthrough_position_uv.vert.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderFile, "texture_sampler.frag.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::GradientComputeShaderFile, "gradient_generator.comp.spv");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexShaderKey, "vertMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainFragmentShaderKey, "fragMain");
    schema.RegisterImmutableParam<std::string>(AppConstants::GradientComputeShaderKey, "compGradient");

    schema.RegisterImmutableParam<std::string>(AppConstants::MainVertexBuffer, "mainVertexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainIndexBuffer, "mainIndexBuffer");
    schema.RegisterImmutableParam<std::string>(AppConstants::GradientStorageImage, "gradientStorageImage");
    schema.RegisterImmutableParam<std::string>(AppConstants::GradientStorageImageView, "gradientStorageImageView");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainSampler, "mainSampler");
    schema.RegisterImmutableParam<std::string>(AppConstants::MainDescSetLayout, "mainDescSetLayout");
    schema.RegisterImmutableParam<std::string>(AppConstants::ComputeDescSetLayout, "computeDescSetLayout");

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
