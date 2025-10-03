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
        const auto set = registry_.GetDescriptorSet(req.LayoutName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.BindingIndex;
        write.dstArrayElement = req.ArrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.Buffers.size());
        write.descriptorType = req.Type;
        write.pBufferInfo = req.Buffers.empty() ? nullptr : req.Buffers.data();
        writes.push_back(write);
    }

    // Image write requests
    for (auto& req: imageRequests_) {
        const auto set = registry_.GetDescriptorSet(req.LayoutName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.BindingIndex;
        write.dstArrayElement = req.ArrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.Images.size());
        write.descriptorType = req.Type;
        write.pImageInfo = req.Images.empty() ? nullptr : req.Images.data();
        writes.push_back(write);
    }

    // Texel buffer write requests
    for (auto& req: texelRequests_) {
        const auto set = registry_.GetDescriptorSet(req.LayoutName);
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set->GetHandle();
        write.dstBinding = req.BindingIndex;
        write.dstArrayElement = req.ArrayElement;
        write.descriptorCount = static_cast<uint32_t>(req.BufferViews.size());
        write.descriptorType = req.Type;
        write.pTexelBufferView = req.BufferViews.empty() ? nullptr : req.BufferViews.data();
        writes.push_back(write);
    }

    // Copy requests
    for (auto& req: copyRequests_) {
        VkCopyDescriptorSet copy{};
        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        copy.srcSet = req.SourceSet->GetHandle();
        copy.srcBinding = req.SourceBindingIndex;
        copy.srcArrayElement = req.SourceArrayElement;
        copy.dstSet = req.DestSet->GetHandle();
        copy.dstBinding = req.DestBindingIndex;
        copy.dstArrayElement = req.DestArrayElement;
        copy.descriptorCount = req.Count;
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
