/**
 * @file    VulkanDeviceMemory.h
 * @brief   This file contains wrapper class implementation for VkDeviceMemory.
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

class VulkanDeviceMemory final : public VulkanObject<VulkanDevice, VkDeviceMemory>
{
public:
    VulkanDeviceMemory(std::shared_ptr<VulkanDevice> device, VkDeviceMemory deviceMemory);

    ~VulkanDeviceMemory() override;

    [[nodiscard]] void* MapMemory(VkDeviceSize size, VkDeviceSize offset, const VkMemoryMapFlags& flags = 0) const;

    // Pair: size, offset
    void FlushMappedMemoryRanges(const std::vector<std::pair<VkDeviceSize, VkDeviceSize>>& mappedMemoryRanges) const;

    void UnmapMemory() const;
};
} // common::vulkan_wrapper