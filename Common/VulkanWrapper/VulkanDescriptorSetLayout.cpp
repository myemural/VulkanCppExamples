/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanDescriptorSetLayout.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::shared_ptr<VulkanDevice> device,
                                                     VkDescriptorSetLayout descriptorSetLayout)
    : VulkanObject(std::move(device), descriptorSetLayout)
{
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyDescriptorSetLayout(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
