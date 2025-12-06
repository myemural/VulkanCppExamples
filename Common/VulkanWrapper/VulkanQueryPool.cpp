/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanQueryPool.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanQueryPool::VulkanQueryPool(std::shared_ptr<VulkanDevice> device, VkQueryPool queryPool)
    : VulkanObject(std::move(device), queryPool)
{
}

void VulkanQueryPool::GetQueryPoolResults(const std::uint32_t firstQuery,
                                          const std::uint32_t queryCount,
                                          const size_t dataSize,
                                          void* data,
                                          const VkDeviceSize stride,
                                          const VkQueryResultFlags& flags) const
{
    if (const auto device = GetParent()) {
        const auto result = vkGetQueryPoolResults(device->GetHandle(), handle_, firstQuery, queryCount, dataSize, data,
                                                  stride, flags);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error in getting query results!");
        }
    }
}

VulkanQueryPool::~VulkanQueryPool()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDeviceWaitIdle(device->GetHandle());
            vkDestroyQueryPool(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
