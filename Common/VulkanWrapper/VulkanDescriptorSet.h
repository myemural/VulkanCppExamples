/**
 * @file    VulkanDescriptorSet.h
 * @brief   This file contains wrapper class implementation for VkDescriptorSet.
 * @author  Mustafa Yemural (myemural)
 * @date    23.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDescriptorPool;

class VulkanDescriptorSet final : public VulkanObject<VulkanDescriptorPool, VkDescriptorSet>
{
public:
    COMMON_API explicit VulkanDescriptorSet(std::shared_ptr<VulkanDescriptorPool> descPool,
                                            VkDescriptorSet descriptorSet);

    [[nodiscard]] COMMON_API VkWriteDescriptorSet
    CreateWriteDescriptorSet(std::uint32_t dstBinding,
                             std::uint32_t dstArrayElement,
                             std::uint32_t descCount,
                             const VkDescriptorType& descType,
                             const std::vector<VkDescriptorBufferInfo>& descBufferInfos = {},
                             const std::vector<VkDescriptorImageInfo>& descImageInfos = {}) const;

    COMMON_API ~VulkanDescriptorSet() override;
};
} // namespace common::vulkan_wrapper
