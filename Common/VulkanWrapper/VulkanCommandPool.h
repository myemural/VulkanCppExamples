/**
 * @file    VulkanCommandPool.h
 * @brief   This file contains wrapper class implementation for VkCommandPool.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <vector>

#include "VulkanObject.h"

#include <vulkan/vulkan_core.h>

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanCommandBuffer;

class VulkanCommandPool final : public VulkanObject<VulkanDevice, VkCommandPool>,
                                public std::enable_shared_from_this<VulkanCommandPool>
{
public:
    VulkanCommandPool(std::shared_ptr<VulkanDevice> device, VkCommandPool cmdPool);

    ~VulkanCommandPool() override;

    std::vector<std::shared_ptr<VulkanCommandBuffer>> CreateCommandBuffers(std::uint32_t count,
                                                                           const VkCommandBufferLevel& level);

    bool ResetCommandPool(const VkCommandPoolResetFlags& resetFlags = 0) const;
};

class VulkanCommandPoolBuilder
{
public:
    VulkanCommandPoolBuilder();

    VulkanCommandPoolBuilder& SetCreateFlags(const VkCommandPoolCreateFlags& flags);

    VulkanCommandPoolBuilder& SetQueueFamilyIndex(std::uint32_t familyIndex);

    [[nodiscard]] std::shared_ptr<VulkanCommandPool> Build(std::shared_ptr<VulkanDevice> device) const;

private:
    VkCommandPoolCreateInfo createInfo_;
};
} // namespace common::vulkan_wrapper
