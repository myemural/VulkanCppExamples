/**
 * @file    VulkanImage.h
 * @brief   This file contains wrapper class implementation for VkImage.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanDeviceMemory;

class VulkanImage final : public VulkanObject<VulkanDevice, VkImage>
{
public:
    VulkanImage(std::shared_ptr<VulkanDevice> device, VkImage image);

    ~VulkanImage() override;

    [[nodiscard]] VkMemoryRequirements GetImageMemoryRequirements() const;

    void BindImageMemory(const std::shared_ptr<VulkanDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset) const;

    [[nodiscard]] VkImageMemoryBarrier
    CreateImageMemoryBarrier(const VkAccessFlags& srcAccessMask,
                             const VkAccessFlags& dstAccessMask,
                             const VkImageLayout& oldLayout,
                             const VkImageLayout& newLayout,
                             const std::optional<VkImageSubresourceRange>& subresourceRange = std::nullopt) const;
};

class VulkanImageBuilder
{
public:
    VulkanImageBuilder();

    VulkanImageBuilder& SetCreateFlags(const VkImageCreateFlags& flags);

    VulkanImageBuilder& SetImageType(const VkImageType& imageType);

    VulkanImageBuilder& SetFormat(const VkFormat& format);

    VulkanImageBuilder& SetDimensions(std::uint32_t width, std::uint32_t height, std::uint32_t depth = 1);

    VulkanImageBuilder& SetMipLevels(std::uint32_t mipLevels);

    VulkanImageBuilder& SetArrayLayers(std::uint32_t arrayLayers);

    VulkanImageBuilder& SetSampleCount(const VkSampleCountFlagBits& sampleCountBits);

    VulkanImageBuilder& SetImageTiling(const VkImageTiling& tiling);

    VulkanImageBuilder& SetImageUsageFlags(const VkImageUsageFlags& usageFlags);

    VulkanImageBuilder& SetSharingMode(const VkSharingMode& sharingMode);

    VulkanImageBuilder& SetQueueFamilyIndices(const std::vector<std::uint32_t>& queueFamilyIndices);

    VulkanImageBuilder& SetInitialImageLayout(const VkImageLayout& layout);

    [[nodiscard]] std::shared_ptr<VulkanImage> Build(std::shared_ptr<VulkanDevice> device) const;

private:
    VkImageCreateInfo createInfo_;
};
} // namespace common::vulkan_wrapper
