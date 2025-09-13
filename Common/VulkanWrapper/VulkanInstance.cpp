/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanInstance.h"

#include <algorithm>
#include <iostream>

namespace common::vulkan_wrapper
{
inline VkInstanceCreateInfo GetDefaultInstanceCreateInfo()
{
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    return createInfo;
}

inline VkApplicationInfo GetDefaultApplicationInfo()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Default Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "DefaultEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    return appInfo;
}

VulkanInstance::VulkanInstance(VkInstance const instance)
    : VulkanObject(nullptr, instance)
{
}

VulkanInstance::~VulkanInstance()
{
    if (handle_ != VK_NULL_HANDLE) {
        vkDestroyInstance(handle_, nullptr);
    }
}

PFN_vkVoidFunction VulkanInstance::GetInstanceProcAddr(const std::string &name) const
{
    return vkGetInstanceProcAddr(handle_, name.c_str());
}

VulkanInstanceBuilder::VulkanInstanceBuilder()
    : createInfo_{GetDefaultInstanceCreateInfo()}, appInfo_{GetDefaultApplicationInfo()}
{
}

VulkanInstanceBuilder &VulkanInstanceBuilder::SetApplicationInfo(
    const std::function<void(VkApplicationInfo &)> &appInfoCallback)
{
    appInfoCallback(appInfo_);
    createInfo_.pApplicationInfo = &appInfo_;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::AddLayer(const std::string &layerName)
{
    layers_.emplace_back(layerName);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::AddLayers(const std::vector<std::string> &layerNames)
{
    layers_.insert(layers_.end(), layerNames.begin(), layerNames.end());
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::AddExtension(const std::string &extensionName)
{
    extensions_.emplace_back(extensionName);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::AddExtensions(const std::vector<std::string> &extensionNames)
{
    extensions_.insert(extensions_.end(), extensionNames.begin(), extensionNames.end());
    return *this;
}

std::shared_ptr<VulkanInstance> VulkanInstanceBuilder::Build()
{
    if (!layers_.empty()) {
        std::ranges::transform(layers_, std::back_inserter(layersStr_),
                               [](const std::string &s) { return s.c_str(); });
        createInfo_.enabledLayerCount = static_cast<uint32_t>(layersStr_.size());
        createInfo_.ppEnabledLayerNames = layersStr_.data();
    }

    if (!extensions_.empty()) {
        std::ranges::transform(extensions_, std::back_inserter(extensionsStr_),
                               [](const std::string &s) { return s.c_str(); });
        createInfo_.enabledExtensionCount = static_cast<uint32_t>(extensionsStr_.size());
        createInfo_.ppEnabledExtensionNames = extensionsStr_.data();
    }

    VkInstance instance;
    if (vkCreateInstance(&createInfo_, nullptr, &instance) != VK_SUCCESS) {
        std::cout << "Failed to create instance!" << '\n';
        return nullptr;
    }

    return std::make_shared<VulkanInstance>(instance);
}
} // common::vulkan_wrapper
