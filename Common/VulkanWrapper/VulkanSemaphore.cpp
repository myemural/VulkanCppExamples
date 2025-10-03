/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanSemaphore.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanSemaphore::VulkanSemaphore(std::shared_ptr<VulkanDevice> device, VkSemaphore const semaphore)
    : VulkanObject(std::move(device), semaphore)
{
}

VulkanSemaphore::~VulkanSemaphore()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDeviceWaitIdle(device->GetHandle());
            vkDestroySemaphore(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
