/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanCommandBuffer.h"

#include <algorithm>

#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"

namespace common::vulkan_wrapper
{
VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<VulkanCommandPool> cmdPool, VkCommandBuffer cmdBuffer)
    : VulkanObject(std::move(cmdPool), cmdBuffer)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto pool = GetParent()) {
            if (const auto device = pool->GetParent()) {
                vkDeviceWaitIdle(device->GetHandle());
                vkFreeCommandBuffers(device->GetHandle(), pool->GetHandle(), 1, &handle_);
                handle_ = VK_NULL_HANDLE;
            }
        }
    }
}

bool
VulkanCommandBuffer::BeginCommandBuffer(const std::function<void(VkCommandBufferBeginInfo&)>& beginInfoCallback) const
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (beginInfoCallback) {
        beginInfoCallback(beginInfo);
    }

    if (vkBeginCommandBuffer(handle_, &beginInfo) != VK_SUCCESS) {
        std::cerr << "Failed to begin recording command buffer!" << std::endl;
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::EndCommandBuffer() const
{
    if (vkEndCommandBuffer(handle_) != VK_SUCCESS) {
        std::cerr << "Failed to end recording command buffer!" << std::endl;
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::ResetCommandBuffer(const VkCommandBufferResetFlags& resetFlags) const
{
    if (vkResetCommandBuffer(handle_, resetFlags)) {
        std::cerr << "Failed to reset command buffer!" << std::endl;
        return false;
    }
    return true;
}

void VulkanCommandBuffer::BeginRenderPass(const std::function<void(VkRenderPassBeginInfo&)>& beginInfoCallback,
                                          const VkSubpassContents& subpassContents) const
{
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfoCallback(beginInfo);
    vkCmdBeginRenderPass(handle_, &beginInfo, subpassContents);
}

void VulkanCommandBuffer::EndRenderPass() const { vkCmdEndRenderPass(handle_); }

void VulkanCommandBuffer::NextSubpass(const VkSubpassContents& subpassContents) const
{
    vkCmdNextSubpass(handle_, subpassContents);
}

void VulkanCommandBuffer::BindDescriptorSets(const VkPipelineBindPoint& pipelineBindPoint,
                                             const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                             const std::uint32_t firstSet,
                                             const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descriptorSets,
                                             const std::vector<std::uint32_t>& dynamicOffsets) const
{
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSets.size());
    for (std::size_t i = 0; i < descriptorSets.size(); ++i) {
        vkDescriptorSets[i] = descriptorSets[i]->GetHandle();
    }

    vkCmdBindDescriptorSets(handle_, pipelineBindPoint, pipelineLayout->GetHandle(), firstSet, descriptorSets.size(),
                            vkDescriptorSets.empty() ? nullptr : vkDescriptorSets.data(), dynamicOffsets.size(),
                            dynamicOffsets.empty() ? nullptr : dynamicOffsets.data());
}

void VulkanCommandBuffer::BindIndexBuffer(const std::shared_ptr<VulkanBuffer>& indexBuffer,
                                          const VkDeviceSize& offset,
                                          const VkIndexType& indexType) const
{
    vkCmdBindIndexBuffer(handle_, indexBuffer->GetHandle(), offset, indexType);
}

void VulkanCommandBuffer::BindPipeline(const std::shared_ptr<VulkanPipeline>& pipeline,
                                       const VkPipelineBindPoint& bindPoint) const
{
    vkCmdBindPipeline(handle_, bindPoint, pipeline->GetHandle());
}

void VulkanCommandBuffer::BindVertexBuffers(const std::vector<std::shared_ptr<VulkanBuffer>>& vertexBuffers,
                                            const std::uint32_t firstBinding,
                                            const std::uint32_t bindingCount,
                                            const std::vector<VkDeviceSize>& offsets) const
{
    std::vector<VkBuffer> vkVertexBuffers(vertexBuffers.size());
    std::ranges::transform(vertexBuffers, vkVertexBuffers.begin(),
                           [](const auto& buffer) { return buffer->GetHandle(); });

    vkCmdBindVertexBuffers(handle_, firstBinding, bindingCount, vkVertexBuffers.data(), offsets.data());
}

void VulkanCommandBuffer::CopyBuffer(const std::shared_ptr<VulkanBuffer>& srcBuffer,
                                     const std::shared_ptr<VulkanBuffer>& dstBuffer,
                                     const std::vector<VkBufferCopy>& regions) const
{
    vkCmdCopyBuffer(handle_, srcBuffer->GetHandle(), dstBuffer->GetHandle(), regions.size(),
                    regions.empty() ? nullptr : regions.data());
}

void VulkanCommandBuffer::CopyBufferToImage(const std::shared_ptr<VulkanBuffer>& srcBuffer,
                                            const std::shared_ptr<VulkanImage>& dstImage,
                                            const VkImageLayout& imageLayout,
                                            const std::vector<VkBufferImageCopy>& regions) const
{
    vkCmdCopyBufferToImage(handle_, srcBuffer->GetHandle(), dstImage->GetHandle(), imageLayout, regions.size(),
                           regions.empty() ? nullptr : regions.data());
}

void VulkanCommandBuffer::Draw(const std::uint32_t vertexCount,
                               const std::uint32_t instanceCount,
                               const std::uint32_t firstVertex,
                               const std::uint32_t firstInstance) const
{
    vkCmdDraw(handle_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::DrawIndexed(const std::uint32_t indexCount,
                                      const std::uint32_t instanceCount,
                                      const std::uint32_t firstIndex,
                                      const std::int32_t vertexOffset,
                                      const std::uint32_t firstInstance) const
{
    vkCmdDrawIndexed(handle_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::PipelineBarrier(const VkPipelineStageFlags& srcStage,
                                          const VkPipelineStageFlags& dstStage,
                                          const std::vector<VkImageMemoryBarrier>& imageMemoryBarrier,
                                          const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
                                          const std::vector<VkMemoryBarrier>& memoryBarriers,
                                          const VkDependencyFlags& dependencyFlags) const
{
    vkCmdPipelineBarrier(handle_, srcStage, dstStage, dependencyFlags, memoryBarriers.size(),
                         memoryBarriers.empty() ? nullptr : memoryBarriers.data(), bufferMemoryBarriers.size(),
                         bufferMemoryBarriers.empty() ? nullptr : bufferMemoryBarriers.data(),
                         imageMemoryBarrier.size(), imageMemoryBarrier.empty() ? nullptr : imageMemoryBarrier.data());
}

void VulkanCommandBuffer::PushConstants(const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                        const VkShaderStageFlags& stageFlags,
                                        const std::uint32_t offset,
                                        const std::uint32_t size,
                                        const void* values) const
{
    vkCmdPushConstants(handle_, pipelineLayout->GetHandle(), stageFlags, offset, size, values);
}

void VulkanCommandBuffer::SetBlendConstants(const float r, const float g, const float b, const float a) const
{
    const float blendConstants[4] = {r, g, b, a};
    vkCmdSetBlendConstants(handle_, blendConstants);
}

void VulkanCommandBuffer::SetLineWidth(const float lineWidth) const { vkCmdSetLineWidth(handle_, lineWidth); }

void VulkanCommandBuffer::ClearAttachments(const std::vector<VkClearAttachment>& attachments,
                                           const std::vector<VkClearRect>& rects) const
{
    vkCmdClearAttachments(handle_, attachments.size(), attachments.empty() ? nullptr : attachments.data(), rects.size(),
                          rects.empty() ? nullptr : rects.data());
}
} // namespace common::vulkan_wrapper
