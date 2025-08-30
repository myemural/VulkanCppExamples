/**
 * @file    VulkanSwapChain.h
 * @brief   This file contains wrapper class implementation for VkSwapchainKHR.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{

class VulkanDevice;
class VulkanImageView;
class VulkanFence;
class VulkanSemaphore;
class VulkanSurface;

class VulkanSwapChain final : public VulkanObject<VulkanDevice, VkSwapchainKHR>
{
public:
    VulkanSwapChain(std::shared_ptr<VulkanDevice> device, VkSwapchainKHR swapChain);

    ~VulkanSwapChain() override;

    void SetSwapChainImageViews(const std::shared_ptr<VulkanDevice> &device, const VkFormat& imageFormat);

    [[nodiscard]] std::vector<std::shared_ptr<VulkanImageView>> GetSwapChainImageViews() const;

    [[nodiscard]] std::uint32_t AcquireNextImage(const std::shared_ptr<VulkanSemaphore> &semaphore, const std::shared_ptr<VulkanFence> &fence, std::uint64_t timeout = UINT64_MAX) const;

private:
    std::vector<std::shared_ptr<VulkanImageView>> swapChainImageViews_;
};

class VulkanSwapChainBuilder
{
public:
    VulkanSwapChainBuilder();

    VulkanSwapChainBuilder& SetCreateFlags(const VkSwapchainCreateFlagsKHR& flags);

    VulkanSwapChainBuilder& SetMinImageCount(std::uint32_t minImageCount);

    VulkanSwapChainBuilder& SetImageFormat(const VkFormat& format);

    VulkanSwapChainBuilder& SetImageColorSpace(const VkColorSpaceKHR& colorSpace);

    VulkanSwapChainBuilder& SetImageExtent(std::uint32_t width, std::uint32_t height);

    VulkanSwapChainBuilder& SetImageArrayLayers(std::uint32_t imageArrayLayers);

    VulkanSwapChainBuilder& SetImageUsageFlags(const VkImageUsageFlags& imageUsage);

    VulkanSwapChainBuilder& SetImageSharingMode(const VkSharingMode& sharingMode);

    VulkanSwapChainBuilder& SetQueueFamilyIndices(const std::vector<std::uint32_t>& familyIndices);

    VulkanSwapChainBuilder& SetPreTransformFlagBits(const VkSurfaceTransformFlagBitsKHR& flagBits);

    VulkanSwapChainBuilder& SetCompositeAlphaFlagBits(const VkCompositeAlphaFlagBitsKHR& compositeAlpha);

    VulkanSwapChainBuilder& SetPresentMode(const VkPresentModeKHR& presentMode);

    VulkanSwapChainBuilder& SetClipped(bool isClipped);

    VulkanSwapChainBuilder& SetOldSwapChain(const std::shared_ptr<VulkanSwapChain> &oldSwapChain);

    std::shared_ptr<VulkanSwapChain> Build(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<VulkanSurface> &surface);

private:
    VkSwapchainCreateInfoKHR createInfo{};
};
} // common::vulkan_wrapper