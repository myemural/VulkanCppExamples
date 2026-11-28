/**
 * @file    VulkanPipeline.h
 * @brief   This file contains wrapper class implementation for VkPipeline.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanPipelineCache;
class VulkanPipelineLayout;
class VulkanRenderPass;

class VulkanPipeline final : public VulkanObject<VulkanDevice, VkPipeline>
{
public:
    COMMON_API explicit VulkanPipeline(std::shared_ptr<VulkanDevice> device, VkPipeline pipeline);

    COMMON_API ~VulkanPipeline() override;

    [[nodiscard]] COMMON_API std::vector<std::uint8_t>
    GetRayTracingShaderGroupHandles(std::uint32_t firstGroup, std::uint32_t groupCount, std::size_t dataSize) const;
};

class COMMON_API VulkanGraphicsPipelineBuilder
{
public:
    VulkanGraphicsPipelineBuilder();

    VulkanGraphicsPipelineBuilder& SetCreateFlags(const VkPipelineCreateFlags& flags);

    VulkanGraphicsPipelineBuilder&
    AddShaderStage(const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetVertexInputState(const std::function<void(VkPipelineVertexInputStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetInputAssemblyState(const std::function<void(VkPipelineInputAssemblyStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetTessellationState(const std::function<void(VkPipelineTessellationStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetViewportState(const std::function<void(VkPipelineViewportStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetRasterizationState(const std::function<void(VkPipelineRasterizationStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetMultisampleState(const std::function<void(VkPipelineMultisampleStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetDepthStencilState(const std::function<void(VkPipelineDepthStencilStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetColorBlendState(const std::function<void(VkPipelineColorBlendStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder&
    SetDynamicState(const std::function<void(VkPipelineDynamicStateCreateInfo&)>& builderFunc);

    VulkanGraphicsPipelineBuilder& SetSubpassIndex(std::uint32_t subpassIndex);

    VulkanGraphicsPipelineBuilder& SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                                   std::int32_t basePipelineIndex);

    std::shared_ptr<VulkanPipeline> Build(std::shared_ptr<VulkanDevice> device,
                                          const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                          const std::shared_ptr<VulkanRenderPass>& renderPass,
                                          const std::shared_ptr<VulkanPipelineCache>& pipelineCache = nullptr);

private:
    VkGraphicsPipelineCreateInfo createInfo_;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;
    VkPipelineVertexInputStateCreateInfo vertexInputState_;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState_;
    VkPipelineTessellationStateCreateInfo tessellationState_;
    VkPipelineViewportStateCreateInfo viewportState_;
    VkPipelineRasterizationStateCreateInfo rasterizationState_;
    VkPipelineMultisampleStateCreateInfo multisampleState_;
    VkPipelineDepthStencilStateCreateInfo depthStencilState_;
    VkPipelineColorBlendStateCreateInfo colorBlendState_;
    VkPipelineDynamicStateCreateInfo dynamicState_;
};

class COMMON_API VulkanComputePipelineBuilder
{
public:
    VulkanComputePipelineBuilder();

    VulkanComputePipelineBuilder& SetCreateFlags(const VkPipelineCreateFlags& flags);

    VulkanComputePipelineBuilder&
    SetShaderStage(const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc);

    VulkanComputePipelineBuilder& SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                                  std::int32_t basePipelineIndex);

    std::shared_ptr<VulkanPipeline> Build(std::shared_ptr<VulkanDevice> device,
                                          const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                          const std::shared_ptr<VulkanPipelineCache>& pipelineCache = nullptr);

private:
    VkComputePipelineCreateInfo createInfo_;
    VkPipelineShaderStageCreateInfo shaderStage_;
};

class COMMON_API VulkanRayTracingPipelineBuilder
{
public:
    VulkanRayTracingPipelineBuilder();

    VulkanRayTracingPipelineBuilder& SetCreateFlags(const VkPipelineCreateFlags& flags);

    VulkanRayTracingPipelineBuilder&
    AddShaderStage(const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc);

    VulkanRayTracingPipelineBuilder& AddGeneralShaderGroup(std::uint32_t generalShaderIndex);

    VulkanRayTracingPipelineBuilder& AddTrianglesHitShaderGroup(std::uint32_t closestHitShaderIndex,
                                                                std::uint32_t anyHitShaderIndex = VK_SHADER_UNUSED_KHR);

    VulkanRayTracingPipelineBuilder&
    AddProceduralHitShaderGroup(std::uint32_t intersectionShaderIndex,
                                std::uint32_t closestHitShaderIndex = VK_SHADER_UNUSED_KHR,
                                std::uint32_t anyHitShaderIndex = VK_SHADER_UNUSED_KHR);

    VulkanRayTracingPipelineBuilder& SetMaxRecursionDepth(std::uint32_t maxRecursionDepth);

    VulkanRayTracingPipelineBuilder& SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                                     std::int32_t basePipelineIndex);

    [[nodiscard]] std::uint32_t GetShaderGroupCount() const;

    std::shared_ptr<VulkanPipeline> Build(std::shared_ptr<VulkanDevice> device,
                                          const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                          const std::shared_ptr<VulkanPipelineCache>& pipelineCache = nullptr);

private:
    VkRayTracingPipelineCreateInfoKHR createInfo_;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages_;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups_;
};
} // namespace common::vulkan_wrapper
