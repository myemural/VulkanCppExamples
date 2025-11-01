/**
 * @file    VulkanDescriptorSetLayout.h
 * @brief   This file contains wrapper class implementation for VkDescriptorSetLayout.
 * @author  Mustafa Yemural (myemural)
 * @date    22.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;

class VulkanDescriptorSetLayout final : public VulkanObject<VulkanDevice, VkDescriptorSetLayout>
{
public:
    COMMON_API explicit VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> device, VkDescriptorSetLayout descriptorSetLayout);

    COMMON_API ~VulkanDescriptorSetLayout() override;
};
} // namespace common::vulkan_wrapper
