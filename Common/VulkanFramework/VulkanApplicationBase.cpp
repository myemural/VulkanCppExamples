/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplicationBase.h"

#include <utility>

#include "AppCommonConfig.h"

namespace common::vulkan_framework
{
VulkanApplicationBase::VulkanApplicationBase(utility::ParameterServer params)
    : params_{std::move(params)}
{
}

bool VulkanApplicationBase::Run()
{
    if (!CreateInstance()) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return false;
    }

    if (!Init()) {
        std::cerr << "Failed to initialize Vulkan application!" << std::endl;
        return false;
    }

    while (!ShouldClose()) {
        DrawFrame();
        Update();
    }

    Cleanup();

    return true;
}

bool VulkanApplicationBase::CreateInstance()
{
    instance_ = vulkan_wrapper::VulkanInstanceBuilder()
            .SetApplicationInfo([=](auto &info) {
                info.pApplicationName = params_.Get<std::string>(VulkanParams::ApplicationName).c_str();
                info.pEngineName = params_.Get<std::string>(VulkanParams::EngineName).c_str();
                info.engineVersion = params_.Get<std::uint32_t>(VulkanParams::EngineVersion);
                info.apiVersion = params_.Get<std::uint32_t>(VulkanParams::VulkanApiVersion);
            })
            .AddLayers(params_.Get<std::vector<std::string> >(VulkanParams::InstanceLayers))
            .AddExtensions(params_.Get<std::vector<std::string> >(VulkanParams::InstanceExtensions))
            .Build();

    if (!instance_) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return false;
    }

    return true;
}
} // common::vulkan_framework
