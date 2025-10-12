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
#include "ApplicationData.h"
#include "VulkanHelpers.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
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

        float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        camera = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 4.0f), aspectRatio);

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
        CreateFramebuffers(resources_->GetImageView(GetParamStr(AppConstants::DepthImage),
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

    timeSpeedUbo.time = static_cast<float>(glfwGetTime());
    timeSpeedUbo.speed = GetParamFloat(AppSettings::CloudSpeed);
    resources_->SetBuffer(GetParamStr(AppConstants::TimeSpeedUniformBuffer), &timeSpeedUbo,
                          sizeof(UniformBufferObject));
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
    ApplicationPipelinesAndPasses::PreUpdate();

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
    cloudTextureHandler_ = textureLoader.Load(GetParamStr(AppConstants::CloudTexturePath));

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const std::uint32_t cubeVertexBufferSize = cubeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t cubeIndexBufferSize = cubeIndices.size() * sizeof(cubeIndices[0]);
    const std::uint32_t planeVertexBufferSize = planeVertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t planeIndexBufferSize = planeIndices.size() * sizeof(planeIndices[0]);
    constexpr std::uint32_t timeSpeedUboSize = sizeof(UniformBufferObject);

    resourceCreateInfo.Buffers = {
        {GetParamStr(AppConstants::CubeVertexBuffer), cubeVertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::CubeIndexBuffer), cubeIndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::PlaneVertexBuffer), planeVertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::PlaneIndexBuffer), planeIndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::TimeSpeedUniformBuffer), timeSpeedUboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
                                  .Modules = {{.Name = GetParamStr(AppConstants::BackgroundVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::BackgroundVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::CubeVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::CubeVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::BackgroundFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::BackgroundFragmentShaderFile)},
                                              {.Name = GetParamStr(AppConstants::CubeFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::CubeFragmentShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.Descriptors = {
        .MaxSets = 3,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}},
        .Layouts = {
            {.Name = GetParamStr(AppConstants::CubeDescSetLayout),
             .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
            {.Name = GetParamStr(AppConstants::BackgroundDescSetLayout),
             .Bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}};

    resourceCreateInfo.Images = {
        ImageResourceCreateInfo{.Name = GetParamStr(AppConstants::CrateImage),
                                .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .Format = VK_FORMAT_R8G8B8A8_SRGB,
                                .Dimensions = {crateTextureHandler_.Width, crateTextureHandler_.Height, 1},
                                .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::CrateImageView),
                                                              .Format = VK_FORMAT_R8G8B8A8_SRGB}}},
        ImageResourceCreateInfo{.Name = GetParamStr(AppConstants::CloudImage),
                                .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .Format = VK_FORMAT_R8G8B8A8_SRGB,
                                .Dimensions = {cloudTextureHandler_.Width, cloudTextureHandler_.Height, 1},
                                .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::CloudImageView),
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
    resources_->SetBuffer(GetParamStr(AppConstants::PlaneVertexBuffer), planeVertices.data(),
                          planeVertices.size() * sizeof(VertexPos3Uv2));
    resources_->SetBuffer(GetParamStr(AppConstants::PlaneIndexBuffer), planeIndices.data(),
                          planeIndices.size() * sizeof(planeIndices[0]));

    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::CrateImage), crateTextureHandler_);
    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::CloudImage), cloudTextureHandler_);

    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPasses()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    backgroundRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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

    if (!backgroundRenderPass_) {
        throw std::runtime_error("Failed to create render pass (background)!");
    }

    foregroundRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
               })
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat_;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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

    if (!foregroundRenderPass_) {
        throw std::runtime_error("Failed to create render pass (foreground)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    backgroundPipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::BackgroundDescSetLayout))});

    if (!backgroundPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (background)!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos3Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

    backgroundPipeline_ =
            device_->CreateGraphicsPipeline(backgroundPipelineLayout_, backgroundRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(GetParamStr(AppConstants::BackgroundVertexShaderKey))
                                    ->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(GetParamStr(AppConstants::BackgroundFragmentShaderKey))
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

    if (!backgroundPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (background)!");
    }

    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MvpData);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    cubePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::CubeDescSetLayout))}, {mvpPushConstant});

    if (!cubePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (cube)!");
    }

    cubePipeline_ = device_->CreateGraphicsPipeline(cubePipelineLayout_, foregroundRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::CubeVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::CubeFragmentShaderKey))->GetHandle();
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

    if (!cubePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (cube)!");
    }
}

void VulkanApplication::CreateFramebuffers(const std::shared_ptr<VulkanImageView>& depthImageView)
{
    for (const auto& swapImage: swapChainImageViews_) {
        const auto backgroundFramebuffer =
                device_->CreateFramebuffer(backgroundRenderPass_, {swapImage, depthImageView}, [&](auto& builder) {
                    builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
                });

        if (!backgroundFramebuffer) {
            throw std::runtime_error("Failed to create framebuffer (background)!");
        }

        const auto foregroundFramebuffer =
                device_->CreateFramebuffer(foregroundRenderPass_, {swapImage, depthImageView}, [&](auto& builder) {
                    builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
                });

        if (!foregroundFramebuffer) {
            throw std::runtime_error("Failed to create framebuffer (foreground)!");
        }

        backgroundFramebuffers_.push_back(backgroundFramebuffer);
        foregroundFramebuffers_.push_back(foregroundFramebuffer);
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> backgroundSamplerInfos;
    backgroundSamplerInfos.emplace_back(
            resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
            resources_->GetImageView(GetParamStr(AppConstants::CloudImage), GetParamStr(AppConstants::CloudImageView))
                    ->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.emplace_back(resources_->GetBuffer(GetParamStr(AppConstants::TimeSpeedUniformBuffer))->GetHandle(), 0,
                             VK_WHOLE_SIZE);

    std::vector<VkDescriptorImageInfo> cubeSamplerInfos;
    cubeSamplerInfos.emplace_back(
            resources_->GetSampler(GetParamStr(AppConstants::MainSampler))->GetHandle(),
            resources_->GetImageView(GetParamStr(AppConstants::CrateImage), GetParamStr(AppConstants::CrateImageView))
                    ->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest samplerBackgroundUpdateRequest;
    samplerBackgroundUpdateRequest.LayoutName = GetParamStr(AppConstants::BackgroundDescSetLayout);
    samplerBackgroundUpdateRequest.BindingIndex = 0;
    samplerBackgroundUpdateRequest.Images = backgroundSamplerInfos;
    samplerBackgroundUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest uboUpdateRequest;
    uboUpdateRequest.LayoutName = GetParamStr(AppConstants::BackgroundDescSetLayout);
    uboUpdateRequest.BindingIndex = 1;
    uboUpdateRequest.Buffers = bufferInfos;
    uboUpdateRequest.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest samplerCubeUpdateRequest;
    samplerCubeUpdateRequest.LayoutName = GetParamStr(AppConstants::CubeDescSetLayout);
    samplerCubeUpdateRequest.BindingIndex = 0;
    samplerCubeUpdateRequest.Images = cubeSamplerInfos;
    samplerCubeUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .BufferWriteRequests = {uboUpdateRequest},
        .ImageWriteRequests = {samplerBackgroundUpdateRequest, samplerCubeUpdateRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(swapChainImageViews_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

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

    // First render pass for background
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = backgroundRenderPass_->GetHandle();
                    beginInfo.framebuffer = backgroundFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::BackgroundDescSetLayout))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, backgroundPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers{resources_->GetBuffer(GetParamStr(AppConstants::PlaneVertexBuffer))};
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});
        currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::PlaneIndexBuffer)));
        currentCmdBuffer->BindPipeline(backgroundPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        currentCmdBuffer->DrawIndexed(planeIndices.size(), 1, 0, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    // Second render pass for foreground
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = foregroundRenderPass_->GetHandle();
                    beginInfo.framebuffer = foregroundFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::CubeDescSetLayout))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, cubePipelineLayout_, 0, descSets);
        const std::vector vertexBuffers{resources_->GetBuffer(GetParamStr(AppConstants::CubeVertexBuffer))};
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});
        currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(GetParamStr(AppConstants::CubeIndexBuffer)));
        currentCmdBuffer->BindPipeline(cubePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        for (auto& cubeMvp: cubeMvpData) {
            currentCmdBuffer->PushConstants(cubePipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MvpData),
                                            &cubeMvp);
            currentCmdBuffer->DrawIndexed(cubeIndices.size(), 1, 0, 0, 0);
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
        model = glm::translate(model, modelPositions[i]);

        const glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 proj = camera->GetProjectionMatrix();

        // Calculate MVP matrix
        cubeMvpData[i].mvpMatrix = proj * view * model;
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
} // namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
