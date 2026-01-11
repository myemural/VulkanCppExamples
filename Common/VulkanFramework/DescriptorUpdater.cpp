/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "DescriptorUpdater.h"

namespace common::vulkan_framework
{
DescriptorUpdater::DescriptorUpdater(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device,
                                     DescriptorRegistry& registry)
    : device_{device}, registry_(registry)
{
}

void DescriptorUpdater::ApplyUpdates()

{
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<VkCopyDescriptorSet> copies;

    // Buffer write requests
    for (auto& req: bufferRequests_) {
        const auto set = registry_.GetDescriptorSet(req.descriptorSetName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.bindingIndex;
        write.dstArrayElement = req.arrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.buffers.size());
        write.descriptorType = req.type;
        write.pBufferInfo = req.buffers.empty() ? nullptr : req.buffers.data();
        writes.push_back(write);
    }

    // Image write requests
    for (auto& req: imageRequests_) {
        const auto set = registry_.GetDescriptorSet(req.descriptorSetName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.bindingIndex;
        write.dstArrayElement = req.arrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.images.size());
        write.descriptorType = req.type;
        write.pImageInfo = req.images.empty() ? nullptr : req.images.data();
        writes.push_back(write);
    }

    // Texel buffer write requests
    for (auto& req: texelRequests_) {
        const auto set = registry_.GetDescriptorSet(req.descriptorSetName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.bindingIndex;
        write.dstArrayElement = req.arrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.bufferViews.size());
        write.descriptorType = req.type;
        write.pTexelBufferView = req.bufferViews.empty() ? nullptr : req.bufferViews.data();
        writes.push_back(write);
    }

    // Copy requests
    for (auto& req: copyRequests_) {
        VkCopyDescriptorSet copy{};
        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        copy.srcSet = req.sourceSet->GetHandle();
        copy.srcBinding = req.sourceBindingIndex;
        copy.srcArrayElement = req.sourceArrayElement;
        copy.dstSet = req.destSet->GetHandle();
        copy.dstBinding = req.destBindingIndex;
        copy.dstArrayElement = req.destArrayElement;
        copy.descriptorCount = req.count;
        copies.push_back(copy);
    }

    device_->UpdateDescriptorSets(writes, copies);

    // Clear all requests
    bufferRequests_.clear();
    imageRequests_.clear();
    texelRequests_.clear();
    copyRequests_.clear();
}
} // namespace common::vulkan_framework
