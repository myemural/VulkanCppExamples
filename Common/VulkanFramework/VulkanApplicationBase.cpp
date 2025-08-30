/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplicationBase.h"

#include <utility>

#include "VulkanInstance.h"

namespace common::vulkan_framework
{
VulkanApplicationBase::VulkanApplicationBase(ApplicationCreateConfig  cfg)
    : applicationConfig_(std::move(cfg))
{}

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
                info.pApplicationName = applicationConfig_.ApplicationName.c_str();
                info.applicationVersion = applicationConfig_.ApplicationVersion;
                info.pEngineName = applicationConfig_.EngineName.c_str();
                info.engineVersion = applicationConfig_.EngineVersion;
                info.apiVersion = applicationConfig_.VulkanApiVersion;
            })
            .AddLayers(applicationConfig_.InstanceLayers)
            .AddExtensions(applicationConfig_.InstanceExtensions)
            .Build();

    if (!instance_) {
        std::cerr << "Failed to create Vulkan instance!" << std::endl;
        return false;
    }

    return true;
}
} // common::vulkan_framework