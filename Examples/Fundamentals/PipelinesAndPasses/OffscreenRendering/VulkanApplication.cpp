/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "AppConfig.h"
#include "ApplicationData.h"
#include "TimeUtils.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationPipelinesAndPasses(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        InitInputSystem();

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(GetParamU32(AppConstants::MaxFramesInFlight));

        CreateResources();
        InitResources();

        CreateRenderPasses();
        CreatePipelines();
        CreateDefaultFramebuffers(resources_->GetImageView(GetParamStr(AppConstants::DepthImage),
                                                           GetParamStr(AppConstants::DepthImageView)));
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
    inFlightFences_[currentIndex_]->ResetFence();

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    CalculateAndSetMvp();
    RecordPresentCommandBuffers(imageIndex);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::PreUpdate()
{
    // Calculate delta time
    const double currentFrame = GetCurrentTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    // Poll events
    ApplicationPipelinesAndPasses::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::Cleanup() noexcept
{
    ApplicationPipelinesAndPasses::Cleanup();
    crateTextureHandler_.Clear();
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = static_cast<float>(currentWindowWidth_) / 2.0f;
    lastY_ = static_cast<float>(currentWindowHeight_) / 2.0f;

    window_->DisableCursor();

    window_->OnMouseMove([&](const MouseMoveEvent& event) {
        const auto xPos = static_cast<float>(event.X);
        const auto yPos = static_cast<float>(event.Y);

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

        yawAngle_ += xOffset;
        pitchAngle_ += yOffset;

        pitchAngle_ = std::clamp(pitchAngle_, -89.0f, 89.0f);

        const float yawRad = glm::radians(yawAngle_);
        const float pitchRad = glm::radians(pitchAngle_);

        const glm::vec3 front{std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad),
                              std::sin(yawRad) * std::cos(pitchRad)};
        cameraFront_ = glm::normalize(front);
    });
}

void VulkanApplication::CreateResources()
{
    depthImageFormat_ = physicalDevice_->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    crateTextureHandler_ = textureLoader.Load(GetParamStr(AppConstants::CrateTexturePath));

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const std::uint32_t cubeVertexBufSize = cubeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t cubeIndexBufSize = cubeIndices.size() * sizeof(cubeIndices[0]);
    const std::uint32_t planeVertexBufSize = planeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t planeIndexBufSize = planeIndices.size() * sizeof(planeIndices[0]);

    resourceCreateInfo.Buffers = {
        {.Name = GetParamStr(AppConstants::CubeVertexBuffer),
         .BufferSizeInBytes = cubeVertexBufSize,
         .UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         .MemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::CubeIndexBuffer), cubeIndexBufSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::PlaneVertexBuffer), planeVertexBufSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::PlaneIndexBuffer), planeIndexBufSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.GetByteSize(),
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
                                  .Modules = {{.Name = GetParamStr(AppConstants::QuadVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::QuadVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::SceneVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::SceneVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::QuadFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::QuadFragmentShaderFile)},
                                              {.Name = GetParamStr(AppConstants::SceneFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::SceneFragmentShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.Descriptors = {
        .MaxSets = 2,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}},
        .Layouts = {
            {.Name = GetParamStr(AppConstants::SceneDescSetLayout),
             .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
            {.Name = GetParamStr(AppConstants::QuadDescSetLayout),
             .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}};

    resourceCreateInfo.Images = {
        ImageResourceCreateInfo{.Name = GetParamStr(AppConstants::CrateImage),
                                .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .Format = VK_FORMAT_R8G8B8A8_SRGB,
                                .Dimensions = {crateTextureHandler_.Width, crateTextureHandler_.Height, 1},
                                .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::CrateImageView),
                                                              .Format = VK_FORMAT_R8G8B8A8_SRGB}}},
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::DepthImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = depthImageFormat_,
            .Dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::DepthImageView),
                                          .Format = depthImageFormat_,
                                          .SubresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}},
        ImageResourceCreateInfo{.Name = GetParamStr(AppConstants::OffscreenImage),
                                .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .Format = VK_FORMAT_R8G8B8A8_UNORM,
                                .Dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
                                .UsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::OffscreenImageView),
                                                              .Format = VK_FORMAT_R8G8B8A8_UNORM}}}};

    resourceCreateInfo.Samplers = {
        {.Name = GetParamStr(AppConstants::MainSampler),
         .FilteringBehavior = {.MagFilter = VK_FILTER_LINEAR, .MinFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitResources() const
{
    resources_->SetBuffer(GetParamStr(AppConstants::CubeVertexBuffer), cubeVertices.data(),
                          cubeVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::CubeIndexBuffer), cubeIndices.data(),
                          cubeIndices.size() * sizeof(cubeIndices[0]));
    resources_->SetBuffer(GetParamStr(AppConstants::PlaneVertexBuffer), planeVertices.data(),
                          planeVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::PlaneIndexBuffer), planeIndices.data(),
                          planeIndices.size() * sizeof(planeIndices[0]));
    resources_->SetBuffer(GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.Data,
                          crateTextureHandler_.GetByteSize());

    resources_->SetImageFromBuffer(cmdPool_, queue_, GetParamStr(AppConstants::CrateImage),
                                   resources_->GetBuffer(GetParamStr(AppConstants::ImageStagingBuffer)),
                                   {crateTextureHandler_.Width, crateTextureHandler_.Height, 1});

    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPasses()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

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
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat_;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
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

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }

    offscreenRP_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               })
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat_;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
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

    scenePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::SceneDescSetLayout))}, {mvpPushConstant});

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

    scenePipeline_ = device_->CreateGraphicsPipeline(scenePipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneFragmentShaderKey))->GetHandle();
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
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!scenePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (scene)!");
    }

    quadPipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::QuadDescSetLayout))});

    if (!quadPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (quad)!");
    }

    quadPipeline_ = device_->CreateGraphicsPipeline(quadPipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::QuadVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::QuadFragmentShaderKey))->GetHandle();
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
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneFragmentShaderKey))->GetHandle();
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
    const auto& offscreenImageView = resources_->GetImageView(GetParamStr(AppConstants::OffscreenImage),
                                                              GetParamStr(AppConstants::OffscreenImageView));
    const auto& depthImageView =
            resources_->GetImageView(GetParamStr(AppConstants::DepthImage), GetParamStr(AppConstants::DepthImageView));
    offscreenFramebuffer_ =
            device_->CreateFramebuffer(offscreenRP_, {offscreenImageView, depthImageView}, [&](auto& builder) {
                builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
            });

    if (!offscreenFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (offscreen)!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> sceneImageInfos;
    sceneImageInfos.emplace_back(
            resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
            resources_->GetImageView(GetParamStr(AppConstants::CrateImage), GetParamStr(AppConstants::CrateImageView))
                    ->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> quadImageInfos;
    quadImageInfos.emplace_back(resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
                                resources_
                                        ->GetImageView(GetParamStr(AppConstants::OffscreenImage),
                                                       GetParamStr(AppConstants::OffscreenImageView))
                                        ->GetHandle(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest sceneImageUpdateRequest;
    sceneImageUpdateRequest.LayoutName = GetParamStr(AppConstants::SceneDescSetLayout);
    sceneImageUpdateRequest.BindingIndex = 0;
    sceneImageUpdateRequest.Images = sceneImageInfos;
    sceneImageUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest quadImageUpdateRequest;
    quadImageUpdateRequest.LayoutName = GetParamStr(AppConstants::QuadDescSetLayout);
    quadImageUpdateRequest.BindingIndex = 0;
    quadImageUpdateRequest.Images = quadImageInfos;
    quadImageUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .ImageWriteRequests = {sceneImageUpdateRequest, quadImageUpdateRequest}};

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

    if (!cmdBuffersPresent_[currentImageIndex]->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Offscreen render pass
    {
        cmdBuffersPresent_[currentImageIndex]->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = offscreenRP_->GetHandle();
                    beginInfo.framebuffer = offscreenFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        cmdBuffersPresent_[currentImageIndex]->BindPipeline(offscreenPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        RenderScene(cmdBuffersPresent_[currentImageIndex]);

        cmdBuffersPresent_[currentImageIndex]->EndRenderPass();
    }

    // Swapchain render pass
    {
        cmdBuffersPresent_[currentImageIndex]->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        cmdBuffersPresent_[currentImageIndex]->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        RenderScene(cmdBuffersPresent_[currentImageIndex]);

        cmdBuffersPresent_[currentImageIndex]->BindPipeline(quadPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        RenderQuad(cmdBuffersPresent_[currentImageIndex]);

        cmdBuffersPresent_[currentImageIndex]->EndRenderPass();
    }

    if (!cmdBuffersPresent_[currentImageIndex]->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    for (size_t i = 0; i < NUM_CUBES; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, modelPositions[i]);

        const glm::mat4 view = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);

        const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), // FOV
                                          aspectRatio,         // Aspect ratio
                                          0.1f,                // Near clipping-plane
                                          20.0f                // Far clipping plane
        );
        proj[1][1] *= -1;                                      // Vulkan trick for projection


        // Calculate MVP matrix
        mvpData[i].mvpMatrix = proj * view * model;
    }
}

void VulkanApplication::ProcessInput()
{
    const float cameraSpeed = GetParamFloat(AppSettings::CameraSpeed) * static_cast<float>(deltaTime_);
    if (window_->IsKeyPressed(GLFW_KEY_W)) {
        cameraPos_ += cameraSpeed * cameraFront_;
    }
    if (window_->IsKeyPressed(GLFW_KEY_S)) {
        cameraPos_ -= cameraSpeed * cameraFront_;
    }
    if (window_->IsKeyPressed(GLFW_KEY_A)) {
        cameraPos_ -= glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
    if (window_->IsKeyPressed(GLFW_KEY_D)) {
        cameraPos_ += glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
}

void VulkanApplication::RenderScene(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer)
{
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipelineLayout_, 0,
                                  {resources_->GetDescriptorSet(GetParamStr(AppConstants::SceneDescSetLayout))});
    cmdBuffer->BindVertexBuffers({resources_->GetBuffer(GetParamStr(AppConstants::CubeVertexBuffer))}, 0, 1, {0});
    cmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::CubeIndexBuffer)), 0,
                               VK_INDEX_TYPE_UINT16);

    for (auto& mvp: mvpData) {
        cmdBuffer->PushConstants(scenePipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MvpData), &mvp);
        cmdBuffer->DrawIndexed(cubeIndices.size(), 1, 0, 0, 0);
    }
}

void VulkanApplication::RenderQuad(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipelineLayout_, 0,
                                  {resources_->GetDescriptorSet(GetParamStr(AppConstants::QuadDescSetLayout))});
    cmdBuffer->BindVertexBuffers({resources_->GetBuffer(GetParamStr(AppConstants::PlaneVertexBuffer))}, 0, 1, {0});
    cmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::PlaneIndexBuffer)), 0,
                               VK_INDEX_TYPE_UINT16);

    cmdBuffer->DrawIndexed(planeIndices.size(), 1, 0, 0, 0);
}
} // namespace examples::fundamentals::pipelines_and_passes::offscreen_rendering
