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

#include "CoreDefines.h"
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
    COMMON_API VulkanCommandBuffer(std::shared_ptr<VulkanCommandPool> cmdPool, VkCommandBuffer cmdBuffer);

    COMMON_API ~VulkanCommandBuffer() override;

    COMMON_API bool BeginCommandBuffer(const std::function<void(VkCommandBufferBeginInfo&)>& beginInfoCallback) const;

    [[nodiscard]] COMMON_API bool EndCommandBuffer() const;

    [[nodiscard]] COMMON_API bool ResetCommandBuffer(const VkCommandBufferResetFlags& resetFlags = 0) const;

    COMMON_API void BeginRenderPass(const std::function<void(VkRenderPassBeginInfo&)>& beginInfoCallback,
                         const VkSubpassContents& subpassContents) const;

    COMMON_API void EndRenderPass() const;

    COMMON_API void NextSubpass(const VkSubpassContents& subpassContents) const;

    COMMON_API void BindDescriptorSets(const VkPipelineBindPoint& pipelineBindPoint,
                            const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                            std::uint32_t firstSet,
                            const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descriptorSets = {},
                            const std::vector<std::uint32_t>& dynamicOffsets = {}) const;

    COMMON_API void BindIndexBuffer(const std::shared_ptr<VulkanBuffer>& indexBuffer,
                         const VkDeviceSize& offset = 0,
                         const VkIndexType& indexType = VK_INDEX_TYPE_UINT16) const;

    COMMON_API void BindPipeline(const std::shared_ptr<VulkanPipeline>& pipeline, const VkPipelineBindPoint& bindPoint) const;

    COMMON_API void BindVertexBuffers(const std::vector<std::shared_ptr<VulkanBuffer>>& vertexBuffers,
                           std::uint32_t firstBinding,
                           std::uint32_t bindingCount,
                           const std::vector<VkDeviceSize>& offsets) const;

    COMMON_API void CopyBuffer(const std::shared_ptr<VulkanBuffer>& srcBuffer,
                    const std::shared_ptr<VulkanBuffer>& dstBuffer,
                    const std::vector<VkBufferCopy>& regions) const;

    COMMON_API void CopyBufferToImage(const std::shared_ptr<VulkanBuffer>& srcBuffer,
                           const std::shared_ptr<VulkanImage>& dstImage,
                           const VkImageLayout& imageLayout,
                           const std::vector<VkBufferImageCopy>& regions) const;

    COMMON_API void Draw(std::uint32_t vertexCount,
              std::uint32_t instanceCount,
              std::uint32_t firstVertex,
              std::uint32_t firstInstance) const;

    COMMON_API void DrawIndexed(std::uint32_t indexCount,
                     std::uint32_t instanceCount,
                     std::uint32_t firstIndex,
                     std::int32_t vertexOffset,
                     std::uint32_t firstInstance) const;

    COMMON_API void PipelineBarrier(const VkPipelineStageFlags& srcStage,
                         const VkPipelineStageFlags& dstStage,
                         const std::vector<VkImageMemoryBarrier>& imageMemoryBarrier,
                         const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers = {},
                         const std::vector<VkMemoryBarrier>& memoryBarriers = {},
                         const VkDependencyFlags& dependencyFlags = 0) const;

    COMMON_API void PushConstants(const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                       const VkShaderStageFlags& stageFlags,
                       std::uint32_t offset,
                       std::uint32_t size,
                       const void* values) const;

    COMMON_API void SetBlendConstants(float r, float g, float b, float a) const;

    COMMON_API void SetLineWidth(float lineWidth) const;

    COMMON_API void ClearAttachments(const std::vector<VkClearAttachment>& attachments,
                          const std::vector<VkClearRect>& rects) const;

    COMMON_API void SetViewports(std::uint32_t firstViewport, const std::vector<VkViewport>& viewports) const;

    COMMON_API void SetScissors(std::uint32_t firstScissor, const std::vector<VkRect2D>& scissors) const;
};
} // namespace common::vulkan_wrapper
