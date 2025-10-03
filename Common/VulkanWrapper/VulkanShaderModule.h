/**
 * @file    VulkanShaderModule.h
 * @brief   This file contains wrapper class implementation for VkShaderModule.
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
class VulkanDevice;

class VulkanShaderModule final : public VulkanObject<VulkanDevice, VkShaderModule>
{
public:
    explicit VulkanShaderModule(std::shared_ptr<VulkanDevice> device, VkShaderModule shaderModule);

    ~VulkanShaderModule() override;
};
} // namespace common::vulkan_wrapper
