/**
 * @file    DescriptorRegistry.h
 * @brief   This class contains a registry about all descriptor set layouts and descriptor sets.
 * @author  Mustafa Yemural (myemural)
 * @date    23.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <map>

#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSet.h"

namespace common::vulkan_framework
{

class DescriptorRegistry
{
public:
    explicit DescriptorRegistry(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device);

    void CreatePool(std::uint32_t maxSets, const std::vector<VkDescriptorPoolSize> &poolSizes,
                    const VkDescriptorPoolCreateFlags &flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    DescriptorRegistry &AddLayout(const std::string &layoutName,
                                  const std::vector<VkDescriptorSetLayoutBinding> &bindings);

    void CreateDescriptorSet(const std::string& layoutName);

    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> GetDescriptorLayout(const std::string& layoutName);

    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> GetDescriptorSet(const std::string& setName);

private:
    std::shared_ptr<vulkan_wrapper::VulkanDevice> device_;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorPool> descPool_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>> descriptorSetsLayouts_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet>> descriptorSets_;
};

} // common::vulkan_framework
