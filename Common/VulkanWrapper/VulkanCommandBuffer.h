/**
 * @file    VulkanCommandBuffer.h
 * @brief   This file contains wrapper class implementation for VkCommandBuffer.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <functional>
#include <memory>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanImage;
class VulkanBuffer;
class VulkanCommandPool;
class VulkanDescriptorSet;
class VulkanPipeline;
class VulkanPipelineLayout;

class VulkanCommandBuffer final : public VulkanObject<VulkanCommandPool, VkCommandBuffer>
{
public:
    VulkanCommandBuffer(std::shared_ptr<VulkanCommandPool> cmdPool, VkCommandBuffer cmdBuffer);

    ~VulkanCommandBuffer() override;

    bool BeginCommandBuffer(const std::function<void(VkCommandBufferBeginInfo &)> &beginInfoCallback) const;

    [[nodiscard]] bool EndCommandBuffer() const;

    [[nodiscard]] bool ResetCommandBuffer(const VkCommandBufferResetFlags &resetFlags = 0) const;

    void BeginRenderPass(const std::function<void(VkRenderPassBeginInfo &)> &beginInfoCallback,
                         const VkSubpassContents &subpassContents) const;

    void EndRenderPass() const;

    void BindDescriptorSets(const VkPipelineBindPoint &pipelineBindPoint,
                            const std::shared_ptr<VulkanPipelineLayout> &pipelineLayout,
                            std::uint32_t firstSet,
                            const std::vector<std::shared_ptr<VulkanDescriptorSet> > &descriptorSets = {},
                            const std::vector<std::uint32_t> &dynamicOffsets = {}) const;

    void BindIndexBuffer(const std::shared_ptr<VulkanBuffer> &indexBuffer, const VkDeviceSize &offset,
                         const VkIndexType &indexType) const;

    void BindPipeline(const std::shared_ptr<VulkanPipeline> &pipeline, const VkPipelineBindPoint &bindPoint) const;

    void BindVertexBuffers(const std::vector<std::shared_ptr<VulkanBuffer> > &vertexBuffers, std::uint32_t firstBinding,
                           std::uint32_t bindingCount, const std::vector<VkDeviceSize> &offsets) const;

    void CopyBuffer(const std::shared_ptr<VulkanBuffer> &srcBuffer, const std::shared_ptr<VulkanBuffer> &dstBuffer,
                    const std::vector<VkBufferCopy> &regions) const;

    void CopyBufferToImage(const std::shared_ptr<VulkanBuffer> &srcBuffer, const std::shared_ptr<VulkanImage> &dstImage,
                           const VkImageLayout &imageLayout, const std::vector<VkBufferImageCopy> &regions) const;

    void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex,
              std::uint32_t firstInstance) const;

    void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex,
                     std::int32_t vertexOffset, std::uint32_t firstInstance) const;

    void PipelineBarrier(const VkPipelineStageFlags &srcStage, const VkPipelineStageFlags &dstStage,
                         const std::vector<VkImageMemoryBarrier> &imageMemoryBarrier,
                         const std::vector<VkBufferMemoryBarrier> &bufferMemoryBarriers = {},
                         const std::vector<VkMemoryBarrier> &memoryBarriers = {},
                         const VkDependencyFlags &dependencyFlags = 0) const;

    void PushConstants(const std::shared_ptr<VulkanPipelineLayout> &pipelineLayout,
                       const VkShaderStageFlags &stageFlags, std::uint32_t offset, std::uint32_t size,
                       const void *values) const;
};
} // common::vulkan_wrapper