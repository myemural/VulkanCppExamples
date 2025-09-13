/**
 * @file    VulkanInstance.h
 * @brief   This file contains wrapper class implementation for VkInstance.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanInstance final : public VulkanObject<void, VkInstance>
{
public:
    explicit VulkanInstance(VkInstance instance);

    ~VulkanInstance() override;

    [[nodiscard]] PFN_vkVoidFunction GetInstanceProcAddr(const std::string& name) const;
};

class VulkanInstanceBuilder
{
public:
    VulkanInstanceBuilder();

    VulkanInstanceBuilder &SetApplicationInfo(const std::function<void(VkApplicationInfo &)> &appInfoCallback);

    VulkanInstanceBuilder &AddLayer(const std::string &layerName);

    VulkanInstanceBuilder &AddLayers(const std::vector<std::string> &layerNames);

    VulkanInstanceBuilder &AddExtension(const std::string &extensionName);

    VulkanInstanceBuilder &AddExtensions(const std::vector<std::string> &extensionNames);

    std::shared_ptr<VulkanInstance> Build();

private:
    VkInstanceCreateInfo createInfo_;
    VkApplicationInfo appInfo_;
    std::vector<std::string> layers_;
    std::vector<const char *> layersStr_;
    std::vector<std::string> extensions_;
    std::vector<const char *> extensionsStr_;
};
} // common::vulkan_wrapper
