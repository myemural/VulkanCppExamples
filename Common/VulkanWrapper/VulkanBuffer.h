/**
 * @file    VulkanBuffer.h
 * @brief   This file contains wrapper class implementation for VkBuffer.
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

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanDeviceMemory;

class VulkanBuffer final : public VulkanObject<VulkanDevice, VkBuffer>
{
public:
    VulkanBuffer(std::shared_ptr<VulkanDevice> device, VkBuffer buffer);

    ~VulkanBuffer() override;

    [[nodiscard]] VkMemoryRequirements GetBufferMemoryRequirements() const;

    void BindBufferMemory(const std::shared_ptr<VulkanDeviceMemory> &deviceMemory, VkDeviceSize memoryOffset) const;
};

class VulkanBufferBuilder
{
public:
    VulkanBufferBuilder();

    VulkanBufferBuilder &SetCreateFlags(const VkBufferCreateFlags &createFlags);

    VulkanBufferBuilder &SetSize(const VkDeviceSize &size);

    VulkanBufferBuilder &SetUsage(const VkBufferUsageFlags &usageFlags);

    VulkanBufferBuilder &SetSharingMode(const VkSharingMode &sharingMode);

    VulkanBufferBuilder &SetQueueFamilyIndices(const std::vector<std::uint32_t> &queueFamilyIndices);

    [[nodiscard]] std::shared_ptr<VulkanBuffer> Build(std::shared_ptr<VulkanDevice> device) const;

private:
    VkBufferCreateInfo createInfo_;
};
} // common::vulkan_wrapper
