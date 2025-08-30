/**
 * @file    VulkanSemaphore.h
 * @brief   This file contains wrapper class implementation for VkSemaphore.
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

class VulkanSemaphore final : public VulkanObject<VulkanDevice, VkSemaphore>
{
public:
    explicit VulkanSemaphore(std::shared_ptr<VulkanDevice> device, VkSemaphore semaphore);

    ~VulkanSemaphore() override;
};
} // common::vulkan_wrapper