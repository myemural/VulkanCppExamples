/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanDescriptorPool.h"

#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanDescriptorPool::VulkanDescriptorPool(std::shared_ptr<VulkanDevice> device, VkDescriptorPool descriptorPool)
    : VulkanObject(std::move(device), descriptorPool)
{
}

std::vector<std::shared_ptr<VulkanDescriptorSet>> VulkanDescriptorPool::CreateDescriptorSets(
        const std::vector<std::shared_ptr<VulkanDescriptorSetLayout>>& descriptorSetLayouts)
{
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = handle_;
    allocateInfo.descriptorSetCount = descriptorSetLayouts.size();

    std::vector<VkDescriptorSetLayout> vkDescSetLayouts(descriptorSetLayouts.size());
    for (size_t i = 0; i < descriptorSetLayouts.size(); i++) {
        vkDescSetLayouts[i] = descriptorSetLayouts[i]->GetHandle();
    }
    allocateInfo.pSetLayouts = vkDescSetLayouts.data();

    std::vector<VkDescriptorSet> vkDescSets(descriptorSetLayouts.size());
    const auto device = GetParent();
    if (!device || vkAllocateDescriptorSets(device->GetHandle(), &allocateInfo, vkDescSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets;
    for (auto& descSet: vkDescSets) {
        auto vulkanDescSet = std::make_shared<VulkanDescriptorSet>(shared_from_this(), descSet);
        descriptorSets.push_back(vulkanDescSet);
    }

    return descriptorSets;
}

void VulkanDescriptorPool::ResetDescriptorPool(const VkDescriptorPoolResetFlags& resetFlags) const
{
    const auto device = GetParent();
    if (!device || vkResetDescriptorPool(device->GetHandle(), handle_, resetFlags) != VK_SUCCESS) {
        throw std::runtime_error("Failed to reset descriptor pool!");
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyDescriptorPool(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}
} // namespace common::vulkan_wrapper
