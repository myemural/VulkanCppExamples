/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanDescriptorSet.h"

#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"

namespace common::vulkan_wrapper
{

VulkanDescriptorSet::VulkanDescriptorSet(std::shared_ptr<VulkanDescriptorPool> descPool,
                                         VkDescriptorSet descriptorSet)
    : VulkanObject(std::move(descPool), descriptorSet)
{
}

/// TODO: Add BufferView after implemented it
VkWriteDescriptorSet VulkanDescriptorSet::CreateWriteDescriptorSet(const std::uint32_t dstBinding,
                                                                   const std::uint32_t dstArrayElement,
                                                                   const std::uint32_t descCount,
                                                                   const VkDescriptorType &descType,
                                                                   const std::vector<VkDescriptorBufferInfo> &
                                                                   descBufferInfos,
                                                                   const std::vector<VkDescriptorImageInfo> &
                                                                   descImageInfos) const
{
    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.pNext = nullptr;
    descriptorWrite.dstSet = handle_;
    descriptorWrite.dstBinding = dstBinding;
    descriptorWrite.dstArrayElement = dstArrayElement;
    descriptorWrite.descriptorCount = descCount;
    descriptorWrite.descriptorType = descType;
    descriptorWrite.pImageInfo = descImageInfos.empty() ? nullptr : descImageInfos.data();
    descriptorWrite.pBufferInfo = descBufferInfos.empty() ? nullptr : descBufferInfos.data();
    descriptorWrite.pTexelBufferView = nullptr; /// TODO: Add BufferView after implemented it

    return descriptorWrite;
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto pool = GetParent()) {
            if (const auto device = pool->GetParent()) {
                vkDeviceWaitIdle(device->GetHandle());
                vkFreeDescriptorSets(device->GetHandle(), pool->GetHandle(), 1, &handle_);
                handle_ = VK_NULL_HANDLE;
            }
        }
    }
}
} // common::vulkan_wrapper