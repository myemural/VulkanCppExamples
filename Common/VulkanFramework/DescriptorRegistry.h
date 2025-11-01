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

#include "CoreDefines.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"

namespace common::vulkan_framework
{

struct COMMON_API DescriptorResourceCreateInfo
{
    struct Layout
    {
        std::string Name;
        std::vector<VkDescriptorSetLayoutBinding> Bindings;
    };

    struct DescriptorSet
    {
        std::string Name;
        std::string LayoutName;
    };

    std::uint32_t MaxSets;
    std::vector<VkDescriptorPoolSize> PoolSizes;
    std::vector<Layout> Layouts;
    std::vector<DescriptorSet> DescriptorSets;
};

class COMMON_API DescriptorRegistry
{
public:
    /**
     * @param device Refers VulkanDevice object.
     */
    explicit DescriptorRegistry(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Creates descriptor sets and descriptors from given information.
     * @param createInfo Descriptor create information.
     */
    void CreateDescriptors(const DescriptorResourceCreateInfo& createInfo);

    /**
     * @brief Creates a descriptor pool with the specified settings.
     * @param maxSets Specifies the maximum number of descriptor sets that can be taken from the pool.
     * @param poolSizes It is a vector that specifies how many of each type of descriptor set can be created.
     * @param flags Specifies the creation flags of the descriptor pool.
     */
    void CreatePool(std::uint32_t maxSets,
                    const std::vector<VkDescriptorPoolSize>& poolSizes,
                    const VkDescriptorPoolCreateFlags& flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    /**
     * @brief Creates a descriptor set layout and add to registry map.
     * @param layoutName Specifies the layout name that will be used as the key name later.
     * @param bindings Specifies descriptor set layout bindings.
     * @return Returns a reference to this object so that the function can be called repeatedly.
     */
    DescriptorRegistry& CreateLayout(const std::string& layoutName,
                                     const std::vector<VkDescriptorSetLayoutBinding>& bindings);

    /**
     * @brief Creates a descriptor set and add to registry map.
     * @param descriptorSetName Name of the descriptor set.
     * @param layoutName Name of the layout that associated with the descriptor set.
     * @return Returns a reference to this object so that the function can be called repeatedly.
     */
    DescriptorRegistry& CreateSet(const std::string& descriptorSetName, const std::string& layoutName);

    /**
     * @brief Gets specified descriptor set layout.
     * @param layoutName Specifies the layout name to be taken.
     * @return Returns descriptor set layout.
     */
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> GetDescriptorLayout(const std::string& layoutName);

    /**
     * @brief Gets specified descriptor set.
     * @param setName Specifies the descriptor set name to be taken.
     * @return Returns descriptor set.
     */
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> GetDescriptorSet(const std::string& setName);

    /**
     * @brief Deletes a descriptor set layout.
     * @param layoutName Name of the descriptor set layout.
     */
    void DeleteDescriptorLayout(const std::string& layoutName);

    /**
     * @brief Deletes a descriptor set.
     * @param setName Name of the descriptor set .
     */
    void DeleteDescriptorSet(const std::string& setName);


private:
    std::shared_ptr<vulkan_wrapper::VulkanDevice> device_;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorPool> descPool_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>> descriptorSetLayouts_;
    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet>> descriptorSets_;
};
} // namespace common::vulkan_framework
