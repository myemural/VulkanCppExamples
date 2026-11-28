/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanPipeline.h"

#include "VulkanDevice.h"
#include "VulkanPipelineCache.h"
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

inline VkComputePipelineCreateInfo GetDefaultComputePipelineCreateInfo()
{
    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = GetDefaultShaderStageCreateInfo();
    createInfo.layout = VK_NULL_HANDLE;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;
    return createInfo;
}

inline VkRayTracingPipelineCreateInfoKHR GetDefaultRayTracingPipelineCreateInfo()
{
    VkRayTracingPipelineCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stageCount = 0;
    createInfo.pStages = nullptr;
    createInfo.groupCount = 0;
    createInfo.pGroups = nullptr;
    createInfo.maxPipelineRayRecursionDepth = 1;
    createInfo.pLibraryInfo = nullptr;
    createInfo.pLibraryInterface = nullptr;
    createInfo.pDynamicState = nullptr;
    createInfo.layout = VK_NULL_HANDLE;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;
    return createInfo;
}

inline VkRayTracingShaderGroupCreateInfoKHR GetDefaultRayTracingShaderGroupCreateInfo()
{
    VkRayTracingShaderGroupCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    createInfo.generalShader = VK_SHADER_UNUSED_KHR;
    createInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
    createInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
    createInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
    createInfo.pShaderGroupCaptureReplayHandle = nullptr;
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

std::vector<std::uint8_t> VulkanPipeline::GetRayTracingShaderGroupHandles(const std::uint32_t firstGroup,
                                                                          const std::uint32_t groupCount,
                                                                          const std::size_t dataSize) const
{
    const auto device = GetParent();
    if (!device) {
        throw std::runtime_error("Device is null!");
    }

    const auto pfnGetRayTracingShaderGroupHandles = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
            device->GetDeviceProcAddr("vkGetRayTracingShaderGroupHandlesKHR"));
    if (pfnGetRayTracingShaderGroupHandles == nullptr) {
        throw std::runtime_error("vkGetRayTracingShaderGroupHandlesKHR couldn't be loaded!");
    }

    // Handles are returned tightly packed (each one is shaderGroupHandleSize bytes)
    std::vector<std::uint8_t> handles(dataSize);
    if (pfnGetRayTracingShaderGroupHandles(device->GetHandle(), handle_, firstGroup, groupCount, dataSize,
                                           handles.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to get ray tracing shader group handles!");
    }

    return handles;
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

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetCreateFlags(const VkPipelineCreateFlags& flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanGraphicsPipelineBuilder&
VulkanGraphicsPipelineBuilder::AddShaderStage(const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc)
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = GetDefaultShaderStageCreateInfo();
    builderFunc(shaderStageCreateInfo);
    shaderStages_.push_back(shaderStageCreateInfo);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetVertexInputState(
        const std::function<void(VkPipelineVertexInputStateCreateInfo&)>& builderFunc)
{
    builderFunc(vertexInputState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetInputAssemblyState(
        const std::function<void(VkPipelineInputAssemblyStateCreateInfo&)>& builderFunc)
{
    builderFunc(inputAssemblyState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetTessellationState(
        const std::function<void(VkPipelineTessellationStateCreateInfo&)>& builderFunc)
{
    builderFunc(tessellationState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetViewportState(
        const std::function<void(VkPipelineViewportStateCreateInfo&)>& builderFunc)
{
    builderFunc(viewportState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetRasterizationState(
        const std::function<void(VkPipelineRasterizationStateCreateInfo&)>& builderFunc)
{
    builderFunc(rasterizationState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetMultisampleState(
        const std::function<void(VkPipelineMultisampleStateCreateInfo&)>& builderFunc)
{
    builderFunc(multisampleState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDepthStencilState(
        const std::function<void(VkPipelineDepthStencilStateCreateInfo&)>& builderFunc)
{
    builderFunc(depthStencilState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetColorBlendState(
        const std::function<void(VkPipelineColorBlendStateCreateInfo&)>& builderFunc)
{
    builderFunc(colorBlendState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetDynamicState(
        const std::function<void(VkPipelineDynamicStateCreateInfo&)>& builderFunc)
{
    builderFunc(dynamicState_);
    return *this;
}

VulkanGraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::SetSubpassIndex(const std::uint32_t subpassIndex)
{
    createInfo_.subpass = subpassIndex;
    return *this;
}

VulkanGraphicsPipelineBuilder&
VulkanGraphicsPipelineBuilder::SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                               const std::int32_t basePipelineIndex)
{
    createInfo_.basePipelineHandle = basePipeline->GetHandle();
    createInfo_.basePipelineIndex = basePipelineIndex;
    return *this;
}

std::shared_ptr<VulkanPipeline>
VulkanGraphicsPipelineBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                     const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                     const std::shared_ptr<VulkanRenderPass>& renderPass,
                                     const std::shared_ptr<VulkanPipelineCache>& pipelineCache)
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

    const VkPipelineCache cache = pipelineCache ? pipelineCache->GetHandle() : VK_NULL_HANDLE;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(device->GetHandle(), cache, 1, &createInfo_, nullptr, &graphicsPipeline) !=
        VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanPipeline>(std::move(device), graphicsPipeline);
}

VulkanComputePipelineBuilder::VulkanComputePipelineBuilder()
    : createInfo_{GetDefaultComputePipelineCreateInfo()}, shaderStage_{GetDefaultShaderStageCreateInfo()}
{
}

VulkanComputePipelineBuilder& VulkanComputePipelineBuilder::SetCreateFlags(const VkPipelineCreateFlags& flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanComputePipelineBuilder&
VulkanComputePipelineBuilder::SetShaderStage(const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc)
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = GetDefaultShaderStageCreateInfo();
    builderFunc(shaderStageCreateInfo);
    shaderStage_ = shaderStageCreateInfo;
    return *this;
}

VulkanComputePipelineBuilder&
VulkanComputePipelineBuilder::SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                              const std::int32_t basePipelineIndex)
{
    createInfo_.basePipelineHandle = basePipeline->GetHandle();
    createInfo_.basePipelineIndex = basePipelineIndex;
    return *this;
}

std::shared_ptr<VulkanPipeline>
VulkanComputePipelineBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                    const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                    const std::shared_ptr<VulkanPipelineCache>& pipelineCache)
{
    createInfo_.stage = shaderStage_;
    createInfo_.layout = pipelineLayout->GetHandle();

    const VkPipelineCache cache = pipelineCache ? pipelineCache->GetHandle() : VK_NULL_HANDLE;

    VkPipeline computePipeline = VK_NULL_HANDLE;
    if (vkCreateComputePipelines(device->GetHandle(), cache, 1, &createInfo_, nullptr, &computePipeline) !=
        VK_SUCCESS) {
        std::cerr << "Failed to create compute pipeline!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanPipeline>(std::move(device), computePipeline);
}

VulkanRayTracingPipelineBuilder::VulkanRayTracingPipelineBuilder()
    : createInfo_{GetDefaultRayTracingPipelineCreateInfo()}
{
}

VulkanRayTracingPipelineBuilder& VulkanRayTracingPipelineBuilder::SetCreateFlags(const VkPipelineCreateFlags& flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanRayTracingPipelineBuilder& VulkanRayTracingPipelineBuilder::AddShaderStage(
        const std::function<void(VkPipelineShaderStageCreateInfo&)>& builderFunc)
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = GetDefaultShaderStageCreateInfo();
    builderFunc(shaderStageCreateInfo);
    shaderStages_.push_back(shaderStageCreateInfo);
    return *this;
}

VulkanRayTracingPipelineBuilder&
VulkanRayTracingPipelineBuilder::AddGeneralShaderGroup(const std::uint32_t generalShaderIndex)
{
    VkRayTracingShaderGroupCreateInfoKHR shaderGroup = GetDefaultRayTracingShaderGroupCreateInfo();
    shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroup.generalShader = generalShaderIndex;
    shaderGroups_.push_back(shaderGroup);
    return *this;
}

VulkanRayTracingPipelineBuilder&
VulkanRayTracingPipelineBuilder::AddTrianglesHitShaderGroup(const std::uint32_t closestHitShaderIndex,
                                                            const std::uint32_t anyHitShaderIndex)
{
    VkRayTracingShaderGroupCreateInfoKHR shaderGroup = GetDefaultRayTracingShaderGroupCreateInfo();
    shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    shaderGroup.closestHitShader = closestHitShaderIndex;
    shaderGroup.anyHitShader = anyHitShaderIndex;
    shaderGroups_.push_back(shaderGroup);
    return *this;
}

VulkanRayTracingPipelineBuilder&
VulkanRayTracingPipelineBuilder::AddProceduralHitShaderGroup(const std::uint32_t intersectionShaderIndex,
                                                             const std::uint32_t closestHitShaderIndex,
                                                             const std::uint32_t anyHitShaderIndex)
{
    VkRayTracingShaderGroupCreateInfoKHR shaderGroup = GetDefaultRayTracingShaderGroupCreateInfo();
    shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    shaderGroup.intersectionShader = intersectionShaderIndex;
    shaderGroup.closestHitShader = closestHitShaderIndex;
    shaderGroup.anyHitShader = anyHitShaderIndex;
    shaderGroups_.push_back(shaderGroup);
    return *this;
}

VulkanRayTracingPipelineBuilder&
VulkanRayTracingPipelineBuilder::SetMaxRecursionDepth(const std::uint32_t maxRecursionDepth)
{
    createInfo_.maxPipelineRayRecursionDepth = maxRecursionDepth;
    return *this;
}

VulkanRayTracingPipelineBuilder&
VulkanRayTracingPipelineBuilder::SetBasePipeline(const std::shared_ptr<VulkanPipeline>& basePipeline,
                                                 const std::int32_t basePipelineIndex)
{
    createInfo_.basePipelineHandle = basePipeline->GetHandle();
    createInfo_.basePipelineIndex = basePipelineIndex;
    return *this;
}

std::uint32_t VulkanRayTracingPipelineBuilder::GetShaderGroupCount() const
{
    return static_cast<std::uint32_t>(shaderGroups_.size());
}

std::shared_ptr<VulkanPipeline>
VulkanRayTracingPipelineBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                       const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                       const std::shared_ptr<VulkanPipelineCache>& pipelineCache)
{
    const auto pfnCreateRayTracingPipelines = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            device->GetDeviceProcAddr("vkCreateRayTracingPipelinesKHR"));
    if (pfnCreateRayTracingPipelines == nullptr) {
        std::cerr << "vkCreateRayTracingPipelinesKHR couldn't be loaded!" << std::endl;
        return nullptr;
    }

    createInfo_.stageCount = static_cast<std::uint32_t>(shaderStages_.size());
    createInfo_.pStages = shaderStages_.empty() ? nullptr : shaderStages_.data();
    createInfo_.groupCount = static_cast<std::uint32_t>(shaderGroups_.size());
    createInfo_.pGroups = shaderGroups_.empty() ? nullptr : shaderGroups_.data();
    createInfo_.layout = pipelineLayout->GetHandle();

    const VkPipelineCache cache = pipelineCache ? pipelineCache->GetHandle() : VK_NULL_HANDLE;

    VkPipeline rayTracingPipeline = VK_NULL_HANDLE;
    if (pfnCreateRayTracingPipelines(device->GetHandle(), VK_NULL_HANDLE, cache, 1, &createInfo_, nullptr,
                                     &rayTracingPipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create ray tracing pipeline!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanPipeline>(std::move(device), rayTracingPipeline);
}

} // namespace common::vulkan_wrapper
