/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanSurface.h"

#include "VulkanInstance.h"

namespace common::vulkan_wrapper
{

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR const surface)
    : VulkanObject(std::move(instance), surface)
{
}

VulkanSurface::~VulkanSurface()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto instance = GetParent()) {
            vkDestroySurfaceKHR(instance->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // common::vulkan_wrapper