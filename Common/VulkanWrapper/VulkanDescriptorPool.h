/**
 * @file    VulkanDescriptorPool.h
 * @brief   This file contains wrapper class implementation for VkDescriptorPool.
 * @author  Mustafa Yemural (myemural)
 * @date    22.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanDescriptorSet;
class VulkanDescriptorSetLayout;

class VulkanDescriptorPool final : public VulkanObject<VulkanDevice, VkDescriptorPool>,
                                   public std::enable_shared_from_this<VulkanDescriptorPool>
{
public:
    COMMON_API explicit VulkanDescriptorPool(std::shared_ptr<VulkanDevice> device, VkDescriptorPool descriptorPool);

    COMMON_API std::vector<std::shared_ptr<VulkanDescriptorSet>>
    CreateDescriptorSets(const std::vector<std::shared_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayouts);

    COMMON_API void ResetDescriptorPool(const VkDescriptorPoolResetFlags& resetFlags = 0) const;

    COMMON_API ~VulkanDescriptorPool() override;
};
} // namespace common::vulkan_wrapper
