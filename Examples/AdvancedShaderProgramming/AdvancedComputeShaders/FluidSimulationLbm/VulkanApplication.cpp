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
#include "VulkanQueryPool.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
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

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
        CreateFramebuffers();
        CreateCommandBuffers();
        CreateQueryPools();
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

    PrintPerformanceStats();
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

    // Fill buffer create infos
    constexpr VkBufferUsageFlags storageBufferUsage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    const auto cellCount = currentWindowWidth_ * currentWindowHeight_;
    const std::uint32_t distributionBufferSize = cellCount * kDirectionCount * sizeof(float);
    const std::uint32_t dyeBufferSize = cellCount * sizeof(float);
    const std::uint32_t obstacleBufferSize = cellCount * sizeof(std::uint32_t);

    resourceCreateInfo.buffers = {
        {kDistributionBufferA, distributionBufferSize, storageBufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kDistributionBufferB, distributionBufferSize, storageBufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kDyeBufferA, dyeBufferSize, storageBufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kDyeBufferB, dyeBufferSize, storageBufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kObstacleBuffer, obstacleBufferSize, storageBufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
    };

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);
    const auto collideStreamComputeShaderAsset = assetManager_->Load<ShaderAsset>(kCollideStreamComputeShaderFile);
    const auto advectDyeComputeShaderAsset = assetManager_->Load<ShaderAsset>(kAdvectDyeComputeShaderFile);
    const auto paintObstacleComputeShaderAsset = assetManager_->Load<ShaderAsset>(kPaintObstacleComputeShaderFile);
    const auto colorizeComputeShaderAsset = assetManager_->Load<ShaderAsset>(kColorizeComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
            {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)},
            {.name = kCollideStreamComputeShaderKey, .asset = assetManager_->Get(collideStreamComputeShaderAsset)},
            {.name = kAdvectDyeComputeShaderKey, .asset = assetManager_->Get(advectDyeComputeShaderAsset)},
            {.name = kPaintObstacleComputeShaderKey, .asset = assetManager_->Get(paintObstacleComputeShaderAsset)},
            {.name = kColorizeComputeShaderKey, .asset = assetManager_->Get(colorizeComputeShaderAsset)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = kScreenStorageImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .views = {ImageViewCreateInfo{.viewName = kScreenStorageImageView, .format = VK_FORMAT_R8G8B8A8_UNORM}}}};

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kComputeDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kComputeDescSetAToB, .layoutName = kComputeDescSetLayout},
                           {.name = kComputeDescSetBToA, .layoutName = kComputeDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorImageInfo> screenSamplerImageInfos;
    screenSamplerImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kScreenStorageImage, kScreenStorageImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> screenStorageImageInfos;
    screenStorageImageInfos.emplace_back(
            VK_NULL_HANDLE, resources_->GetImageView(kScreenStorageImage, kScreenStorageImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorBufferInfo> distBufferAInfos;
    distBufferAInfos.emplace_back(resources_->GetBuffer(kDistributionBufferA)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> distBufferBInfos;
    distBufferBInfos.emplace_back(resources_->GetBuffer(kDistributionBufferB)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> dyeBufferAInfos;
    dyeBufferAInfos.emplace_back(resources_->GetBuffer(kDyeBufferA)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> dyeBufferBInfos;
    dyeBufferBInfos.emplace_back(resources_->GetBuffer(kDyeBufferB)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> obstacleBufferInfos;
    obstacleBufferInfos.emplace_back(resources_->GetBuffer(kObstacleBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.descriptorSetName = kMainDescSet;
    samplerUpdateRequest.bindingIndex = 0;
    samplerUpdateRequest.images = screenSamplerImageInfos;
    samplerUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest computeAToBDistBufferPrev;
    computeAToBDistBufferPrev.descriptorSetName = kComputeDescSetAToB;
    computeAToBDistBufferPrev.bindingIndex = 0;
    computeAToBDistBufferPrev.buffers = distBufferAInfos;
    computeAToBDistBufferPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBDistBufferNext;
    computeAToBDistBufferNext.descriptorSetName = kComputeDescSetAToB;
    computeAToBDistBufferNext.bindingIndex = 1;
    computeAToBDistBufferNext.buffers = distBufferBInfos;
    computeAToBDistBufferNext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBDyeBufferPrev;
    computeAToBDyeBufferPrev.descriptorSetName = kComputeDescSetAToB;
    computeAToBDyeBufferPrev.bindingIndex = 2;
    computeAToBDyeBufferPrev.buffers = dyeBufferAInfos;
    computeAToBDyeBufferPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBDyeBufferNext;
    computeAToBDyeBufferNext.descriptorSetName = kComputeDescSetAToB;
    computeAToBDyeBufferNext.bindingIndex = 3;
    computeAToBDyeBufferNext.buffers = dyeBufferBInfos;
    computeAToBDyeBufferNext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBObstacleBuffer;
    computeAToBObstacleBuffer.descriptorSetName = kComputeDescSetAToB;
    computeAToBObstacleBuffer.bindingIndex = 4;
    computeAToBObstacleBuffer.buffers = obstacleBufferInfos;
    computeAToBObstacleBuffer.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest computeAToBScreen;
    computeAToBScreen.descriptorSetName = kComputeDescSetAToB;
    computeAToBScreen.bindingIndex = 5;
    computeAToBScreen.images = screenStorageImageInfos;
    computeAToBScreen.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    BufferWriteRequest computeBToADistBufferPrev;
    computeBToADistBufferPrev.descriptorSetName = kComputeDescSetBToA;
    computeBToADistBufferPrev.bindingIndex = 0;
    computeBToADistBufferPrev.buffers = distBufferBInfos;
    computeBToADistBufferPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToADistBufferNext;
    computeBToADistBufferNext.descriptorSetName = kComputeDescSetBToA;
    computeBToADistBufferNext.bindingIndex = 1;
    computeBToADistBufferNext.buffers = distBufferAInfos;
    computeBToADistBufferNext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToADyeBufferPrev;
    computeBToADyeBufferPrev.descriptorSetName = kComputeDescSetBToA;
    computeBToADyeBufferPrev.bindingIndex = 2;
    computeBToADyeBufferPrev.buffers = dyeBufferBInfos;
    computeBToADyeBufferPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToADyeBufferNext;
    computeBToADyeBufferNext.descriptorSetName = kComputeDescSetBToA;
    computeBToADyeBufferNext.bindingIndex = 3;
    computeBToADyeBufferNext.buffers = dyeBufferAInfos;
    computeBToADyeBufferNext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToAObstacleBuffer;
    computeBToAObstacleBuffer.descriptorSetName = kComputeDescSetBToA;
    computeBToAObstacleBuffer.bindingIndex = 4;
    computeBToAObstacleBuffer.buffers = obstacleBufferInfos;
    computeBToAObstacleBuffer.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest computeBToAScreen;
    computeBToAScreen.descriptorSetName = kComputeDescSetBToA;
    computeBToAScreen.bindingIndex = 5;
    computeBToAScreen.images = screenStorageImageInfos;
    computeBToAScreen.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {computeAToBDistBufferPrev, computeAToBDistBufferNext, computeAToBDyeBufferPrev,
                                computeAToBDyeBufferNext, computeAToBObstacleBuffer, computeBToADistBufferPrev,
                                computeBToADistBufferNext, computeBToADyeBufferPrev, computeBToADyeBufferNext,
                                computeBToAObstacleBuffer},
        .imageWriteRequests = {samplerUpdateRequest, computeAToBScreen, computeBToAScreen}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::InitInputSystem()
{
    window_->OnKey([&](const KeyEvent& event) {
        if (event.action != GLFW_PRESS) {
            return;
        }

        switch (event.key) {
            case GLFW_KEY_1:
                displayMode_ = DisplayMode::VELOCITY_MAGNITUDE;
                std::cout << "Display mode changed: Velocity Magnitude" << std::endl;
                break;
            case GLFW_KEY_2:
                displayMode_ = DisplayMode::VORTICITY;
                std::cout << "Display mode changed: Vorticity" << std::endl;
                break;
            case GLFW_KEY_3:
                displayMode_ = DisplayMode::DENSITY;
                std::cout << "Display mode changed: Density" << std::endl;
                break;
            case GLFW_KEY_4:
                displayMode_ = DisplayMode::DYE;
                std::cout << "Display mode changed: Dye" << std::endl;
                break;
            case GLFW_KEY_SPACE:
                isSimulationPaused_ = !isSimulationPaused_;
                std::cout << "Simulation paused: " << (isSimulationPaused_ ? "YES" : "NO") << std::endl;
                break;
            case GLFW_KEY_R:
                lbmStepCounter_ = 0;
                std::cout << "LBM step counter reset!" << std::endl;
                break;
            default:
                break;
        }
    });

    window_->OnMouseMove([this](const MouseMoveEvent& event) {
        mousePos_ = glm::vec2(static_cast<float>(event.x), static_cast<float>(event.y));
    });

    window_->OnMouseButton([this](const MouseButtonEvent& event) {
        const bool isPressed = event.action != GLFW_RELEASE;

        if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
            mouseMode_ = isPressed ? MouseMode::ADD : MouseMode::NONE;
        } else if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
            mouseMode_ = isPressed ? MouseMode::ERASE : MouseMode::NONE;
        }
    });
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
    VkPushConstantRange lbmPushConstant;
    lbmPushConstant.offset = 0;
    lbmPushConstant.size = sizeof(LbmPushConstants);
    lbmPushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kComputeDescSetLayout)}, {lbmPushConstant});

    if (!computePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    collideStreamPipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kCollideStreamComputeShaderKey)->GetHandle();
        });
    });

    if (!collideStreamPipeline_) {
        throw std::runtime_error("Failed to create collide stream pipeline!");
    }

    advectDyePipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kAdvectDyeComputeShaderKey)->GetHandle();
        });
    });

    if (!advectDyePipeline_) {
        throw std::runtime_error("Failed to create advect dye pipeline!");
    }

    paintObstaclePipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kPaintObstacleComputeShaderKey)->GetHandle();
        });
    });

    if (!paintObstaclePipeline_) {
        throw std::runtime_error("Failed to create paint obstacle pipeline!");
    }

    colorizePipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kColorizeComputeShaderKey)->GetHandle();
        });
    });

    if (!colorizePipeline_) {
        throw std::runtime_error("Failed to create colorize pipeline!");
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

void VulkanApplication::CreateQueryPools()
{
    timestampQueryPool_ = device_->CreateQueryPool(VK_QUERY_TYPE_TIMESTAMP, 2);

    if (!timestampQueryPool_) {
        throw std::runtime_error("Failed to create timestamp query pool!");
    }
}

void VulkanApplication::PrintPerformanceStats()
{
    if (!isFirstFrameDone_ || ++statFrameCounter_ < 120U) {
        return;
    }

    statFrameCounter_ = 0U;

    std::uint64_t timestamps[2];
    timestampQueryPool_->GetQueryPoolResults(0, 2, sizeof(timestamps), timestamps, sizeof(std::uint64_t),
                                             VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    const auto timestampPeriod = physicalDevice_->GetProperties().limits.timestampPeriod;
    const auto elapsedNanoSeconds = static_cast<double>(timestamps[1] - timestamps[0]) * timestampPeriod;

    if (elapsedNanoSeconds <= 0.0) {
        return;
    }

    // Million lattice updates per second (MLUPS)
    const auto latticeUpdates = static_cast<double>(currentWindowWidth_ * currentWindowHeight_) * kStepsPerFrame;
    std::cout << "LBM: " << (latticeUpdates * 1000.0 / elapsedNanoSeconds) << " MLUPS (" << kStepsPerFrame
              << " steps in " << (elapsedNanoSeconds / 1.0e6) << " ms)" << std::endl;
}

void VulkanApplication::RecordSimulationStep(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer,
                                             const std::uint32_t stepIndex)
{
    const auto groupCountX = CeilDiv(currentWindowWidth_, kLocalSizeX);
    const auto groupCountY = CeilDiv(currentWindowHeight_, kLocalSizeY);

    const std::vector genericComputeMemBarriers{
        VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT}};

    // Even steps read A and write B, odd steps read B and write A
    const std::vector descSets{
        resources_->GetDescriptorSet(stepIndex % 2 == 0 ? kComputeDescSetAToB : kComputeDescSetBToA)};
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout_, 0, descSets);

    LbmPushConstants pushConstants{};
    pushConstants.gridWidth = currentWindowWidth_;
    pushConstants.gridHeight = currentWindowHeight_;
    pushConstants.relaxationTime = kRelaxationTime;
    pushConstants.inletVelocity = kInletVelocity;
    pushConstants.stepIndex = stepIndex;
    pushConstants.displayMode = static_cast<std::uint32_t>(displayMode_);
    pushConstants.mousePos = mousePos_;
    pushConstants.brushRadius = kBrushRadius;
    pushConstants.mouseMode = static_cast<std::uint32_t>(mouseMode_);

    cmdBuffer->PushConstants(computePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LbmPushConstants),
                             &pushConstants);

    cmdBuffer->BindPipeline(collideStreamPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
    cmdBuffer->Dispatch(groupCountX, groupCountY, 1);

    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {},
                               genericComputeMemBarriers);

    cmdBuffer->BindPipeline(advectDyePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
    cmdBuffer->Dispatch(groupCountX, groupCountY, 1);

    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {},
                               genericComputeMemBarriers);
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];
    const auto screenStorageImage = resources_->GetImage(kScreenStorageImage);

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    currentCmdBuffer->ResetQueryPool(timestampQueryPool_, 0, 2);

    // Clear all simulation buffers the very first step (and after every reset action)
    if (lbmStepCounter_ == 0) {
        for (const auto& bufferName:
             {kDistributionBufferA, kDistributionBufferB, kDyeBufferA, kDyeBufferB, kObstacleBuffer}) {
            currentCmdBuffer->FillBuffer(resources_->GetBuffer(bufferName), 0, VK_WHOLE_SIZE, 0);
        }

        const std::vector transferToComputeMemBarriers{
            VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
                            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT}};
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {},
                                          transferToComputeMemBarriers);
    }

    // Barrier phase for proper image layout transitions
    if (!isFirstFrameDone_) {
        const std::vector initialBarriers{screenStorageImage->CreateImageMemoryBarrier(
                0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL)};

        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          initialBarriers);
    } else {
        const std::vector screenReadOnlyToGeneralBarrier{screenStorageImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL)};
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, screenReadOnlyToGeneralBarrier);
    }

    // Compute phase
    {
        const auto groupCountX = CeilDiv(currentWindowWidth_, kLocalSizeX);
        const auto groupCountY = CeilDiv(currentWindowHeight_, kLocalSizeY);

        // The obstacle brush writes into the buffer that the next simulation step is going to read
        if (mouseMode_ != MouseMode::NONE) {
            const std::vector brushDescSets{
                resources_->GetDescriptorSet((lbmStepCounter_ % 2 == 0) ? kComputeDescSetAToB : kComputeDescSetBToA)};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout_, 0,
                                                 brushDescSets);


            LbmPushConstants pushConstants{};
            pushConstants.gridWidth = currentWindowWidth_;
            pushConstants.gridHeight = currentWindowHeight_;
            pushConstants.relaxationTime = kRelaxationTime;
            pushConstants.inletVelocity = kInletVelocity;
            pushConstants.stepIndex = lbmStepCounter_;
            pushConstants.displayMode = static_cast<std::uint32_t>(displayMode_);
            pushConstants.mousePos = mousePos_;
            pushConstants.brushRadius = kBrushRadius;
            pushConstants.mouseMode = static_cast<std::uint32_t>(mouseMode_);

            currentCmdBuffer->PushConstants(computePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                            sizeof(LbmPushConstants), &pushConstants);

            currentCmdBuffer->BindPipeline(paintObstaclePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
            currentCmdBuffer->Dispatch(groupCountX, groupCountY, 1);

            const std::vector genericComputeMemBarriers{
                VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
                                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT}};
            currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {}, genericComputeMemBarriers);
        }

        currentCmdBuffer->WriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestampQueryPool_, 0);

        if (!isSimulationPaused_) {
            for (auto step = 0U; step < kStepsPerFrame; ++step) {
                RecordSimulationStep(currentCmdBuffer, lbmStepCounter_);
                ++lbmStepCounter_;
            }
        }

        currentCmdBuffer->WriteTimestamp(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampQueryPool_, 1);

        // The last step wrote into the output buffers
        const std::vector colorizeDescSets{
            resources_->GetDescriptorSet((lbmStepCounter_ % 2 == 0) ? kComputeDescSetAToB : kComputeDescSetBToA)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout_, 0,
                                             colorizeDescSets);

        LbmPushConstants pushConstants{};
        pushConstants.gridWidth = currentWindowWidth_;
        pushConstants.gridHeight = currentWindowHeight_;
        pushConstants.relaxationTime = kRelaxationTime;
        pushConstants.inletVelocity = kInletVelocity;
        pushConstants.stepIndex = lbmStepCounter_;
        pushConstants.displayMode = static_cast<std::uint32_t>(displayMode_);
        pushConstants.mousePos = mousePos_;
        pushConstants.brushRadius = kBrushRadius;
        pushConstants.mouseMode = static_cast<std::uint32_t>(mouseMode_);

        currentCmdBuffer->PushConstants(computePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(LbmPushConstants), &pushConstants);

        currentCmdBuffer->BindPipeline(colorizePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        currentCmdBuffer->Dispatch(groupCountX, groupCountY, 1);
    }

    // Render phase
    {
        // Change image layout from GENERAL to SHADER_READ_ONLY
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

    isFirstFrameDone_ = true;
}
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
