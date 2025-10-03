/**
 * @file    VulkanFramebuffer.h
 * @brief   This file contains wrapper class implementation for VkFramebuffer.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanRenderPass;
class VulkanImageView;

class VulkanFramebuffer final : public VulkanObject<VulkanDevice, VkFramebuffer>
{
public:
    VulkanFramebuffer(std::shared_ptr<VulkanDevice> device, VkFramebuffer framebuffer);

    ~VulkanFramebuffer() override;
};

class VulkanFramebufferBuilder
{
public:
    VulkanFramebufferBuilder();

    VulkanFramebufferBuilder& SetCreateFlags(const VkFramebufferCreateFlags& flags);

    VulkanFramebufferBuilder& SetDimensions(std::uint32_t width, std::uint32_t height, std::uint32_t layers = 1);

    std::shared_ptr<VulkanFramebuffer> Build(std::shared_ptr<VulkanDevice> device,
                                             const std::shared_ptr<VulkanRenderPass>& renderPass,
                                             const std::vector<std::shared_ptr<VulkanImageView>>& attachments);

private:
    VkFramebufferCreateInfo createInfo_;
};
} // namespace common::vulkan_wrapper
