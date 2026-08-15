/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "MathUtils.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "TimeUtils.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationAdvancedComputeShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationAdvancedComputeShaders::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        CreateAndUpdateDescriptorSets();

        CreateRenderPass();
        CreatePipelines();
        CreateFramebuffers();
        CreateCommandBuffers();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void VulkanApplication::DrawFrame()
{
    inFlightFences_[currentFrameIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentFrameIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentFrameIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentFrameIndex_];

    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);
    const auto gameOfLifeComputeShaderAsset = assetManager_->Load<ShaderAsset>(kGameOfLifeComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)},
                    {.name = kGameOfLifeComputeShaderKey, .asset = assetManager_->Get(gameOfLifeComputeShaderAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kScreenStorageImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kScreenStorageImageView, .format = VK_FORMAT_R8G8B8A8_UNORM}}},
        ImageResourceCreateInfo{
            .name = kCellStateImageA,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8_UINT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kCellStateImageAView, .format = VK_FORMAT_R8_UINT}}},
        ImageResourceCreateInfo{
            .name = kCellStateImageB,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8_UINT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kCellStateImageBView, .format = VK_FORMAT_R8_UINT}}}};

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 6}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kComputeDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kComputeDescSetAToB, .layoutName = kComputeDescSetLayout},
                           {.name = kComputeDescSetBToA, .layoutName = kComputeDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorImageInfo> imageSamplerInfos;
    imageSamplerInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                   resources_->GetImageView(kScreenStorageImage, kScreenStorageImageView)->GetHandle(),
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> imageStorageInfos;
    imageStorageInfos.emplace_back(VK_NULL_HANDLE,
                                   resources_->GetImageView(kScreenStorageImage, kScreenStorageImageView)->GetHandle(),
                                   VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> stateAStorageInfos;
    stateAStorageInfos.emplace_back(VK_NULL_HANDLE,
                                    resources_->GetImageView(kCellStateImageA, kCellStateImageAView)->GetHandle(),
                                    VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> stateBStorageInfos;
    stateBStorageInfos.emplace_back(VK_NULL_HANDLE,
                                    resources_->GetImageView(kCellStateImageB, kCellStateImageBView)->GetHandle(),
                                    VK_IMAGE_LAYOUT_GENERAL);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.descriptorSetName = kMainDescSet;
    samplerUpdateRequest.bindingIndex = 0;
    samplerUpdateRequest.images = imageSamplerInfos;
    samplerUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest computeAToBPrev;
    computeAToBPrev.descriptorSetName = kComputeDescSetAToB;
    computeAToBPrev.bindingIndex = 0;
    computeAToBPrev.images = stateAStorageInfos;
    computeAToBPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    ImageWriteRequest computeAToBNext;
    computeAToBNext.descriptorSetName = kComputeDescSetAToB;
    computeAToBNext.bindingIndex = 1;
    computeAToBNext.images = stateBStorageInfos;
    computeAToBNext.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    ImageWriteRequest computeAToBDest;
    computeAToBDest.descriptorSetName = kComputeDescSetAToB;
    computeAToBDest.bindingIndex = 2;
    computeAToBDest.images = imageStorageInfos;
    computeAToBDest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    ImageWriteRequest computeBToAPrev;
    computeBToAPrev.descriptorSetName = kComputeDescSetBToA;
    computeBToAPrev.bindingIndex = 0;
    computeBToAPrev.images = stateBStorageInfos;
    computeBToAPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    ImageWriteRequest computeBToANext;
    computeBToANext.descriptorSetName = kComputeDescSetBToA;
    computeBToANext.bindingIndex = 1;
    computeBToANext.images = stateAStorageInfos;
    computeBToANext.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    ImageWriteRequest computeBToADest;
    computeBToADest.descriptorSetName = kComputeDescSetBToA;
    computeBToADest.bindingIndex = 2;
    computeBToADest.images = imageStorageInfos;
    computeBToADest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .imageWriteRequests = {samplerUpdateRequest, computeAToBPrev, computeAToBNext, computeAToBDest, computeBToAPrev,
                               computeBToANext, computeBToADest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
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
    VkPushConstantRange gameOfLifePushConstant;
    gameOfLifePushConstant.offset = 0;
    gameOfLifePushConstant.size = sizeof(GameOfLifePushConstants);
    gameOfLifePushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computePipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kComputeDescSetLayout)},
                                                           {gameOfLifePushConstant});

    if (!computePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    computePipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kGameOfLifeComputeShaderKey)->GetHandle();
        });
    });

    if (!computePipeline_) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    quadPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)});

    if (!quadPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    quadPipeline_ = device_->CreateGraphicsPipeline(quadPipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
        });
        builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
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
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        });
    });

    if (!quadPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for fullscreen quad)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Present framebuffers
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage}, [&](auto& builder) {
            builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer (for present)!");
        }

        presentFramebuffers_.push_back(framebuffer);
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(presentFramebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

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

    // Barrier phase for proper image layout transitions
    {
        const auto screenStorageImage = resources_->GetImage(kScreenStorageImage);
        const auto cellStateImageA = resources_->GetImage(kCellStateImageA);
        const auto cellStateImageB = resources_->GetImage(kCellStateImageB);

        if (frameCounter_ == 0) {
            // One-time UNDEFINED to GENERAL transitions for all three storage images
            const std::vector initialBarriers{
                screenStorageImage->CreateImageMemoryBarrier(0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                             VK_IMAGE_LAYOUT_GENERAL),
                cellStateImageA->CreateImageMemoryBarrier(0, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                                                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL),
                cellStateImageB->CreateImageMemoryBarrier(0, VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                                                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)};

            currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                              initialBarriers);
        } else {
            // For screen quad image, translate from SHADER_READ_ONLY to GENERAL
            const auto screenReadOnlyToGeneral = screenStorageImage->CreateImageMemoryBarrier(
                    VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_IMAGE_LAYOUT_GENERAL);

            // These are just for enabling read and write accesses for state images
            const auto stateAGeneralToGeneral = cellStateImageA->CreateImageMemoryBarrier(
                    VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL);
            const auto stateBGeneralToGeneral = cellStateImageB->CreateImageMemoryBarrier(
                    VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                    VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL);

            currentCmdBuffer->PipelineBarrier(
                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    {screenReadOnlyToGeneral, stateAGeneralToGeneral, stateBGeneralToGeneral});
        }
    }

    // Compute phase
    {
        currentCmdBuffer->BindPipeline(computePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);

        // Ping-pong: Even frames read from A and write to B, odd frames read from B write to A
        const bool isAtoB = frameCounter_ % 2 == 0;
        const std::vector descSets{resources_->GetDescriptorSet(isAtoB ? kComputeDescSetAToB : kComputeDescSetBToA)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout_, 0, descSets);

        GameOfLifePushConstants pushConstants{};
        pushConstants.time = static_cast<float>(GetCurrentTime());
        pushConstants.frameIndex = frameCounter_;

        currentCmdBuffer->PushConstants(computePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(GameOfLifePushConstants), &pushConstants);

        const auto groupCountX = CeilDiv(currentWindowWidth_, kLocalSizeX);
        const auto groupCountY = CeilDiv(currentWindowHeight_, kLocalSizeY);
        currentCmdBuffer->Dispatch(groupCountX, groupCountY, 1);

        ++frameCounter_;
    }

    // Render phase
    {
        // Change image layout from GENERAL to SHADER_READ_ONLY
        const auto screenStorageImage = resources_->GetImage(kScreenStorageImage);
        const auto barrierGeneralToShaderRead = screenStorageImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          {barrierGeneralToShaderRead});

        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);
        currentCmdBuffer->BindPipeline(quadPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipelineLayout_, 0, descSets);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
