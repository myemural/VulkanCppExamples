/**
 * @file    VulkanSurface.h
 * @brief   This file contains wrapper class implementation for VkSurfaceKHR.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanInstance;

class VulkanSurface final : public VulkanObject<VulkanInstance, VkSurfaceKHR>
{
public:
    VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR surface);

    ~VulkanSurface() override;
};
} // namespace common::vulkan_wrapper
