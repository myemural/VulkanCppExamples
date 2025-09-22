/**
 * @file    BufferResource.h
 * @brief   This file contains the implementation of the BufferResource class, which performs operations related to
 *          creating a Vulkan buffer, allocating it, and loading data into the buffer.
 * @author  Mustafa Yemural (myemural)
 * @date    23.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>

#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDeviceMemory.h"

namespace common::vulkan_framework
{

struct BufferResourceCreateInfo
{
    std::string Name;
    std::uint32_t BufferSizeInBytes;
    VkBufferUsageFlags UsageFlags;
    VkMemoryPropertyFlags MemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
};

class BufferResource
{
public:
    /**
     * @param physicalDevice Refers VulkanPhysicalDevice object.
     * @param device Refers VulkanDevice object.
     */
    BufferResource(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice> &physicalDevice,
                   const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device);

    /**
     * @brief Creates a Vulkan buffer with specified type and usage.
     * @param createInfo Buffer create information.
     */
    void CreateBuffer(const BufferResourceCreateInfo &createInfo);

    /// TODO: Creating buffer with concurrency support will be added later.

    /**
     * @brief Maps memory region to read/write it easily on host side.
     * @param mapSize Mapping memory size.
     * @param mapOffset Mapping memory offset.
     */
    void MapMemory(VkDeviceSize mapSize = VK_WHOLE_SIZE, VkDeviceSize mapOffset = 0);

    /**
     * @brief Flushes data to the mapped memory area.
     * @param data The data that will be flushed to the mapped area.
     * @param dataSize Flushing data size.
     * @param mappedMemoryRanges (size, offset) pair of the mapped memory ranges.
     */
    void FlushData(const void *data, std::uint64_t dataSize,
                   const std::vector<std::pair<VkDeviceSize, VkDeviceSize> > &mappedMemoryRanges = {{VK_WHOLE_SIZE, 0}})
    const;

    /**
     * @brief Unmaps memory region.
     */
    void UnmapMemory() const;

    /**
     * @return Returns VulkanBuffer object that held from this class.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetBuffer() const { return buffer_; }

private:
    /**
     * @brief Allocates appropriate memory for the buffer.
     */
    void AllocateBufferMemory();

    std::weak_ptr<vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    BufferResourceCreateInfo createInfo_;
    std::shared_ptr<vulkan_wrapper::VulkanBuffer> buffer_ = nullptr;
    std::shared_ptr<vulkan_wrapper::VulkanDeviceMemory> deviceMemory_ = nullptr;
    void *mappedData_ = nullptr;
};
} // common::vulkan_framework
