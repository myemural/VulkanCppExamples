/**
 * @file    VulkanImageView.h
 * @brief   This file contains wrapper class implementation for VkImageView.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{

class VulkanDevice;
class VulkanImage;

class VulkanImageView final : public VulkanObject<VulkanDevice, VkImageView>
{
public:
    VulkanImageView(std::shared_ptr<VulkanDevice> device, VkImageView imageView);

    ~VulkanImageView() override;
};

class VulkanImageViewBuilder
{
public:
    VulkanImageViewBuilder();

    VulkanImageViewBuilder& SetCreateFlags(const VkImageViewCreateFlags& flags);

    VulkanImageViewBuilder& SetImageViewType(const VkImageViewType& viewType);

    VulkanImageViewBuilder& SetFormat(const VkFormat& format);

    VulkanImageViewBuilder& SetComponents(const VkComponentMapping& components);

    VulkanImageViewBuilder& SetSubresourceRange(const VkImageSubresourceRange& subresourceRange);

    VulkanImageViewBuilder& SetImage(const std::shared_ptr<VulkanImage> &image);

    std::shared_ptr<VulkanImageView> Build(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<VulkanImage> &image);

    std::shared_ptr<VulkanImageView> Build(std::shared_ptr<VulkanDevice> device, VkImage image);

private:
    VkImageViewCreateInfo createInfo;
};
} // common::vulkan_wrapper