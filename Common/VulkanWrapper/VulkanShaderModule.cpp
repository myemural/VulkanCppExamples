/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanShaderModule.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanShaderModule::VulkanShaderModule(std::shared_ptr<VulkanDevice> device, VkShaderModule const shaderModule)
    : VulkanObject(std::move(device), shaderModule)
{
}

VulkanShaderModule::~VulkanShaderModule()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyShaderModule(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
