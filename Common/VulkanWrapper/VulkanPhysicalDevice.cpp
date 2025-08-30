/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanPhysicalDevice.h"

#include <iostream>
#include <utility>

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"

namespace common::vulkan_wrapper
{

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice const physicalDevice)
    : VulkanObject(nullptr, physicalDevice)
{
}

std::uint32_t VulkanPhysicalDevice::FindMemoryType(std::uint32_t typeFilter, const VkMemoryPropertyFlags &properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(handle_, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

VkPhysicalDeviceProperties VulkanPhysicalDevice::GetProperties() const
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(handle_, &props);
    return props;
}

std::vector<VkQueueFamilyProperties> VulkanPhysicalDevice::GetQueueFamilyProperties() const
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(handle_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties{queueFamilyCount};
    vkGetPhysicalDeviceQueueFamilyProperties(handle_, &queueFamilyCount, queueFamilyProperties.data());

    return queueFamilyProperties;
}

std::uint32_t VulkanPhysicalDevice::GetSurfaceSupportedQueueFamilyIndex(const VkSurfaceKHR &surface) const
{
    const auto queueFamilyProperties = GetQueueFamilyProperties();

    std::vector<std::uint32_t> queueFamilyIndices;
    for (uint32_t queueFamilyIndex = 0; const auto &familyProperty: queueFamilyProperties) {
        if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.emplace_back(queueFamilyIndex);
        }
        ++queueFamilyIndex;
    }

    for (const auto &familyIndex: queueFamilyIndices) {
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(handle_, familyIndex, surface, &presentSupport);
        if (presentSupport) {
            return familyIndex;
        }
    }

    return UINT32_MAX;
}

std::optional<VkSurfaceCapabilitiesKHR> VulkanPhysicalDevice::GetSurfaceCapabilities(VkSurfaceKHR surface) const
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle_, surface, &surfaceCapabilities) != VK_SUCCESS) {
        std::cerr << "Failed to get surface capabilities!" << std::endl;
        return std::nullopt;
    }

    return surfaceCapabilities;
}

std::optional<VkSurfaceFormatKHR> VulkanPhysicalDevice::GetSurfaceFormat(const VkSurfaceKHR &surface,
                                                                         const VkFormat &selectedFormat,
                                                                         const VkColorSpaceKHR &selectedColorSpace) const
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle_, surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle_, surface, &formatCount, surfaceFormats.data());

    for (VkSurfaceFormatKHR &entry: surfaceFormats) {
        if (entry.format == selectedFormat && entry.colorSpace == selectedColorSpace) {
            return entry;
        }
    }

    return std::nullopt;
}

VkPhysicalDeviceFeatures VulkanPhysicalDevice::GetSupportedFeatures() const
{
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(handle_, &supportedFeatures);
    return supportedFeatures;
}

std::shared_ptr<VulkanDevice> VulkanPhysicalDevice::CreateDevice(
    const std::function<void(VulkanDeviceBuilder &)> &builderFunc)
{
    VulkanDeviceBuilder builder;
    builderFunc(builder);

    auto device = builder.Build(shared_from_this());
    // devices_.push_back(device);
    return device;
}

VulkanPhysicalDeviceSelector &VulkanPhysicalDeviceSelector::FilterByDeviceType(
    const VkPhysicalDeviceType &deviceType)
{
    deviceType_ = deviceType;
    return *this;
}

VulkanPhysicalDeviceSelector &VulkanPhysicalDeviceSelector::FilterByQueueTypes(const VkQueueFlags &queueTypeFlags)
{
    queueTypeFlags_ = queueTypeFlags;
    return *this;
}

VulkanPhysicalDeviceSelector &VulkanPhysicalDeviceSelector::FilterBySurfaceSupport(
    std::shared_ptr<VulkanSurface> surface)
{
    surface_ = std::move(surface);
    return *this;
}

std::vector<std::shared_ptr<VulkanPhysicalDevice> > VulkanPhysicalDeviceSelector::Select(
    const std::shared_ptr<VulkanInstance> &instance) const
{
    // Query physical devices
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance->GetHandle(), &deviceCount, nullptr);
    if (deviceCount == 0 || result != VK_SUCCESS) {
        std::cerr << "Failed to find Vulkan supported GPUs!" << '\n';
        return {};
    }

    // Get all physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance->GetHandle(), &deviceCount, devices.data());
    if (devices.empty() || result != VK_SUCCESS) {
        std::cerr << "Failed to get Vulkan supported GPUs!" << '\n';
        return {};
    }

    // Filter by Device Type
    if (deviceType_ != VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM) {
        std::erase_if(devices, [=](auto &device) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            return properties.deviceType != deviceType_;
        });
    }

    // Filter by Queue Types
    if (queueTypeFlags_ != VK_QUEUE_FLAG_BITS_MAX_ENUM) {
        std::erase_if(devices, [=](auto &device) {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilyProperties{queueFamilyCount};
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

            for (const auto &familyProperty: queueFamilyProperties) {
                if (familyProperty.queueFlags & queueTypeFlags_) {
                    return false;
                }
            }
            return true;
        });
    }

    // Filter by Surface
    if (surface_) {
        std::erase_if(devices, [=](auto &device) {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilyProperties{queueFamilyCount};
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

            for (uint32_t queueFamilyIndex = 0; const auto &familyProperty: queueFamilyProperties) {
                if (familyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    VkBool32 presentSupport;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface_->GetHandle(),
                                                         &presentSupport);
                    if (presentSupport) {
                        return false;
                    }
                }
                ++queueFamilyIndex;
            }
            return true;
        });
    }

    if (devices.empty()) {
        std::cerr << "Failed to find Vulkan supported devices!" << '\n';
        return {};
    }

    std::vector<std::shared_ptr<VulkanPhysicalDevice> > physicalDevices(devices.size());
    for (size_t i = 0; i < devices.size(); ++i) {
        physicalDevices[i] = std::make_shared<VulkanPhysicalDevice>(devices[i]);
    }

    return physicalDevices;
}
} // common::vulkan_wrapper