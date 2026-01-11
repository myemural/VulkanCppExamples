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

#include "CoreDefines.h"
#include "VulkanDevice.h"
#include "VulkanDeviceMemory.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanPhysicalDevice.h"

namespace common::vulkan_framework
{
struct COMMON_API ImageViewCreateInfo
{
    std::string viewName;
    VkImageViewCreateFlags createFlags = 0;
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
};

struct COMMON_API ImageResourceCreateInfo
{
    std::string name;
    VkMemoryPropertyFlags memProperties;
    VkImageCreateFlags createFlags = 0;
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent3D dimensions = {};
    std::uint32_t mipLevels = 1;
    std::uint32_t arrayLayers = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    /// TODO: SharingMode and QueueFamilyIndices will be added later.
    std::vector<ImageViewCreateInfo> views;
};

class COMMON_API ImageResource
{
public:
    /**
     * @param physicalDevice Refers VulkanPhysicalDevice object.
     * @param device Refers VulkanDevice object.
     */
    ImageResource(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                  const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Creates images from given information.
     * @param createInfo Image create information.
     */
    void CreateImage(const ImageResourceCreateInfo& createInfo);

    /**
     * @brief Changes image layout from old one to new one.
     * @param cmdPool Command pool that the command buffer will be created.
     * @param queue Queue that the command buffer will be sent.
     * @param oldLayout Old image layout.
     * @param newLayout New image layout.
     * @param subresourceRanges Subresource ranges for the image.
     */
    void ChangeImageLayout(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                           const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                           const VkImageLayout& oldLayout,
                           const VkImageLayout& newLayout,
                           const std::optional<VkImageSubresourceRange>& subresourceRanges = std::nullopt) const;

    /**
     * @brief Copies data from staging buffer to the image on device.
     * @param cmdPool Command pool that the command buffer will be created.
     * @param queue Queue that the command buffer will be sent.
     * @param stagingBuffer Source buffer to be copied.
     * @param copyRegion Copy region on the buffer.
     */
    void CopyDataFromBuffer(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                            const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                            const std::shared_ptr<vulkan_wrapper::VulkanBuffer>& stagingBuffer,
                            const VkBufferImageCopy& copyRegion) const;

    /**
     * @brief Deletes image view from the resource.
     * @param imageViewName Image view name.
     */
    void DeleteImageView(const std::string& imageViewName);

    /**
     * @brief Returns name of the image resource.
     * @return Returns name of the image resource.
     */
    [[nodiscard]] std::string GetName() const { return name_; }

    /**
     * @brief Returns the image resource.
     * @return Returns the image resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImage> GetImage() const { return image_; }

    /**
     * @brief Returns the image view of the image resource.
     * @param viewName Name of the image view.
     * @return Returns the image view of the image resource.
     */
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
