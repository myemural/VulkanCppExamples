/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "BufferResource.h"

namespace common::vulkan_framework
{

using namespace common::vulkan_wrapper;

BufferResource::BufferResource(const std::shared_ptr<VulkanPhysicalDevice> &physicalDevice,
                               const std::shared_ptr<VulkanDevice> &device)
    : physicalDevice_{physicalDevice}, device_{device}
{
}

void BufferResource::CreateBuffer(uint32_t size, const VkBufferUsageFlags &usageFlags)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    buffer_ = devicePtr->CreateBuffer([&](auto &builder) {
        builder.SetSize(size);
        builder.SetUsage(usageFlags);
    });

    if (!buffer_) {
        throw std::runtime_error("Failed to create buffer!");
    }
}

void BufferResource::AllocateBufferMemory(const VkMemoryPropertyFlags &properties)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    const auto physicalDevicePtr = physicalDevice_.lock();
    if (!physicalDevicePtr) {
        throw std::runtime_error("Physical device object not found!");
    }

    const auto memoryReq = buffer_->GetBufferMemoryRequirements();

    const uint32_t memoryTypeIndex = physicalDevicePtr->FindMemoryType(memoryReq.memoryTypeBits, properties);

    deviceMemory_ = devicePtr->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!deviceMemory_) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    buffer_->BindBufferMemory(deviceMemory_, 0);
}

void BufferResource::MapMemory(const VkDeviceSize mapSize, const VkDeviceSize mapOffset)
{
    mappedData_ = deviceMemory_->MapMemory(mapSize, mapOffset);
}

void BufferResource::FlushData(const void *data, const std::uint64_t dataSize,
                               const std::vector<std::pair<VkDeviceSize, VkDeviceSize> > &mappedMemoryRanges) const
{
    std::memcpy(mappedData_, data, dataSize);

    deviceMemory_->FlushMappedMemoryRanges(mappedMemoryRanges);
}

void BufferResource::UnmapMemory() const
{
    deviceMemory_->UnmapMemory();
}
} // common::vulkan_framework
