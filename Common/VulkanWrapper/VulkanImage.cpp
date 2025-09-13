/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanImage.h"

#include "VulkanDevice.h"
#include "VulkanDeviceMemory.h"

namespace common::vulkan_wrapper
{
inline VkImageCreateInfo GetDefaultImageCreateInfo()
{
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = VK_FORMAT_UNDEFINED;
    createInfo.extent = VkExtent3D{1, 1, 1};
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return createInfo;
}

VulkanImage::VulkanImage(std::shared_ptr<VulkanDevice> device, VkImage image)
    : VulkanObject(std::move(device), image)
{
}

VulkanImage::~VulkanImage()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyImage(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VkMemoryRequirements VulkanImage::GetImageMemoryRequirements() const
{
    VkMemoryRequirements memoryReq;
    if (const auto device = GetParent()) {
        vkGetImageMemoryRequirements(device->GetHandle(), handle_, &memoryReq);
    }
    return memoryReq;
}

void VulkanImage::BindImageMemory(const std::shared_ptr<VulkanDeviceMemory> &deviceMemory,
                                  VkDeviceSize memoryOffset) const
{
    const auto device = GetParent();
    if (!device || vkBindImageMemory(device->GetHandle(), handle_, deviceMemory->GetHandle(), memoryOffset) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to bind image memory!");
    }
}

VkImageMemoryBarrier VulkanImage::CreateImageMemoryBarrier(const VkAccessFlags &srcAccessMask,
                                                           const VkAccessFlags &dstAccessMask,
                                                           const VkImageLayout &oldLayout,
                                                           const VkImageLayout &newLayout,
                                                           const std::optional<VkImageSubresourceRange> &
                                                           subresourceRange) const
{
    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            barrier.pNext = nullptr;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; /// TODO: Currently we are not using multiple queues
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = handle_;

    if (subresourceRange.has_value()) {
        const VkImageSubresourceRange &currentSubresourceRange = subresourceRange.value();
        barrier.subresourceRange.aspectMask = currentSubresourceRange.aspectMask;
        barrier.subresourceRange.baseMipLevel = currentSubresourceRange.baseMipLevel;
        barrier.subresourceRange.levelCount = currentSubresourceRange.levelCount;
        barrier.subresourceRange.baseArrayLayer = currentSubresourceRange.baseArrayLayer;
        barrier.subresourceRange.layerCount = currentSubresourceRange.layerCount;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
    }

    return barrier;
}

VulkanImageBuilder::VulkanImageBuilder()
    : createInfo_(GetDefaultImageCreateInfo())
{
}

VulkanImageBuilder & VulkanImageBuilder::SetCreateFlags(const VkImageCreateFlags &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetImageType(const VkImageType &imageType)
{
    createInfo_.imageType = imageType;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetFormat(const VkFormat &format)
{
    createInfo_.format = format;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetDimensions(const std::uint32_t width, const std::uint32_t height,
                                                      const std::uint32_t depth)
{
    createInfo_.extent = VkExtent3D{width, height, depth};
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetMipLevels(const std::uint32_t mipLevels)
{
    createInfo_.mipLevels = mipLevels;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetArrayLayers(const std::uint32_t arrayLayers)
{
    createInfo_.arrayLayers = arrayLayers;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetSampleCount(const VkSampleCountFlagBits &sampleCountBits)
{
    createInfo_.samples = sampleCountBits;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetImageTiling(const VkImageTiling &tiling)
{
    createInfo_.tiling = tiling;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetImageUsageFlags(const VkImageUsageFlags &usageFlags)
{
    createInfo_.usage = usageFlags;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetSharingMode(const VkSharingMode &sharingMode)
{
    createInfo_.sharingMode = sharingMode;
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetQueueFamilyIndices(const std::vector<std::uint32_t> &queueFamilyIndices)
{
    createInfo_.queueFamilyIndexCount = queueFamilyIndices.size();
    createInfo_.pQueueFamilyIndices = queueFamilyIndices.data();
    return *this;
}

VulkanImageBuilder &VulkanImageBuilder::SetInitialImageLayout(const VkImageLayout &layout)
{
    createInfo_.initialLayout = layout;
    return *this;
}

std::shared_ptr<VulkanImage> VulkanImageBuilder::Build(std::shared_ptr<VulkanDevice> device) const
{
    VkImage image = VK_NULL_HANDLE;
    if (vkCreateImage(device->GetHandle(), &createInfo_, nullptr, &image) != VK_SUCCESS) {
        std::cerr << "Failed to create image!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanImage>(std::move(device), image);
}
} // common::vulkan_wrapper
