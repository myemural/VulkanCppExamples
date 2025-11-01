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

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanDeviceMemory;

class VulkanBuffer final : public VulkanObject<VulkanDevice, VkBuffer>
{
public:
    COMMON_API VulkanBuffer(std::shared_ptr<VulkanDevice> device, VkBuffer buffer);

    COMMON_API ~VulkanBuffer() override;

    [[nodiscard]] COMMON_API VkMemoryRequirements GetBufferMemoryRequirements() const;

    COMMON_API void BindBufferMemory(const std::shared_ptr<VulkanDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset) const;
};

class COMMON_API VulkanBufferBuilder
{
public:
    VulkanBufferBuilder();

    VulkanBufferBuilder& SetCreateFlags(const VkBufferCreateFlags& createFlags);

    VulkanBufferBuilder& SetSize(const VkDeviceSize& size);

    VulkanBufferBuilder& SetUsage(const VkBufferUsageFlags& usageFlags);

    VulkanBufferBuilder& SetSharingMode(const VkSharingMode& sharingMode);

    VulkanBufferBuilder& SetQueueFamilyIndices(const std::vector<std::uint32_t>& queueFamilyIndices);

    [[nodiscard]] std::shared_ptr<VulkanBuffer> Build(std::shared_ptr<VulkanDevice> device) const;

private:
    VkBufferCreateInfo createInfo_;
};
} // namespace common::vulkan_wrapper
