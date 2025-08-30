/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanFence.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{

VulkanFence::VulkanFence(std::shared_ptr<VulkanDevice> device, VkFence const fence)
    : VulkanObject(std::move(device), fence)
{
}

VulkanFence::~VulkanFence()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDeviceWaitIdle(device->GetHandle());
            vkDestroyFence(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

void VulkanFence::WaitForFence(const bool waitAll, const uint64_t timeout) const
{
    if (const auto device = GetParent()) {
        vkWaitForFences(device->GetHandle(), 1, &handle_, waitAll, timeout);
    }
}

void VulkanFence::ResetFence() const
{
    if (const auto device = GetParent()) {
        vkResetFences(device->GetHandle(), 1, &handle_);
    }
}
} // common::vulkan_wrapper