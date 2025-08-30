/**
 * @file    VulkanFence.h
 * @brief   This file contains wrapper class implementation for VkFence.
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

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{

class VulkanDevice;

class VulkanFence final : public VulkanObject<VulkanDevice, VkFence>
{
public:
    VulkanFence(std::shared_ptr<VulkanDevice> device, VkFence fence);

    ~VulkanFence() override;

    void WaitForFence(bool waitAll, uint64_t timeout) const;

    void ResetFence() const;
};
} // common::vulkan_wrapper