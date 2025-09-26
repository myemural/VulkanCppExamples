/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanPipeline.h"

#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"

namespace common::vulkan_wrapper
{
inline VkGraphicsPipelineCreateInfo GetDefaultGraphicsPipelineCreateInfo()
{
    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stageCount = 0;
    createInfo.pStages = nullptr;
    createInfo.pVertexInputState = nullptr;
    createInfo.pInputAssemblyState = nullptr;
    createInfo.pTessellationState = nullptr;
    createInfo.pViewportState = nullptr;
    createInfo.pRasterizationState = nullptr;
    createInfo.pMultisampleState = nullptr;
    createInfo.pDepthStencilState = nullptr;
    createInfo.pColorBlendState = nullptr;
    createInfo.pDynamicState = nullptr;
    createInfo.layout = VK_NULL_HANDLE;
    createInfo.renderPass = VK_NULL_HANDLE;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;
    return createInfo;
}

inline VkPipelineShaderStageCreateInfo GetDefaultShaderStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    createInfo.module = VK_NULL_HANDLE;
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = nullptr;
    return createInfo;
}

inline VkPipelineVertexInputStateCreateInfo GetDefaultVertexInputStateCreateInfo()
{
    VkPipelineVertexInputStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.vertexBindingDescriptionCount = 0;
    createInfo.pVertexBindingDescriptions = nullptr;
    createInfo.vertexAttributeDescriptionCount = 0;
    createInfo.pVertexAttributeDescriptions = nullptr;
    return createInfo;
}

inline VkPipelineInputAssemblyStateCreateInfo GetDefaultInputAssemblyStateCreateInfo()
{
    VkPipelineInputAssemblyStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.primitiveRestartEnable = VK_FALSE;
    return createInfo;
}

inline VkPipelineTessellationStateCreateInfo GetDefaultTessellationStateCreateInfo()
{
    VkPipelineTessellationStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.patchControlPoints = 0;
    return createInfo;
}

inline VkPipelineViewportStateCreateInfo GetDefaultViewportStateCreateInfo()
{
    VkPipelineViewportStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.viewportCount = 0;
    createInfo.pViewports = nullptr;
    createInfo.scissorCount = 0;
    createInfo.pScissors = nullptr;
    return createInfo;
}

inline VkPipelineRasterizationStateCreateInfo GetDefaultRasterizationStateCreateInfo()
{
    VkPipelineRasterizationStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.depthClampEnable = VK_FALSE;
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    createInfo.polygonMode = VK_POLYGON_MODE_FILL;
    createInfo.cullMode = VK_CULL_MODE_NONE;
    createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    createInfo.depthBiasEnable = VK_FALSE;
    createInfo.depthBiasConstantFactor = 0.0;
    createInfo.depthBiasClamp = 0.0;
    createInfo.depthBiasSlopeFactor = 0.0;
    createInfo.lineWidth = 1.0f;
    return createInfo;
}

inline VkPipelineMultisampleStateCreateInfo GetDefaultMultisampleStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.sampleShadingEnable = VK_FALSE;
    createInfo.minSampleShading = 0.0;
    createInfo.pSampleMask = nullptr;
    createInfo.alphaToCoverageEnable = VK_FALSE;
    createInfo.alphaToOneEnable = VK_FALSE;
    return createInfo;
}

inline VkPipelineDepthStencilStateCreateInfo GetDefaultDepthStencilStateCreateInfo()
{
    VkPipelineDepthStencilStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.depthTestEnable = VK_FALSE;
    createInfo.depthWriteEnable = VK_FALSE;
    createInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    createInfo.depthBoundsTestEnable = VK_FALSE;
    createInfo.stencilTestEnable = VK_FALSE;
    createInfo.front = {};
    createInfo.back = {};
    createInfo.minDepthBounds = 0.0f;
    createInfo.maxDepthBounds = 1.0f;
    return createInfo;
}

inline VkPipelineColorBlendStateCreateInfo GetDefaultColorBlendStateCreateInfo()
{
    VkPipelineColorBlendStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.logicOpEnable = VK_FALSE;
    createInfo.logicOp = VK_LOGIC_OP_COPY;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments = nullptr;
    createInfo.blendConstants[0] = 0.0f;
    createInfo.blendConstants[1] = 0.0f;
    createInfo.blendConstants[2] = 0.0f;
    createInfo.blendConstants[3] = 0.0f;
    return createInfo;
}

inline VkPipelineDynamicStateCreateInfo GetDefaultDynamicStateCreateInfo()
{
    VkPipelineDynamicStateCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.dynamicStateCount = 0;
    createInfo.pDynamicStates = nullptr;
    return createInfo;
}

VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanDevice> device, VkPipeline pipeline)
    : VulkanObject(std::move(device), pipeline)
{
}

VulkanPipeline::~VulkanPipeline()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyPipeline(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder()
    : createInfo_{GetDefaultGraphicsPipelineCreateInfo()},
      vertexInputState_{GetDefaultVertexInputStateCreateInfo()},
      inputAssemblyState_{GetDefaultInputAssemblyStateCreateInfo()},
      tessellationState_{GetDefaultTessellationStateCreateInfo()},
      viewportState_{GetDefaultViewportStateCreateInfo()},
      rasterizationState_{GetDefaultRasterizationStateCreateInfo()},
      multisampleState_{GetDefaultMultisampleStateCreateInfo()},
      depthStencilState_{GetDefaultDepthStencilStateCreateInfo()},
      colorBlendState_{GetDefaultColorBlendStateCreateInfo()},
      dynamicState_{GetDefaultDynamicStateCreateInfo()}
{
}

VulkanGraphicsPipelineBuilder & VulkanGraphicsPipelineBuilder::SetCreateFlags(const VkPipelineCreateFlags &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::AddShaderStage(
    const std::function<void(VkPipelineShaderStageCreateInfo &)> &builderFunc)
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = GetDefaultShaderStageCreateInfo();
    builderFunc(shaderStageCreateInfo);
    shaderStages_.push_back(shaderStageCreateInfo);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetVertexInputState(
    const std::function<void(VkPipelineVertexInputStateCreateInfo &)> &builderFunc)
{
    builderFunc(vertexInputState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetInputAssemblyState(
    const std::function<void(VkPipelineInputAssemblyStateCreateInfo &)> &builderFunc)
{
    builderFunc(inputAssemblyState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetTessellationState(
    const std::function<void(VkPipelineTessellationStateCreateInfo &)> &builderFunc)
{
    builderFunc(tessellationState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetViewportState(
    const std::function<void(VkPipelineViewportStateCreateInfo &)> &builderFunc)
{
    builderFunc(viewportState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetRasterizationState(
    const std::function<void(VkPipelineRasterizationStateCreateInfo &)> &builderFunc)
{
    builderFunc(rasterizationState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetMultisampleState(
    const std::function<void(VkPipelineMultisampleStateCreateInfo &)> &builderFunc)
{
    builderFunc(multisampleState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetDepthStencilState(
    const std::function<void(VkPipelineDepthStencilStateCreateInfo &)> &builderFunc)
{
    builderFunc(depthStencilState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetColorBlendState(
    const std::function<void(VkPipelineColorBlendStateCreateInfo &)> &builderFunc)
{
    builderFunc(colorBlendState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetDynamicState(
    const std::function<void(VkPipelineDynamicStateCreateInfo &)> &builderFunc)
{
    builderFunc(dynamicState_);
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetSubpassIndex(const std::uint32_t subpassIndex)
{
    createInfo_.subpass = subpassIndex;
    return *this;
}

VulkanGraphicsPipelineBuilder &VulkanGraphicsPipelineBuilder::SetBasePipeline(
    const std::shared_ptr<VulkanPipeline> &basePipeline, const std::int32_t basePipelineIndex)
{
    createInfo_.basePipelineHandle = basePipeline->GetHandle();
    createInfo_.basePipelineIndex = basePipelineIndex;
    return *this;
}

std::shared_ptr<VulkanPipeline> VulkanGraphicsPipelineBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                                                     const std::shared_ptr<VulkanPipelineLayout> &
                                                                     pipelineLayout,
                                                                     const std::shared_ptr<VulkanRenderPass> &
                                                                     renderPass)
{
    if (shaderStages_.empty()) {
        std::cerr << "Please set at least 1 shader stage for pipeline!" << std::endl;
        return nullptr;
    }
    createInfo_.stageCount = shaderStages_.size();
    createInfo_.pStages = shaderStages_.data();
    createInfo_.pVertexInputState = &vertexInputState_;
    createInfo_.pInputAssemblyState = &inputAssemblyState_;
    createInfo_.pTessellationState = &tessellationState_;
    createInfo_.pViewportState = &viewportState_;
    createInfo_.pRasterizationState = &rasterizationState_;
    createInfo_.pMultisampleState = &multisampleState_;
    createInfo_.pDepthStencilState = &depthStencilState_;
    createInfo_.pColorBlendState = &colorBlendState_;
    createInfo_.pDynamicState = &dynamicState_;

    createInfo_.layout = pipelineLayout->GetHandle();
    createInfo_.renderPass = renderPass->GetHandle();

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &createInfo_, nullptr, &graphicsPipeline) !=
        VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanPipeline>(std::move(device), graphicsPipeline);
}
} // common::vulkan_wrapper
