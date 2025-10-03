/**
 * @file    VulkanPhysicalDevice.h
 * @brief   This file contains wrapper class implementation for VkPhysicalDevice.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <functional>
#include <optional>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanInstance;
class VulkanSurface;
class VulkanDevice;
class VulkanDeviceBuilder;

class VulkanPhysicalDevice final : public VulkanObject<void, VkPhysicalDevice>,
                                   public std::enable_shared_from_this<VulkanPhysicalDevice>
{
public:
    explicit VulkanPhysicalDevice(VkPhysicalDevice physicalDevice);

    ~VulkanPhysicalDevice() override = default;

    std::uint32_t FindMemoryType(std::uint32_t typeFilter, const VkMemoryPropertyFlags& properties) const;

    VkPhysicalDeviceProperties GetProperties() const;

    std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;

    std::uint32_t GetSurfaceSupportedQueueFamilyIndex(const VkSurfaceKHR& surface) const;

    std::optional<VkSurfaceCapabilitiesKHR> GetSurfaceCapabilities(VkSurfaceKHR surface) const;

    std::optional<VkSurfaceFormatKHR> GetSurfaceFormat(const VkSurfaceKHR& surface,
                                                       const VkFormat& selectedFormat,
                                                       const VkColorSpaceKHR& selectedColorSpace) const;

    VkPhysicalDeviceFeatures GetSupportedFeatures() const;

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidateFormats,
                                 const VkFormatFeatureFlags& features,
                                 const VkImageTiling& tiling = VK_IMAGE_TILING_OPTIMAL) const;

    std::shared_ptr<VulkanDevice> CreateDevice(const std::function<void(VulkanDeviceBuilder&)>& builderFunc);
};

class VulkanPhysicalDeviceSelector
{
public:
    VulkanPhysicalDeviceSelector() = default;

    VulkanPhysicalDeviceSelector& FilterByDeviceType(const VkPhysicalDeviceType& deviceType);

    VulkanPhysicalDeviceSelector& FilterByQueueTypes(const VkQueueFlags& queueTypeFlags);

    VulkanPhysicalDeviceSelector& FilterBySurfaceSupport(std::shared_ptr<VulkanSurface> surface);

    [[nodiscard]] std::vector<std::shared_ptr<VulkanPhysicalDevice>>
    Select(const std::shared_ptr<VulkanInstance>& instance) const;

private:
    VkPhysicalDeviceType deviceType_ = VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
    VkQueueFlags queueTypeFlags_ = VK_QUEUE_FLAG_BITS_MAX_ENUM;
    std::shared_ptr<VulkanSurface> surface_ = nullptr;
};
} // namespace common::vulkan_wrapper
