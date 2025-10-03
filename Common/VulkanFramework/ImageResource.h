/**
 * @file    ImageResource.h
 * @brief   This file contains the implementation of the ImageResource class, which performs operations related to
 *          creating a Vulkan image (and image view), allocating it, and loading data into the image.
 * @author  Mustafa Yemural (myemural)
 * @date    8.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "VulkanDevice.h"
#include "VulkanDeviceMemory.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanPhysicalDevice.h"

namespace common::vulkan_framework
{
struct ImageViewCreateInfo
{
    std::string ViewName;
    VkImageViewCreateFlags CreateFlags = 0;
    VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    VkComponentMapping Components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    VkImageSubresourceRange SubresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
};

struct ImageResourceCreateInfo
{
    std::string Name;
    VkMemoryPropertyFlags MemProperties;
    VkImageCreateFlags CreateFlags = 0;
    VkImageType ImageType = VK_IMAGE_TYPE_2D;
    VkFormat Format = VK_FORMAT_UNDEFINED;
    VkExtent3D Dimensions = {};
    std::uint32_t MipLevels = 1;
    std::uint32_t ArrayLayers = 1;
    VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags UsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    /// TODO: SharingMode and QueueFamilyIndices will be added later.
    std::vector<ImageViewCreateInfo> Views;
};

class ImageResource
{
public:
    ImageResource(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                  const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    void CreateImage(const ImageResourceCreateInfo& createInfo);

    void ChangeImageLayout(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                           const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                           const VkImageLayout& oldLayout,
                           const VkImageLayout& newLayout) const;

    void CopyDataFromBuffer(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                            const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                            const std::shared_ptr<vulkan_wrapper::VulkanBuffer>& stagingBuffer,
                            const VkBufferImageCopy& copyRegion) const;

    [[nodiscard]] std::string GetName() const { return name_; }

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImage> GetImage() const { return image_; }

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImageView> GetImageView(const std::string& viewName) const;

private:
    void AllocateImageMemory();

    std::weak_ptr<vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::string name_;
    std::shared_ptr<vulkan_wrapper::VulkanImage> image_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<vulkan_wrapper::VulkanImageView>> imageViews_;
    VkMemoryPropertyFlags memProps_ = 0;
    std::shared_ptr<vulkan_wrapper::VulkanDeviceMemory> deviceMemory_ = nullptr;
};
} // namespace common::vulkan_framework
