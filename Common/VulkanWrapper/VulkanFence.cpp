/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanFence.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanFence::VulkanFence(std::shared_ptr<VulkanDevice> device, VkFence fence)
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

/// TODO: This can be moved to device or think again about waiting multiple fences
void VulkanFence::WaitForFence(const bool waitAll, const uint64_t timeout) const
{
    const auto device = GetParent();
    if (!device || vkWaitForFences(device->GetHandle(), 1, &handle_, waitAll, timeout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to wait for fences!");
    }
}

/// TODO: This can be moved to device or think again about resetting multiple fences
void VulkanFence::ResetFence() const
{
    const auto device = GetParent();
    if (!device || vkResetFences(device->GetHandle(), 1, &handle_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to reset fences!");
    }
}
} // common::vulkan_wrapper
