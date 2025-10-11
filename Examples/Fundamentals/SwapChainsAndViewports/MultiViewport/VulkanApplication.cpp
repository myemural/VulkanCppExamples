/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <glm/ext/matrix_transform.hpp>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::swap_chains_and_viewports::multi_viewport
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationSwapChainsAndViewports(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        camera = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 4.0f), aspectRatio);

        InitInputSystem();

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(swapChainImageViews_.size(), GetParamU32(AppConstants::MaxFramesInFlight));

        CreateResources();
        InitResources();

        CreateRenderPass();
        CreatePipeline();
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
    // Poll events
    ApplicationSwapChainsAndViewports::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::Cleanup() noexcept
{
    ApplicationSwapChainsAndViewports::Cleanup();
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

        camera->Rotate(xOffset, yOffset);
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
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);

    resourceCreateInfo.Buffers = {
        {GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::MainIndexBuffer), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.GetByteSize(),
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
                                  .Modules = {{.Name = GetParamStr(AppConstants::MainVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::MainFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainFragmentShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.Descriptors = {.MaxSets = 1,
                                      .PoolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
                                      .Layouts = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                                                   .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                                 VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}};

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
                                                               .layerCount = 1}}}}};

    resourceCreateInfo.Samplers = {
        {.Name = GetParamStr(AppConstants::MainSampler),
         .FilteringBehavior = {.MagFilter = VK_FILTER_LINEAR, .MinFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitResources() const
{
    resources_->SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), vertices.data(),
                          vertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::MainIndexBuffer), indices.data(),
                          indices.size() * sizeof(indices[0]));
    resources_->SetBuffer(GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.Data,
                          crateTextureHandler_.GetByteSize());

    resources_->SetImageFromBuffer(cmdPool_, queue_, GetParamStr(AppConstants::CrateImage),
                                   resources_->GetBuffer(GetParamStr(AppConstants::ImageStagingBuffer)),
                                   {crateTextureHandler_.Width, crateTextureHandler_.Height, 1});

    UpdateDescriptorSets();
}

void VulkanApplication::CreateSwapChain()
{
    auto surfaceFormat = physicalDevice_->GetSurfaceFormat(surface_->GetHandle(), VK_FORMAT_B8G8R8A8_SRGB,
                                                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    auto surfaceCapabilities = physicalDevice_->GetSurfaceCapabilities(surface_->GetHandle());

    if (!surfaceFormat.has_value() || !surfaceCapabilities.has_value()) {
        throw std::runtime_error("Failed to get surface format or capabilities!");
    }

    swapChain_ = device_->CreateSwapChain(surface_, [&](auto& builder) {
        builder.SetMinImageCount(surfaceCapabilities.value().minImageCount + 1)
                .SetImageFormat(surfaceFormat->format)
                .SetImageColorSpace(surfaceFormat->colorSpace)
                .SetImageExtent(currentWindowWidth_, currentWindowHeight_)
                .SetPreTransformFlagBits(surfaceCapabilities.value().currentTransform);
    });

    if (!swapChain_) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    swapChainImageViews_ = swapChain_->GetSwapChainImageViews();

    if (swapChainImageViews_.empty()) {
        throw std::runtime_error("Failed to get swap chain image views!");
    }
}

void VulkanApplication::CreateRenderPass()
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
}

void VulkanApplication::CreatePipeline()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MvpData);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))}, {mvpPushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    const std::uint32_t halfWidth = currentWindowWidth_ / 2;
    const std::uint32_t halfHeight = currentWindowHeight_ / 2;
    const auto halfWidthFloat = static_cast<float>(halfWidth);
    const auto halfHeightFloat = static_cast<float>(halfHeight);

    std::array viewports = {
        VkViewport{0, 0, halfWidthFloat, halfHeightFloat, 0.0f, 1.0f},                           // Top-left
        VkViewport{halfWidthFloat, 0, halfWidthFloat, halfHeightFloat, 0.0f, 1.0f},              // Top-right
        VkViewport{0, halfHeightFloat, halfWidthFloat, halfHeightFloat, 0.0f, 1.0f},             // Bottom-left
        VkViewport{halfWidthFloat, halfHeightFloat, halfWidthFloat, halfHeightFloat, 0.0f, 1.0f} // Bottom-right
    };

    scissorRects_ = {
        VkRect2D{0, 0, halfWidth, halfHeight},                                // Top-left
        VkRect2D{static_cast<int32_t>(halfWidth), 0, halfWidth, halfHeight},  // Top-right
        VkRect2D{0, static_cast<int32_t>(halfHeight), halfWidth, halfHeight}, // Bottom-left
        VkRect2D{static_cast<int32_t>(halfWidth), static_cast<int32_t>(halfHeight), halfWidth,
                 halfHeight}                                                  // Bottom-right
    };

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

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
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
            viewportStateCreateInfo.viewportCount = viewports.size();
            viewportStateCreateInfo.pViewports = viewports.data();
            viewportStateCreateInfo.scissorCount = scissorRects_.size();
            viewportStateCreateInfo.pScissors = scissorRects_.data();
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

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    const auto& depthImageView =
            resources_->GetImageView(GetParamStr(AppConstants::DepthImage), GetParamStr(AppConstants::DepthImageView));

    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage, depthImageView}, [&](auto& builder) {
            builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> imageSamplerInfos;
    imageSamplerInfos.emplace_back(
            resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
            resources_->GetImageView(GetParamStr(AppConstants::CrateImage), GetParamStr(AppConstants::CrateImageView))
                    ->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.LayoutName = GetParamStr(AppConstants::MainDescSetLayout);
    samplerUpdateRequest.BindingIndex = 0;
    samplerUpdateRequest.Images = imageSamplerInfos;
    samplerUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {.ImageWriteRequests = {samplerUpdateRequest}};

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
    VkClearAttachment clearAttachment = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .colorAttachment = 0,
    };

    constexpr std::array clearColors = {
        VkClearColorValue{1.0f, 0.0f, 0.0f, 1.0f}, VkClearColorValue{0.0f, 1.0f, 0.0f, 1.0f},
        VkClearColorValue{0.0f, 0.0f, 1.0f, 1.0f}, VkClearColorValue{1.0f, 1.0f, 0.0f, 1.0f}};

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
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

    currentCmdBuffer->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::MainDescSetLayout))};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);
    const std::vector vertexBuffers{resources_->GetBuffer(GetParamStr(AppConstants::MainVertexBuffer))};
    currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});
    currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::MainIndexBuffer)));

    for (const auto& region: regions_) {
        const int viewportIndex = static_cast<int>(region);

        // Clear viewport with specified color
        VkClearRect clearRect = {.rect = scissorRects_[viewportIndex], .baseArrayLayer = 0, .layerCount = 1};
        clearAttachment.clearValue.color = clearColors[viewportIndex];
        currentCmdBuffer->ClearAttachments({clearAttachment}, {clearRect});

        // Draw to viewport
        for (auto& mvp: mvpData_) {
            mvp.viewportIndex = viewportIndex;
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MvpData), &mvp);
            currentCmdBuffer->DrawIndexed(indices.size(), 1, 0, 0, 0);
        }
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    for (size_t i = 0; i < NUM_CUBES; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, modelPositions[i]);

        const glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 proj = camera->GetProjectionMatrix();

        // Calculate MVP matrix
        mvpData_[i].mvpMatrix = proj * view * model;
    }
}

void VulkanApplication::ProcessInput() const
{
    const float cameraSpeed = GetParamFloat(AppSettings::CameraSpeed) * static_cast<float>(deltaTime_);
    if (window_->IsKeyPressed(GLFW_KEY_W)) {
        camera->Move(camera->GetFrontVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_S)) {
        camera->Move(-camera->GetFrontVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_A)) {
        camera->Move(-camera->GetRightVector() * cameraSpeed);
    }
    if (window_->IsKeyPressed(GLFW_KEY_D)) {
        camera->Move(camera->GetRightVector() * cameraSpeed);
    }
}

} // namespace examples::fundamentals::swap_chains_and_viewports::multi_viewport
