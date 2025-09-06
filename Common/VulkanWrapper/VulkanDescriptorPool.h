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
    explicit VulkanDescriptorPool(std::shared_ptr<VulkanDevice> device, VkDescriptorPool descriptorPool);

    std::vector<std::shared_ptr<VulkanDescriptorSet> > CreateDescriptorSets(
        const std::vector<std::shared_ptr<VulkanDescriptorSetLayout> > &descriptorSetLayouts);

    void ResetDescriptorPool(const VkDescriptorPoolResetFlags &resetFlags = 0) const;

    ~VulkanDescriptorPool() override;
};
} // common::vulkan_wrapper