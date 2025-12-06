/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "MathUtils.h"
#include "TimeUtils.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::compute_shaders::fullscreen_gradient
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationComputeShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects();

        CreateResources();
        InitResources();

        CreateRenderPass();
        CreatePipelines();
        CreateDefaultFramebuffers(nullptr);

        CreateCommandBuffers();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void VulkanApplication::DrawFrame()
{
    inFlightFences_[currentIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::CreateResources()
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = quadVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t indexDataSize = quadIndices.size() * sizeof(quadIndices[0]);

    resourceCreateInfo.Buffers = {
        {GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::MainIndexBuffer), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
                                  .Modules = {{.Name = GetParamStr(AppConstants::MainVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::MainFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainFragmentShaderFile)},
                                              {.Name = GetParamStr(AppConstants::GradientComputeShaderKey),
                                               .FileName = GetParamStr(AppConstants::GradientComputeShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.Descriptors = {
        .MaxSets = 2,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}},
        .Layouts = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.Name = GetParamStr(AppConstants::ComputeDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .DescriptorSets = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                            .LayoutName = GetParamStr(AppConstants::MainDescSetLayout)},
                           {.Name = GetParamStr(AppConstants::ComputeDescSetLayout),
                            .LayoutName = GetParamStr(AppConstants::ComputeDescSetLayout)}}};

    resourceCreateInfo.Images = {ImageResourceCreateInfo{
        .Name = GetParamStr(AppConstants::GradientStorageImage),
        .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .Format = VK_FORMAT_R8G8B8A8_UNORM,
        .Dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .UsageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::GradientStorageImageView),
                                      .Format = VK_FORMAT_R8G8B8A8_UNORM}}}};

    resourceCreateInfo.Samplers = {
        {.Name = GetParamStr(AppConstants::MainSampler),
         .FilteringBehavior = {.MagFilter = VK_FILTER_LINEAR, .MinFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitResources() const
{
    resources_->SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), quadVertices.data(),
                          quadVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::MainIndexBuffer), quadIndices.data(),
                          quadIndices.size() * sizeof(quadIndices[0]));

    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
               })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange timePushConstant;
    timePushConstant.offset = 0;
    timePushConstant.size = sizeof(float);
    timePushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    gradientComputePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::ComputeDescSetLayout))}, {timePushConstant});

    if (!gradientComputePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    gradientComputePipeline_ = device_->CreateComputePipeline(gradientComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::GradientComputeShaderKey))->GetHandle();
        });
    });

    if (!gradientComputePipeline_) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    graphicsPipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))});

    if (!graphicsPipelineLayout_) {
        throw std::runtime_error("Failed to create graphics pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos3Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

    graphicsPipeline_ = device_->CreateGraphicsPipeline(graphicsPipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::MainFragmentShaderKey))->GetHandle();
        });
        builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        });
        builder.SetViewportState([&](auto& viewportStateCreateInfo) {
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = &viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = &scissor;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
    });

    if (!graphicsPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> imageSamplerInfos;
    imageSamplerInfos.emplace_back(resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
                                   resources_
                                           ->GetImageView(GetParamStr(AppConstants::GradientStorageImage),
                                                          GetParamStr(AppConstants::GradientStorageImageView))
                                           ->GetHandle(),
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> imageStorageInfos;
    imageStorageInfos.emplace_back(VK_NULL_HANDLE,
                                   resources_
                                           ->GetImageView(GetParamStr(AppConstants::GradientStorageImage),
                                                          GetParamStr(AppConstants::GradientStorageImageView))
                                           ->GetHandle(),
                                   VK_IMAGE_LAYOUT_GENERAL);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.DescriptorSetName = GetParamStr(AppConstants::MainDescSetLayout);
    samplerUpdateRequest.BindingIndex = 0;
    samplerUpdateRequest.Images = imageSamplerInfos;
    samplerUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest storageUpdateRequest;
    storageUpdateRequest.DescriptorSetName = GetParamStr(AppConstants::ComputeDescSetLayout);
    storageUpdateRequest.BindingIndex = 0;
    storageUpdateRequest.Images = imageStorageInfos;
    storageUpdateRequest.Type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .ImageWriteRequests = {samplerUpdateRequest, storageUpdateRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Compute phase
    {
        // Change image layout from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_GENERAL
        const auto gradientStorageImage = resources_->GetImage(GetParamStr(AppConstants::GradientStorageImage));
        const auto barrierUndefinedToGeneral = gradientStorageImage->CreateImageMemoryBarrier(
                0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          {barrierUndefinedToGeneral});

        currentCmdBuffer->BindPipeline(gradientComputePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::ComputeDescSetLayout))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, gradientComputePipelineLayout_, 0,
                                             descSets);

        const auto time = static_cast<float>(GetCurrentTime());
        currentCmdBuffer->PushConstants(gradientComputePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(float),
                                        &time);


        const auto groupCountX = CeilDiv(currentWindowWidth_, LOCAL_SIZE_X);
        const auto groupCountY = CeilDiv(currentWindowHeight_, LOCAL_SIZE_Y);
        currentCmdBuffer->Dispatch(groupCountX, groupCountY, 1);
    }

    // Render phase
    {
        // Change image layout from VK_IMAGE_LAYOUT_GENERAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        const auto gradientStorageImage = resources_->GetImage(GetParamStr(AppConstants::GradientStorageImage));
        const auto barrierGeneralToShaderRead = gradientStorageImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          {barrierGeneralToShaderRead});

        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);
        currentCmdBuffer->BindPipeline(graphicsPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::MainDescSetLayout))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers{resources_->GetBuffer(GetParamStr(AppConstants::MainVertexBuffer))};
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});
        currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::MainIndexBuffer)));
        currentCmdBuffer->DrawIndexed(quadIndices.size(), 1, 0, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}
} // namespace examples::fundamentals::compute_shaders::fullscreen_gradient
