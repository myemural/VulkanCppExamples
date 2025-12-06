/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanPipelineCache.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanPipelineCache::VulkanPipelineCache(std::shared_ptr<VulkanDevice> device, VkPipelineCache pipelineCache)
    : VulkanObject(std::move(device), pipelineCache)
{
}

std::vector<std::uint8_t> VulkanPipelineCache::GetPipelineCacheData() const
{
    std::shared_ptr<VulkanDevice> device;
    if (device = GetParent(); !device) {
        throw std::runtime_error("Getting parent device is failed!");
    }

    size_t cacheSize = 0;
    if (vkGetPipelineCacheData(device->GetHandle(), handle_, &cacheSize, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Getting size of the pipeline cache is failed!");
    }

    std::vector<std::uint8_t> cacheData(cacheSize);
    if (vkGetPipelineCacheData(device->GetHandle(), handle_, &cacheSize, cacheData.data()) != VK_SUCCESS) {
        throw std::runtime_error("Getting pipeline cache data is failed!");
    }

    return cacheData;
}

VulkanPipelineCache::~VulkanPipelineCache()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyPipelineCache(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
