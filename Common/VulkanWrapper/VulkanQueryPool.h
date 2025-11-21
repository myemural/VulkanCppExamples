/**
 * @file    VulkanQueryPool.h
 * @brief   This file contains wrapper class implementation for VkQueryPool.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;

class VulkanQueryPool final : public VulkanObject<VulkanDevice, VkQueryPool>
{
public:
    COMMON_API VulkanQueryPool(std::shared_ptr<VulkanDevice> device, VkQueryPool queryPool);

    COMMON_API void GetQueryPoolResults(std::uint32_t firstQuery,
                                        std::uint32_t queryCount,
                                        size_t dataSize,
                                        void* data,
                                        VkDeviceSize stride = 0,
                                        const VkQueryResultFlags& flags = 0) const;

    COMMON_API ~VulkanQueryPool() override;
};
} // namespace common::vulkan_wrapper
