/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanImageView.h"

#include "VulkanDevice.h"
#include "VulkanImage.h"

namespace common::vulkan_wrapper
{
inline VkImageViewCreateInfo GetDefaultImageViewCreateInfo()
{
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_UNDEFINED;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.image = VK_NULL_HANDLE;
    return imageViewCreateInfo;
}

VulkanImageView::VulkanImageView(std::shared_ptr<VulkanDevice> device, VkImageView imageView)
    : VulkanObject(std::move(device), imageView)
{
}

VulkanImageView::~VulkanImageView()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyImageView(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VulkanImageViewBuilder::VulkanImageViewBuilder() : createInfo_(GetDefaultImageViewCreateInfo()) {}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetCreateFlags(const VkImageViewCreateFlags& flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetImageViewType(const VkImageViewType& viewType)
{
    createInfo_.viewType = viewType;
    return *this;
}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetFormat(const VkFormat& format)
{
    createInfo_.format = format;
    return *this;
}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetComponents(const VkComponentMapping& components)
{
    createInfo_.components = components;
    return *this;
}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetSubresourceRange(const VkImageSubresourceRange& subresourceRange)
{
    createInfo_.subresourceRange = subresourceRange;
    return *this;
}

VulkanImageViewBuilder& VulkanImageViewBuilder::SetImage(const std::shared_ptr<VulkanImage>& image)
{
    createInfo_.image = image->GetHandle();
    return *this;
}

std::shared_ptr<VulkanImageView> VulkanImageViewBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                                               const std::shared_ptr<VulkanImage>& image)
{
    createInfo_.image = image->GetHandle();

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(device->GetHandle(), &createInfo_, nullptr, &imageView) != VK_SUCCESS) {
        std::cerr << "Failed to create image view!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanImageView>(std::move(device), imageView);
}

std::shared_ptr<VulkanImageView> VulkanImageViewBuilder::Build(std::shared_ptr<VulkanDevice> device, VkImage image)
{
    createInfo_.image = image;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(device->GetHandle(), &createInfo_, nullptr, &imageView) != VK_SUCCESS) {
        std::cerr << "Failed to create image view!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanImageView>(std::move(device), imageView);
}
} // namespace common::vulkan_wrapper
