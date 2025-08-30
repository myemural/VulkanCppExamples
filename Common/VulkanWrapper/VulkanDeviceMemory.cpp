/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanDeviceMemory.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{

VulkanDeviceMemory::VulkanDeviceMemory(std::shared_ptr<VulkanDevice> device, VkDeviceMemory const deviceMemory)
    : VulkanObject(std::move(device), deviceMemory)
{
}

VulkanDeviceMemory::~VulkanDeviceMemory()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkFreeMemory(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

void* VulkanDeviceMemory::MapMemory(const VkDeviceSize size, const VkDeviceSize offset, const VkMemoryMapFlags &flags) const
{
    const auto device = GetParent();
    if (!device) {
        throw std::runtime_error("Device not found!");
    }

    void *data;
    if (vkMapMemory(device->GetHandle(), handle_, offset, size, flags, &data) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map device memory!");
    }

    return data;
}

void VulkanDeviceMemory::FlushMappedMemoryRanges(
    const std::vector<std::pair<VkDeviceSize, VkDeviceSize>> &mappedMemoryRanges) const
{
    std::vector<VkMappedMemoryRange> memoryRanges;
    for (const auto &[size, offset] : mappedMemoryRanges) {
        VkMappedMemoryRange memoryRange;
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.pNext = nullptr;
        memoryRange.memory = handle_;
        memoryRange.offset = offset;
        memoryRange.size = size;
        memoryRanges.push_back(memoryRange);
    }

    const auto device = GetParent();
    if (!device || vkFlushMappedMemoryRanges(device->GetHandle(), memoryRanges.size(), memoryRanges.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to flush mapped memory ranges!");
    }
}

void VulkanDeviceMemory::UnmapMemory() const
{
    const auto device = GetParent();
    if (!device) {
        throw std::runtime_error("Device not found!");
    }

    vkUnmapMemory(device->GetHandle(), handle_);
}
} // common::vulkan_wrapper