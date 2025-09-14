/**
 * @file    AppCommonConfig.h
 * @brief   This file keeps common config key names and schema related functions.
 * @author  Mustafa Yemural (myemural)
 * @date    13.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "ParameterServer.h"
#include <vulkan/vulkan_core.h>

namespace common::vulkan_framework
{

// Common Key Names
namespace WindowParams
{
    constexpr auto Width = "Window.Width";
    constexpr auto Height = "Window.Height";
    constexpr auto Title = "Window.Title";
    constexpr auto Resizable = "Window.Resizable";
    constexpr auto SampleCount = "Window.SampleCount";
}

namespace VulkanParams
{
    constexpr auto ApplicationName = "Vulkan.ApplicationName";
    constexpr auto VulkanApiVersion = "Vulkan.VulkanApiVersion";
    constexpr auto ApplicationVersion = "Vulkan.ApplicationVersion";
    constexpr auto EngineName = "Vulkan.EngineName";
    constexpr auto EngineVersion = "Vulkan.EngineVersion";
    constexpr auto InstanceLayers = "Vulkan.InstanceLayers";
    constexpr auto InstanceExtensions = "Vulkan.InstanceExtensions";
    constexpr auto RenderLoopMs = "Vulkan.RenderLoopMs";
}

inline void SetCommonParamSchema(common::utility::ParameterSchema& schema)
{
    schema.RegisterParam<std::uint32_t>(WindowParams::Width, 800);
    schema.RegisterParam<std::uint32_t>(WindowParams::Height, 600);
    schema.RegisterParam<std::string>(WindowParams::Title);
    schema.RegisterParam<bool>(WindowParams::Resizable, false);
    schema.RegisterParam<unsigned int>(WindowParams::SampleCount, 1);

    schema.RegisterParam<std::string>(VulkanParams::ApplicationName);
    schema.RegisterParam<std::uint32_t>(VulkanParams::VulkanApiVersion, VK_API_VERSION_1_0);
    schema.RegisterParam<std::uint32_t>(VulkanParams::ApplicationVersion, VK_MAKE_VERSION(1, 0, 0));
    schema.RegisterParam<std::string>(VulkanParams::EngineName, "DefaultEngine");
    schema.RegisterParam<std::uint32_t>(VulkanParams::EngineVersion, VK_MAKE_VERSION(1, 0, 0));
    schema.RegisterParam<std::vector<std::string>>(VulkanParams::InstanceLayers);
    schema.RegisterParam<std::vector<std::string>>(VulkanParams::InstanceExtensions);
    schema.RegisterParam<long long>(VulkanParams::RenderLoopMs, 8LL);
}

} // namespace common::vulkan_framework