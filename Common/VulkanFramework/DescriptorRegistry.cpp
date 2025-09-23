/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "DescriptorRegistry.h"

namespace common::vulkan_framework
{
DescriptorRegistry::DescriptorRegistry(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device)
    : device_{device}
{}

void DescriptorRegistry::CreateDescriptors(const DescriptorResourceCreateInfo &createInfo)
{
    CreatePool(createInfo.MaxSets, createInfo.PoolSizes);

    for (const auto &[name, bindings]: createInfo.Layouts) {
        AddLayout(name, bindings);
        CreateDescriptorSet(name);
    }
}

void DescriptorRegistry::CreatePool(const std::uint32_t maxSets, const std::vector<VkDescriptorPoolSize> &poolSizes,
                                    const VkDescriptorPoolCreateFlags &flags)
{
    descPool_ = device_->CreateDescriptorPool(maxSets, poolSizes, flags);

    if (!descPool_) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

DescriptorRegistry &DescriptorRegistry::AddLayout(const std::string &layoutName,
                                                  const std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
    descriptorSetsLayouts_[layoutName] = device_->CreateDescriptorSetLayout(bindings);
    if (!descriptorSetsLayouts_[layoutName]) {
        throw std::runtime_error("Failed to add new descriptor set layout!");
    }

    return *this;
}

void DescriptorRegistry::CreateDescriptorSet(const std::string &layoutName)
{
    const auto descSets = descPool_->CreateDescriptorSets({descriptorSetsLayouts_[layoutName]});
    if (descSets.empty()) {
        throw std::runtime_error("Failed to create descriptor sets!");
    }
    descriptorSets_[layoutName] = descSets.front();
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> DescriptorRegistry::GetDescriptorLayout(
    const std::string &layoutName)
{
    return descriptorSetsLayouts_.at(layoutName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> DescriptorRegistry::GetDescriptorSet(const std::string &setName)
{
    return descriptorSets_.at(setName);
}
} // common::vulkan_framework