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

#include <functional>
#include <memory>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;
class VulkanPipelineLayout;
class VulkanRenderPass;

class VulkanPipeline final : public VulkanObject<VulkanDevice, VkPipeline>
{
public:
    explicit VulkanPipeline(std::shared_ptr<VulkanDevice> device, VkPipeline pipeline);

    ~VulkanPipeline() override;
};

class VulkanGraphicsPipelineBuilder
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
                                          const std::shared_ptr<VulkanRenderPass>& renderPass);

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
} // namespace common::vulkan_wrapper
