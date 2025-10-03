/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanPipelineLayout.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanPipelineLayout::VulkanPipelineLayout(std::shared_ptr<VulkanDevice> device, VkPipelineLayout pipelineLayout)
    : VulkanObject(std::move(device), pipelineLayout)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyPipelineLayout(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
