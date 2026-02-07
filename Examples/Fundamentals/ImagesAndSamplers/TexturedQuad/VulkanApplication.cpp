/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanHelpers.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::images_and_samplers::textured_quad
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationImagesAndSamplers(std::move(params)) {}

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

        InitAssetManager();
        CreateResources();
        InitResources();

        CreateDefaultRenderPass();
        CreatePipeline();
        CreateDefaultFramebuffers();

        const std::uint32_t indexCount = indices.size();
        CreateCommandBuffers();
        RecordPresentCommandBuffers(indexCount);
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

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateResources()
{
    // Pre-load textures
    const auto bricksTextureAssetHandler = assetManager_->Load<TextureAsset>(kBricksTexturePath);
    bricksTextureAsset_ = assetManager_->Get(bricksTextureAssetHandler);

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);
    const std::vector<BufferResourceCreateInfo> bufferCreateInfos = {
        {kMainVertexBuffer, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMainIndexBuffer, indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kImageStagingBuffer, static_cast<std::uint32_t>(bricksTextureAsset_.data.size()),
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
    CreateBuffers(bufferCreateInfos);

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    const ShaderModulesCreateInfo shaderModuleCreateInfo = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};
    CreateShaderModules(shaderModuleCreateInfo);

    // Fill descriptor set create infos
    const DescriptorResourceCreateInfo descriptorSetCreateInfo = {
        .maxSets = 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings =
                             {
                                 {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                 {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                             }}},
        .descriptorSets = {{.name = kMainDescSetLayout, .layoutName = kMainDescSetLayout}}};
    CreateDescriptorSets(descriptorSetCreateInfo);

    CreateQuadTextureImage();
    CreateQuadTextureImageView();
    CreateSampler();
}

void VulkanApplication::InitResources()
{
    SetBuffer(kMainVertexBuffer, vertices.data(), vertices.size() * sizeof(VertexPos2Uv2));
    SetBuffer(kMainIndexBuffer, indices.data(), indices.size() * sizeof(indices[0]));
    SetBuffer(kImageStagingBuffer, bricksTextureAsset_.data.data(), bricksTextureAsset_.data.size());

    UpdateDescriptorSets();

    ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyStagingBuffer();
    ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanApplication::CreatePipeline()
{
    pipelineLayout_ = device_->CreatePipelineLayout({descriptorRegistry_->GetDescriptorLayout(kMainDescSetLayout)});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
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
    auto bindingDescription = GenerateBindingDescription<VertexPos2Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> samplerInfos;
    samplerInfos.emplace_back(sampler_->GetHandle());

    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.emplace_back(VK_NULL_HANDLE, quadTextureImageView_->GetHandle(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.descriptorSetName = kMainDescSetLayout;
    samplerUpdateRequest.bindingIndex = 0;
    samplerUpdateRequest.images = samplerInfos;
    samplerUpdateRequest.type = VK_DESCRIPTOR_TYPE_SAMPLER;

    ImageWriteRequest sampledImageUpdateRequest;
    sampledImageUpdateRequest.descriptorSetName = kMainDescSetLayout;
    sampledImageUpdateRequest.bindingIndex = 1;
    sampledImageUpdateRequest.images = imageInfos;
    sampledImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .imageWriteRequests = {samplerUpdateRequest, sampledImageUpdateRequest}};

    UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::CreateQuadTextureImage()
{
    quadTextureImage_ = device_->CreateImage([&](auto& builder) {
        builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
        builder.SetDimensions(bricksTextureAsset_.width, bricksTextureAsset_.height);
    });

    if (!quadTextureImage_) {
        throw std::runtime_error("Failed to create texture image!");
    }

    const auto memoryReq = quadTextureImage_->GetImageMemoryRequirements();

    const uint32_t memoryTypeIndex =
            physicalDevice_->FindMemoryType(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureDeviceMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!textureDeviceMemory_) {
        throw std::runtime_error("Failed to allocate texture device memory!");
    }

    quadTextureImage_->BindImageMemory(textureDeviceMemory_, 0);
}

void VulkanApplication::CreateQuadTextureImageView()
{
    quadTextureImageView_ = device_->CreateImageView(quadTextureImage_,
                                                     [](auto& builder) { builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB); });

    if (!quadTextureImageView_) {
        throw std::runtime_error("Failed to create texture image view!");
    }
}

void VulkanApplication::CreateSampler()
{
    sampler_ = device_->CreateSampler([](auto& builder) { builder.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR); });

    if (!sampler_) {
        throw std::runtime_error("Failed to create sampler!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t indexCount)
{
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkClearValue clearColor;
        clearColor.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[i]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &clearColor;
                },
                VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                                                  {descriptorRegistry_->GetDescriptorSet(kMainDescSetLayout)});
        cmdBuffersPresent_[i]->BindVertexBuffers({buffers_[kMainVertexBuffer]->GetBuffer()}, 0, 1, {0});
        cmdBuffersPresent_[i]->BindIndexBuffer(buffers_[kMainIndexBuffer]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
        cmdBuffersPresent_[i]->DrawIndexed(indexCount, 1, 0, 0, 0);
        cmdBuffersPresent_[i]->EndRenderPass();
        if (!cmdBuffersPresent_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::CopyStagingBuffer()
{
    const auto cmdBufferTransfer = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferTransfer->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        .imageOffset = {0, 0, 0},
        .imageExtent = {bricksTextureAsset_.width, bricksTextureAsset_.height, 1},
    };
    cmdBufferTransfer->CopyBufferToImage(buffers_[kImageStagingBuffer]->GetBuffer(), quadTextureImage_,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copyRegion});

    if (!cmdBufferTransfer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue_->Submit({cmdBufferTransfer});
    queue_->WaitIdle();
}
} // namespace examples::fundamentals::images_and_samplers::textured_quad
