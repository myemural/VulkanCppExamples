/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::multisampling::explicit_resolving
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationMultisampling(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        camera_ = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 4.0f), aspectRatio);

        InitInputSystem();

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

        CreateRenderPasses();
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
    inFlightFences_[currentIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    CalculateAndSetMvp();
    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationMultisampling::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = static_cast<float>(currentWindowWidth_) / 2.0f;
    lastY_ = static_cast<float>(currentWindowHeight_) / 2.0f;

    window_->DisableCursor();

    window_->OnMouseMove([&](const MouseMoveEvent& event) {
        const auto xPos = static_cast<float>(event.x);
        const auto yPos = static_cast<float>(event.y);

        if (firstMouseTriggered_) {
            lastX_ = xPos;
            lastY_ = yPos;
            firstMouseTriggered_ = false;
        }

        float xOffset = xPos - lastX_;
        float yOffset = lastY_ - yPos;
        lastX_ = xPos;
        lastY_ = yPos;

        const float sensitivity = GetParamFloat(AppSettings::MouseSensitivity) * static_cast<float>(deltaTime_);
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        camera_->Rotate(xOffset, yOffset);
    });
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateResources()
{
    depthImageFormat_ = physicalDevice_->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    maxSampleCount_ = physicalDevice_->GetMaxUsableSampleCount();

    // Pre-load textures
    const auto marbleTextureAssetHandler = assetManager_->Load<TextureAsset>(kMarbleTexturePath);
    marbleTextureAsset_ = assetManager_->Get(marbleTextureAssetHandler);

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const std::uint32_t sphereVertexBufSize = sphereVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t sphereIndexBufSize = sphereIndices.size() * sizeof(sphereIndices[0]);
    const std::uint32_t planeVertexBufSize = planeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t planeIndexBufSize = planeIndices.size() * sizeof(planeIndices[0]);

    resourceCreateInfo.buffers = {
        {.name = kSphereVertexBuffer,
         .bufferSizeInBytes = sphereVertexBufSize,
         .usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kSphereIndexBuffer, sphereIndexBufSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kPlaneVertexBuffer, planeVertexBufSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kPlaneIndexBuffer, planeIndexBufSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto quadVertexShaderAsset = assetManager_->Load<ShaderAsset>(kQuadVertexShaderFile);
    const auto sceneVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSceneVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kQuadVertexShaderKey, .asset = assetManager_->Get(quadVertexShaderAsset)},
                    {.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.descriptors = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}},
        .layouts = {{.name = kSceneDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kQuadDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kSceneDescSetLayout, .layoutName = kSceneDescSetLayout},
                           {.name = kQuadDescSetLayout, .layoutName = kQuadDescSetLayout}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kMarbleImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .dimensions = {marbleTextureAsset_.width, marbleTextureAsset_.height, 1},
            .views = {ImageViewCreateInfo{.viewName = kMarbleImageView, .format = VK_FORMAT_R8G8B8A8_SRGB}}},
        ImageResourceCreateInfo{
            .name = kMultisampledImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .samples = maxSampleCount_,
            .usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .views = {ImageViewCreateInfo{.viewName = kMultisampledImageView,
                                          .format = VK_FORMAT_B8G8R8A8_SRGB,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}},

        ImageResourceCreateInfo{
            .name = kResolvedImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kResolvedImageView,
                                          .format = VK_FORMAT_B8G8R8A8_SRGB,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}}};

    resourceCreateInfo.images->emplace_back(ImageResourceCreateInfo{
        .name = kDepthImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = depthImageFormat_,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .samples = maxSampleCount_,
        .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .views = {ImageViewCreateInfo{.viewName = kDepthImageView,
                                      .format = depthImageFormat_,
                                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1}}}});

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitResources() const
{
    resources_->SetBuffer(kSphereVertexBuffer, sphereVertices.data(), sphereVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(kSphereIndexBuffer, sphereIndices.data(), sphereIndices.size() * sizeof(sphereIndices[0]));
    resources_->SetBuffer(kPlaneVertexBuffer, planeVertices.data(), planeVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(kPlaneIndexBuffer, planeIndices.data(), planeIndices.size() * sizeof(planeIndices[0]));

    resources_->SetImageFromTexture(cmdPool_, queue_, kMarbleImage, marbleTextureAsset_);

    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPasses()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([&](auto& attachmentCreateInfo) {
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

    offscreenRP_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([&](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                   attachmentCreateInfo.samples = maxSampleCount_;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
               })
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat_;
                    attachmentCreateInfo.samples = maxSampleCount_;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                    subpassCreateInfo.pDepthStencilAttachment = &depthAttachmentRef;
                });
    });

    if (!offscreenRP_) {
        throw std::runtime_error("Failed to create render pass (offscreen)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MvpData);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    scenePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kSceneDescSetLayout)}, {mvpPushConstant});

    if (!scenePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (scene)!");
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

    quadPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kQuadDescSetLayout)});

    if (!quadPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (quad)!");
    }

    quadPipeline_ = device_->CreateGraphicsPipeline(quadPipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kQuadVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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

    if (!quadPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (quad)!");
    }

    offscreenPipeline_ = device_->CreateGraphicsPipeline(scenePipelineLayout_, offscreenRP_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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
        builder.SetMultisampleState([&](auto& multisampleStateCreateInfo) {
            multisampleStateCreateInfo.rasterizationSamples = maxSampleCount_;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!offscreenPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (offscreen)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(
                renderPass_, {swapImage}, [&](auto& builder) { builder.SetDimensions(windowWidth, windowHeight); });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }

    const auto currentMultisampledImageView = resources_->GetImageView(kMultisampledImage, kMultisampledImageView);

    const auto currentDepthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);

    offscreenFramebuffer_ = device_->CreateFramebuffer(
            offscreenRP_, {currentMultisampledImageView, currentDepthImageView},
            [&](auto& builder) { builder.SetDimensions(currentWindowWidth_, currentWindowHeight_); });

    if (!offscreenFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (offscreen)!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> sceneImageInfos;
    sceneImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                 resources_->GetImageView(kMarbleImage, kMarbleImageView)->GetHandle(),
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> quadImageInfos;
    quadImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                resources_->GetImageView(kResolvedImage, kResolvedImageView)->GetHandle(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest sceneImageUpdateRequest;
    sceneImageUpdateRequest.descriptorSetName = kSceneDescSetLayout;
    sceneImageUpdateRequest.bindingIndex = 0;
    sceneImageUpdateRequest.images = sceneImageInfos;
    sceneImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest quadImageUpdateRequest;
    quadImageUpdateRequest.descriptorSetName = kQuadDescSetLayout;
    quadImageUpdateRequest.bindingIndex = 0;
    quadImageUpdateRequest.images = quadImageInfos;
    quadImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .imageWriteRequests = {sceneImageUpdateRequest, quadImageUpdateRequest}};

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
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    clearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Offscreen render pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = offscreenRP_->GetHandle();
                    beginInfo.framebuffer = offscreenFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        currentCmdBuffer->BindPipeline(offscreenPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        RenderScene(currentCmdBuffer);

        currentCmdBuffer->EndRenderPass();
    }

    // Manual resolving multisampled image
    const auto multisampledImage = resources_->GetImage(kMultisampledImage);
    const auto resolvedImage = resources_->GetImage(kResolvedImage);

    const auto multisampledBarrierSrc = multisampledImage->CreateImageMemoryBarrier(
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    const auto resolvedBarrierDst = resolvedImage->CreateImageMemoryBarrier(
            0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      {multisampledBarrierSrc, resolvedBarrierDst});

    VkImageResolve region{};
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource = region.srcSubresource;
    region.extent = {currentWindowWidth_, currentWindowHeight_, 1};
    currentCmdBuffer->ResolveImage(multisampledImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, resolvedImage,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {region});

    const auto resolvedBarrierShaderRead = resolvedImage->CreateImageMemoryBarrier(
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      {resolvedBarrierShaderRead});

    // Swapchain render pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        currentCmdBuffer->BindPipeline(quadPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        RenderQuad(currentCmdBuffer);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    for (size_t i = 0; i < NUM_SPHERES; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, modelPositions[i]);

        const glm::mat4 view = camera_->GetViewMatrix();
        glm::mat4 proj = camera_->GetProjectionMatrix();

        // Calculate MVP matrix
        mvpData_[i].mvpMatrix = proj * view * model;
    }
}

void VulkanApplication::ProcessInput() const
{
    const float cameraSpeed = GetParamFloat(AppSettings::CameraSpeed) * static_cast<float>(deltaTime_);
    if (window_->IsKeyPressed(GLFW_KEY_W)) {
        camera_->Move(camera_->GetFrontVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_S)) {
        camera_->Move(-camera_->GetFrontVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_A)) {
        camera_->Move(-camera_->GetRightVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_D)) {
        camera_->Move(camera_->GetRightVector() * cameraSpeed);
    }
}

void VulkanApplication::RenderScene(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    const std::vector descSets{resources_->GetDescriptorSet(kSceneDescSetLayout)};
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipelineLayout_, 0, descSets);
    cmdBuffer->BindVertexBuffers({resources_->GetBuffer(kSphereVertexBuffer)}, 0, 1, {0});
    cmdBuffer->BindIndexBuffer(resources_->GetBuffer(kSphereIndexBuffer));

    for (auto& mvp: mvpData_) {
        cmdBuffer->PushConstants(scenePipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MvpData), &mvp);
        cmdBuffer->DrawIndexed(sphereIndices.size(), 1, 0, 0, 0);
    }
}

void VulkanApplication::RenderQuad(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    const std::vector descSets{resources_->GetDescriptorSet(kQuadDescSetLayout)};
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipelineLayout_, 0, descSets);
    cmdBuffer->BindVertexBuffers({resources_->GetBuffer(kPlaneVertexBuffer)}, 0, 1, {0});
    cmdBuffer->BindIndexBuffer(resources_->GetBuffer(kPlaneIndexBuffer));
    cmdBuffer->DrawIndexed(planeIndices.size(), 1, 0, 0, 0);
}
} // namespace examples::fundamentals::multisampling::explicit_resolving
