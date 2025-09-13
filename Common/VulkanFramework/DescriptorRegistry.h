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
    /**
     * @param device Refers VulkanDevice object.
     */
    explicit DescriptorRegistry(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device);

    /**
     * @brief Creates a descriptor pool with the specified settings.
     * @param maxSets Specifies the maximum number of descriptor sets that can be taken from the pool.
     * @param poolSizes It is a vector that specifies how many of each type of descriptor set can be created.
     * @param flags Specifies the creation flags of the descriptor pool.
     */
    void CreatePool(std::uint32_t maxSets, const std::vector<VkDescriptorPoolSize> &poolSizes,
                    const VkDescriptorPoolCreateFlags &flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    /**
     * @brief Creates a descriptor set layout and add to registry map.
     * @param layoutName Specifies the layout name that will be used as the key name later.
     * @param bindings Specifies descriptor set layout bindings.
     * @return Returns a reference to this object so that the function can be called repeatedly.
     */
    DescriptorRegistry &AddLayout(const std::string &layoutName,
                                  const std::vector<VkDescriptorSetLayoutBinding> &bindings);

    /**
     * @brief Creates a descriptor set and add to registry map.
     * @param layoutName Specifies the layout name to be used when creating a descriptor set.
     */
    void CreateDescriptorSet(const std::string &layoutName);

    /**
     * @brief Gets specified descriptor set layout.
     * @param layoutName Specifies the layout name to be taken.
     * @return Returns descriptor set layout.
     */
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> GetDescriptorLayout(const std::string &layoutName);

    /**
     * @brief Gets specified descriptor set.
     * @param setName Specifies the descriptor set name to be taken.
     * @return Returns descriptor set.
     */
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> GetDescriptorSet(const std::string &setName);

private:
    std::shared_ptr<vulkan_wrapper::VulkanDevice> device_;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorPool> descPool_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> > descriptorSetsLayouts_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> > descriptorSets_;
};
} // common::vulkan_framework
