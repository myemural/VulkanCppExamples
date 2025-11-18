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

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "MathUtils.h"
#include "TextureLoader.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::compute_shaders::basic_particles
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

        float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        camera_ = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 4.0f), aspectRatio);

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

        CreateRenderPass();
        CreatePipelines();
        CreateDefaultFramebuffers(resources_->GetImageView(GetParamStr(AppConstants::DepthImage),
                                                           GetParamStr(AppConstants::DepthImageView)));

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
    ApplicationComputeShaders::PreUpdate();

    // Process continuous inputs
    ProcessInput();
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

        camera_->Rotate(xOffset, yOffset);
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
    const std::uint32_t cubeVertexSize = cubeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t cubeIndexSize = cubeIndices.size() * sizeof(cubeIndices[0]);
    const uint32_t particleBufferSize = GetParamU32(AppSettings::ParticleCount) * sizeof(ParticleData);

    resourceCreateInfo.Buffers = {
        {GetParamStr(AppConstants::CubeVertexBuffer), cubeVertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::CubeIndexBuffer), cubeIndexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::ParticleStorageBuffer), particleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
                                  .Modules = {{.Name = GetParamStr(AppConstants::MainVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::MainFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainFragmentShaderFile)},
                                              {.Name = GetParamStr(AppConstants::ParticleVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::ParticleVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::ParticleFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::ParticleFragmentShaderFile)},
                                              {.Name = GetParamStr(AppConstants::ParticleComputeShaderKey),
                                               .FileName = GetParamStr(AppConstants::ParticleComputeShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.Descriptors = {
        .MaxSets = 3,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}},
        .Layouts = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.Name = GetParamStr(AppConstants::ParticleDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}},
                    {.Name = GetParamStr(AppConstants::ParticleComputeDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .DescriptorSets = {{.Name = GetParamStr(AppConstants::CubeDescSet),
                            .LayoutName = GetParamStr(AppConstants::MainDescSetLayout)},
                           {.Name = GetParamStr(AppConstants::ParticleDescSetLayout),
                            .LayoutName = GetParamStr(AppConstants::ParticleDescSetLayout)},
                           {.Name = GetParamStr(AppConstants::ParticleComputeDescSetLayout),
                            .LayoutName = GetParamStr(AppConstants::ParticleComputeDescSetLayout)}}};

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
    resources_->SetBuffer(GetParamStr(AppConstants::CubeVertexBuffer), cubeVertices.data(),
                          cubeVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::CubeIndexBuffer), cubeIndices.data(),
                          cubeIndices.size() * sizeof(cubeIndices[0]));

    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::CrateImage), crateTextureHandler_);

    UpdateDescriptorSets();
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

void VulkanApplication::CreatePipelines()
{
    // Compute pipeline (for particles)
    {
        VkPushConstantRange particlePushConstant;
        particlePushConstant.offset = 0;
        particlePushConstant.size = sizeof(ParticlePushConstant);
        particlePushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        particleComputePipelineLayout_ = device_->CreatePipelineLayout(
                {resources_->GetDescriptorLayout(GetParamStr(AppConstants::ParticleComputeDescSetLayout))},
                {particlePushConstant});

        if (!particleComputePipelineLayout_) {
            throw std::runtime_error("Failed to create compute pipeline layout!");
        }

        particleComputePipeline_ = device_->CreateComputePipeline(particleComputePipelineLayout_, [&](auto& builder) {
            builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                shaderStageCreateInfo.module =
                        resources_->GetShaderModule(GetParamStr(AppConstants::ParticleComputeShaderKey))->GetHandle();
            });
        });

        if (!particleComputePipeline_) {
            throw std::runtime_error("Failed to create compute pipeline!");
        }
    }

    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MvpData);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

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

    // Graphics pipeline (for particles)
    {
        particleGraphicsPipelineLayout_ = device_->CreatePipelineLayout(
                {resources_->GetDescriptorLayout(GetParamStr(AppConstants::ParticleDescSetLayout))}, {mvpPushConstant});

        if (!particleGraphicsPipelineLayout_) {
            throw std::runtime_error("Failed to create graphics pipeline layout (for particles)!");
        }

        particleGraphicsPipeline_ =
                device_->CreateGraphicsPipeline(particleGraphicsPipelineLayout_, renderPass_, [&](auto& builder) {
                    builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                        shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                        shaderStageCreateInfo.module =
                                resources_->GetShaderModule(GetParamStr(AppConstants::ParticleVertexShaderKey))
                                        ->GetHandle();
                    });
                    builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                        shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                        shaderStageCreateInfo.module =
                                resources_->GetShaderModule(GetParamStr(AppConstants::ParticleFragmentShaderKey))
                                        ->GetHandle();
                    });
                    builder.SetInputAssemblyState([&](auto& assemblyStateCreateInfo) {
                        assemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                    });
                    builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                        viewportStateCreateInfo.viewportCount = 1;
                        viewportStateCreateInfo.pViewports = &viewport;
                        viewportStateCreateInfo.scissorCount = 1;
                        viewportStateCreateInfo.pScissors = &scissor;
                    });
                    builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
                        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_POINT;
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

        if (!particleGraphicsPipeline_) {
            throw std::runtime_error("Failed to create graphics pipeline (for particles)!");
        }
    }

    // Graphics pipeline (for cubes)
    {
        mainGraphicsPipelineLayout_ = device_->CreatePipelineLayout(
                {resources_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))}, {mvpPushConstant});

        if (!mainGraphicsPipelineLayout_) {
            throw std::runtime_error("Failed to create graphics pipeline layout (for cubes)!");
        }

        constexpr uint32_t bindingIndex = 0;
        auto bindingDescription = GenerateBindingDescription<VertexPos3Uv2>(bindingIndex);
        const auto posAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Position, bindingIndex);
        const auto uvAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Uv, bindingIndex);
        const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

        mainGraphicsPipeline_ =
                device_->CreateGraphicsPipeline(mainGraphicsPipelineLayout_, renderPass_, [&](auto& builder) {
                    builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                        shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                        shaderStageCreateInfo.module =
                                resources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))
                                        ->GetHandle();
                    });
                    builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                        shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                        shaderStageCreateInfo.module =
                                resources_->GetShaderModule(GetParamStr(AppConstants::MainFragmentShaderKey))
                                        ->GetHandle();
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

        if (!mainGraphicsPipeline_) {
            throw std::runtime_error("Failed to create graphics pipeline (for cubes)!");
        }
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> imageSamplerInfosCube;
    imageSamplerInfosCube.emplace_back(
            resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
            resources_->GetImageView(GetParamStr(AppConstants::CrateImage), GetParamStr(AppConstants::CrateImageView))
                    ->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorBufferInfo> bufferStorageInfos;
    bufferStorageInfos.emplace_back(
            resources_->GetBuffer(GetParamStr(AppConstants::ParticleStorageBuffer))->GetHandle(), 0, VK_WHOLE_SIZE);

    ImageWriteRequest samplerUpdateRequestCube;
    samplerUpdateRequestCube.DescriptorSetName = GetParamStr(AppConstants::CubeDescSet);
    samplerUpdateRequestCube.BindingIndex = 0;
    samplerUpdateRequestCube.Images = imageSamplerInfosCube;
    samplerUpdateRequestCube.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest storageBufferUpdateRequestCube;
    storageBufferUpdateRequestCube.DescriptorSetName = GetParamStr(AppConstants::ParticleDescSetLayout);
    storageBufferUpdateRequestCube.BindingIndex = 0;
    storageBufferUpdateRequestCube.Buffers = bufferStorageInfos;
    storageBufferUpdateRequestCube.Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest storageBufferUpdateRequestParticle;
    storageBufferUpdateRequestParticle.DescriptorSetName = GetParamStr(AppConstants::ParticleComputeDescSetLayout);
    storageBufferUpdateRequestParticle.BindingIndex = 0;
    storageBufferUpdateRequestParticle.Buffers = bufferStorageInfos;
    storageBufferUpdateRequestParticle.Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .BufferWriteRequests = {storageBufferUpdateRequestParticle, storageBufferUpdateRequestCube},
        .ImageWriteRequests = {samplerUpdateRequestCube}};

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

    // Generate particles (compute phase)
    {
        currentCmdBuffer->BindPipeline(particleComputePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{
            resources_->GetDescriptorSet(GetParamStr(AppConstants::ParticleComputeDescSetLayout))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, particleComputePipelineLayout_, 0,
                                             descSets);
        ParticlePushConstant particlePushConstant{};
        particlePushConstant.deltaTime = static_cast<float>(deltaTime_);
        particlePushConstant.particleCount = static_cast<int>(GetParamU32(AppSettings::ParticleCount));
        currentCmdBuffer->PushConstants(particleComputePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(ParticlePushConstant), &particlePushConstant);
        const auto groupCountX = CeilDiv(GetParamU32(AppSettings::ParticleCount), LOCAL_SIZE_X);
        currentCmdBuffer->Dispatch(groupCountX, 1, 1);
    }

    // Render phase
    {
        // Before start render pass, put memory barrier for particle storage buffer
        const auto particleStorageBuffer = resources_->GetBuffer(GetParamStr(AppConstants::ParticleStorageBuffer));
        const auto barrier =
                particleStorageBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, {},
                                          {barrier});

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

        // Draw cubes
        for (const auto mvp: mvpData_) {
            currentCmdBuffer->BindPipeline(mainGraphicsPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
            const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::CubeDescSet))};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, mainGraphicsPipelineLayout_, 0,
                                                 descSets);
            const std::vector vertexBuffers{resources_->GetBuffer(GetParamStr(AppConstants::CubeVertexBuffer))};
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});
            currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::CubeIndexBuffer)));
            currentCmdBuffer->PushConstants(mainGraphicsPipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MvpData),
                                            &mvp);
            currentCmdBuffer->DrawIndexed(cubeIndices.size(), 1, 0, 0, 0);
        }

        // Draw particles
        for (const auto mvp: mvpData_) {
            currentCmdBuffer->BindPipeline(particleGraphicsPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
            const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::ParticleDescSetLayout))};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, particleGraphicsPipelineLayout_, 0,
                                                 descSets);
            currentCmdBuffer->PushConstants(particleGraphicsPipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                            sizeof(MvpData), &mvp);
            currentCmdBuffer->Draw(GetParamU32(AppSettings::ParticleCount), 1, 0, 0);
        }

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    for (size_t i = 0; i < NUM_CUBES; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);

        const glm::mat4 view = camera_->GetViewMatrix();
        glm::mat4 proj = camera_->GetProjectionMatrix();

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
} // namespace examples::fundamentals::compute_shaders::basic_particles
