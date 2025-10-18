/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "DescriptorRegistry.h"

namespace common::vulkan_framework
{
DescriptorRegistry::DescriptorRegistry(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device) : device_{device} {}

void DescriptorRegistry::CreateDescriptors(const DescriptorResourceCreateInfo& createInfo)
{
    CreatePool(createInfo.MaxSets, createInfo.PoolSizes);

    for (const auto& [name, bindings]: createInfo.Layouts) {
        CreateLayout(name, bindings);
    }

    for (const auto& [name, layoutName] : createInfo.DescriptorSets) {
        CreateSet(name, layoutName);
    }
}

void DescriptorRegistry::CreatePool(const std::uint32_t maxSets,
                                    const std::vector<VkDescriptorPoolSize>& poolSizes,
                                    const VkDescriptorPoolCreateFlags& flags)
{
    descPool_ = device_->CreateDescriptorPool(maxSets, poolSizes, flags);

    if (!descPool_) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

DescriptorRegistry& DescriptorRegistry::CreateLayout(const std::string& layoutName,
                                                  const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    descriptorSetLayouts_[layoutName] = device_->CreateDescriptorSetLayout(bindings);
    if (!descriptorSetLayouts_[layoutName]) {
        throw std::runtime_error("Failed to add new descriptor set layout!");
    }

    return *this;
}

DescriptorRegistry& DescriptorRegistry::CreateSet(const std::string& descriptorSetName, const std::string& layoutName)
{
    const auto descSets = descPool_->CreateDescriptorSets({descriptorSetLayouts_[layoutName]});
    if (descSets.empty()) {
        throw std::runtime_error("Failed to create descriptor sets!");
    }
    descriptorSets_[descriptorSetName] = descSets.front();

    return *this;
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>
DescriptorRegistry::GetDescriptorLayout(const std::string& layoutName)
{
    return descriptorSetLayouts_.at(layoutName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> DescriptorRegistry::GetDescriptorSet(const std::string& setName)
{
    return descriptorSets_.at(setName);
}

void DescriptorRegistry::DeleteDescriptorLayout(const std::string& layoutName) { descriptorSetLayouts_.erase(layoutName); }

void DescriptorRegistry::DeleteDescriptorSet(const std::string& setName) { descriptorSets_.erase(setName); }
} // namespace common::vulkan_framework
