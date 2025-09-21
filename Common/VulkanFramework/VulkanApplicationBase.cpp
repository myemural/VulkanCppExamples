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

    try {
        while (!ShouldClose()) {
            PreUpdate();
            DrawFrame();
            PostUpdate();
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        Cleanup();
        return false;
    }

    Cleanup();

    return true;
}

std::string VulkanApplicationBase::GetParamStr(const std::string &key) const
{
    return params_.Get<std::string>(key);
}

std::uint32_t VulkanApplicationBase::GetParamU32(const std::string &key) const
{
    return params_.Get<std::uint32_t>(key);
}

float VulkanApplicationBase::GetParamFloat(const std::string &key) const
{
    return params_.Get<float>(key);
}

bool VulkanApplicationBase::CreateInstance()
{
    instance_ = vulkan_wrapper::VulkanInstanceBuilder()
            .SetApplicationInfo([=](auto &info) {
                info.pApplicationName = GetParamStr(VulkanParams::ApplicationName).c_str();
                info.pEngineName = GetParamStr(VulkanParams::EngineName).c_str();
                info.engineVersion = GetParamU32(VulkanParams::EngineVersion);
                info.apiVersion = GetParamU32(VulkanParams::VulkanApiVersion);
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
