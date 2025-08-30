/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanBuffer.h"

#include "VulkanDevice.h"
#include "VulkanDeviceMemory.h"

namespace common::vulkan_wrapper
{

inline VkBufferCreateInfo GetDefaultBufferCreateInfo()
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = 0;
    createInfo.usage = 0;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    return createInfo;
}

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, VkBuffer const buffer)
    : VulkanObject(std::move(device), buffer)
{
}

VulkanBuffer::~VulkanBuffer()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyBuffer(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VkMemoryRequirements VulkanBuffer::GetBufferMemoryRequirements() const
{
    VkMemoryRequirements memoryReq;
    if (const auto device = GetParent()) {
        vkGetBufferMemoryRequirements(device->GetHandle(), handle_, &memoryReq);
    }
    return memoryReq;
}

void VulkanBuffer::BindBufferMemory(const std::shared_ptr<VulkanDeviceMemory>& deviceMemory, const VkDeviceSize memoryOffset) const
{
    const auto device = GetParent();
    if (!device) {
        throw std::runtime_error("Device is null!");
    }

    if (vkBindBufferMemory(device->GetHandle(), handle_, deviceMemory->GetHandle(), memoryOffset) != VK_SUCCESS) {
        throw std::runtime_error("Failed to bind buffer memory!");
    }
}

VulkanBufferBuilder::VulkanBufferBuilder()
    : createInfo_{GetDefaultBufferCreateInfo()}
{
}

VulkanBufferBuilder & VulkanBufferBuilder::SetCreateFlags(const VkBufferCreateFlags &createFlags)
{
    createInfo_.flags = createFlags;
    return *this;
}

VulkanBufferBuilder & VulkanBufferBuilder::SetSize(const VkDeviceSize &size)
{
    createInfo_.size = size;
    return *this;
}

VulkanBufferBuilder & VulkanBufferBuilder::SetUsage(const VkBufferUsageFlags &usageFlags)
{
    createInfo_.usage = usageFlags;
    return *this;
}

VulkanBufferBuilder & VulkanBufferBuilder::SetSharingMode(const VkSharingMode &sharingMode)
{
    createInfo_.sharingMode = sharingMode;
    return *this;
}

VulkanBufferBuilder & VulkanBufferBuilder::SetQueueFamilyIndices(const std::vector<std::uint32_t> &queueFamilyIndices)
{
    createInfo_.queueFamilyIndexCount = queueFamilyIndices.size();
    createInfo_.pQueueFamilyIndices = queueFamilyIndices.data();
    return *this;
}

std::shared_ptr<VulkanBuffer> VulkanBufferBuilder::Build(std::shared_ptr<VulkanDevice> device) const
{
    VkBuffer buffer = VK_NULL_HANDLE;
    if (vkCreateBuffer(device->GetHandle(), &createInfo_, nullptr, &buffer) != VK_SUCCESS) {
        std::cout << "Failed to create buffer!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanBuffer>(std::move(device), buffer);
}
} // common::vulkan_wrapper