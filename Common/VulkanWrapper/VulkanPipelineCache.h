/**
 * @file    VulkanPipelineCache.h
 * @brief   This file contains wrapper class implementation for VkPipelineCache.
 * @author  Mustafa Yemural (myemural)
 * @date    29.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;

class VulkanPipelineCache final : public VulkanObject<VulkanDevice, VkPipelineCache>
{
public:
    COMMON_API explicit VulkanPipelineCache(std::shared_ptr<VulkanDevice> device, VkPipelineCache pipelineCache);

    [[nodiscard]] COMMON_API std::vector<std::uint8_t> GetPipelineCacheData() const;

    COMMON_API ~VulkanPipelineCache() override;
};
} // namespace common::vulkan_wrapper
