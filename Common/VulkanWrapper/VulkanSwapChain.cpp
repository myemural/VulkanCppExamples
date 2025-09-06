/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanSwapChain.h"

#include "VulkanDevice.h"
#include "VulkanImageView.h"
#include "VulkanSurface.h"
#include "VulkanSemaphore.h"
#include "VulkanFence.h"

namespace common::vulkan_wrapper
{
inline VkSwapchainCreateInfoKHR GetDefaultSwapChainCreateInfo()
{
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = nullptr;
    createInfo.minImageCount = 1;
    createInfo.imageFormat = VK_FORMAT_UNDEFINED;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = VkExtent2D{};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    return createInfo;
}

VulkanSwapChain::VulkanSwapChain(std::shared_ptr<VulkanDevice> device, VkSwapchainKHR const swapChain)
    : VulkanObject(std::move(device), swapChain)
{
}

VulkanSwapChain::~VulkanSwapChain()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroySwapchainKHR(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

void VulkanSwapChain::SetSwapChainImageViews(const std::shared_ptr<VulkanDevice> &device, const VkFormat &imageFormat)
{
    std::vector<VkImage> swapImages;
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(device->GetHandle(), handle_, &swapChainImageCount, nullptr);
    swapImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device->GetHandle(), handle_, &swapChainImageCount, swapImages.data());

    for (const auto swapImage: swapImages) {
        std::shared_ptr<VulkanImageView> imageView = VulkanImageViewBuilder()
                .SetFormat(imageFormat)
                .Build(device, swapImage);
        swapChainImageViews_.emplace_back(imageView);
    }
}

std::vector<std::shared_ptr<VulkanImageView> > VulkanSwapChain::GetSwapChainImageViews() const
{
    return swapChainImageViews_;
}

std::uint32_t VulkanSwapChain::AcquireNextImage(const std::shared_ptr<VulkanSemaphore> &semaphore,
                                                const std::shared_ptr<VulkanFence> &fence,
                                                const std::uint64_t timeout) const
{
    const auto device = GetParent();

    std::uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(device->GetHandle(), handle_, timeout, semaphore ? semaphore->GetHandle() : VK_NULL_HANDLE,
                          fence ? fence->GetHandle() : VK_NULL_HANDLE, &imageIndex);
    return imageIndex;
}

VulkanSwapChainBuilder::VulkanSwapChainBuilder()
    : createInfo_(GetDefaultSwapChainCreateInfo())
{
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetCreateFlags(const VkSwapchainCreateFlagsKHR &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetMinImageCount(const std::uint32_t minImageCount)
{
    createInfo_.minImageCount = minImageCount;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageFormat(const VkFormat &format)
{
    createInfo_.imageFormat = format;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageColorSpace(const VkColorSpaceKHR &colorSpace)
{
    createInfo_.imageColorSpace = colorSpace;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageExtent(const std::uint32_t width, const std::uint32_t height)
{
    createInfo_.imageExtent = VkExtent2D(width, height);
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageArrayLayers(const std::uint32_t imageArrayLayers)
{
    createInfo_.imageArrayLayers = imageArrayLayers;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageUsageFlags(const VkImageUsageFlags &imageUsage)
{
    createInfo_.imageUsage = imageUsage;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetImageSharingMode(const VkSharingMode &sharingMode)
{
    createInfo_.imageSharingMode = sharingMode;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetQueueFamilyIndices(const std::vector<std::uint32_t> &familyIndices)
{
    createInfo_.queueFamilyIndexCount = familyIndices.size();
    createInfo_.pQueueFamilyIndices = familyIndices.data();
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetPreTransformFlagBits(const VkSurfaceTransformFlagBitsKHR &flagBits)
{
    createInfo_.preTransform = flagBits;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetCompositeAlphaFlagBits(
    const VkCompositeAlphaFlagBitsKHR &compositeAlpha)
{
    createInfo_.compositeAlpha = compositeAlpha;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetPresentMode(const VkPresentModeKHR &presentMode)
{
    createInfo_.presentMode = presentMode;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetClipped(bool isClipped)
{
    createInfo_.clipped = isClipped;
    return *this;
}

VulkanSwapChainBuilder &VulkanSwapChainBuilder::SetOldSwapChain(const std::shared_ptr<VulkanSwapChain> &oldSwapChain)
{
    createInfo_.oldSwapchain = oldSwapChain->GetHandle();
    return *this;
}

std::shared_ptr<VulkanSwapChain> VulkanSwapChainBuilder::Build(const std::shared_ptr<VulkanDevice> &device,
                                                               const std::shared_ptr<VulkanSurface> &surface)
{
    createInfo_.surface = surface->GetHandle();

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(device->GetHandle(), &createInfo_, nullptr, &swapChain) != VK_SUCCESS) {
        std::cout << "Failed to create swap chain!" << std::endl;
        return nullptr;
    }

    auto vulkanSwapChain = std::make_shared<VulkanSwapChain>(device, swapChain);
    vulkanSwapChain->SetSwapChainImageViews(device, createInfo_.imageFormat);
    return vulkanSwapChain;
}
} // common::vulkan_wrapper
