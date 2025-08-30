/**
 * @file    VulkanRenderPass.h
 * @brief   This file contains wrapper class implementation for VkRenderPass.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <functional>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{

class VulkanDevice;

class VulkanRenderPass final : public VulkanObject<VulkanDevice, VkRenderPass>
{
public:
    VulkanRenderPass(std::shared_ptr<VulkanDevice> device, VkRenderPass renderPass);

    ~VulkanRenderPass() override;

    [[nodiscard]] VkExtent2D GetRenderAreaGranularity() const;
};

class VulkanRenderPassBuilder
{
public:
    VulkanRenderPassBuilder();

    VulkanRenderPassBuilder& SetCreateFlags(const VkRenderPassCreateFlags& flags);

    VulkanRenderPassBuilder& AddAttachment(const std::function<void(VkAttachmentDescription&)>& setterFunc);

    VulkanRenderPassBuilder& AddSubpass(const std::function<void(VkSubpassDescription&)>& setterFunc);

    VulkanRenderPassBuilder& AddDependency(const std::function<void(VkSubpassDependency&)>& setterFunc);

    std::shared_ptr<VulkanRenderPass> Build(std::shared_ptr<VulkanDevice> device);

private:
    VkRenderPassCreateInfo createInfo{};
    std::vector<VkAttachmentDescription> attachments{};
    std::vector<VkSubpassDescription> subpasses{};
    std::vector<VkSubpassDependency> dependencies{};
};
} // common::vulkan_wrapper