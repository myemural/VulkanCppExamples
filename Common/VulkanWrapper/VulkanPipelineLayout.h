/**
 * @file    VulkanPipelineLayout.h
 * @brief   This file contains wrapper class implementation for VkPipelineLayout.
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

class VulkanPipelineLayout final : public VulkanObject<VulkanDevice, VkPipelineLayout>
{
public:
    explicit VulkanPipelineLayout(std::shared_ptr<VulkanDevice> device, VkPipelineLayout pipelineLayout);

    ~VulkanPipelineLayout() override;
};
} // common::vulkan_wrapper
