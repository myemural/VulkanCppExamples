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
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_lighting::lighting_architectures::clustered_forward_unlimited
{
using namespace constants;
using namespace common::scene;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationLightingArchitectures(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationLightingArchitectures::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        BuildScene();
        CreateAndUpdateDescriptorSets();

        InitInputSystem();

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

    UpdateSceneTransforms();
    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationLightingArchitectures::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateInitialResources()
{
    // Compute cluster count
    const auto tilesX = CeilDiv(currentWindowWidth_, TILE_SIZE_X);
    const auto tilesY = CeilDiv(currentWindowHeight_, TILE_SIZE_Y);
    totalClusterCount_ = tilesX * tilesY * Z_SLICE_COUNT;

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const std::uint32_t clusterHeaderBufferSize = totalClusterCount_ * sizeof(ClusterHeader);
    const std::uint32_t globalLightIndexBufferSize = totalClusterCount_ * MAX_LIGHT_COUNT * sizeof(std::uint32_t);
    resourceCreateInfo.buffers = {
        {kPointLightStorageBuffer, sizeof(PointLightData) * MAX_LIGHT_COUNT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kClusterHeaderStorageBuffer, clusterHeaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kGlobalLightIndexStorageBuffer, globalLightIndexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        {kGlobalCounterStorageBuffer, sizeof(std::uint32_t),
         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}};

    // Fill shader module create infos
    const auto clusteredForwardVertexShaderAsset = assetManager_->Load<ShaderAsset>(kClusteredForwardVertexShaderFile);
    const auto clusteredForwardFragmentShaderAsset =
            assetManager_->Load<ShaderAsset>(kClusteredForwardFragmentShaderFile);
    const auto clusterCountOffsetComputeShaderAsset =
            assetManager_->Load<ShaderAsset>(kClusterCountOffsetComputeShaderFile);
    const auto clusterLightIndicesComputeShaderAsset =
            assetManager_->Load<ShaderAsset>(kClusterLightIndicesComputeShaderFile);

    resourceCreateInfo.shaders = {.modules = {{.name = kClusteredForwardVertexShaderKey,
                                               .asset = assetManager_->Get(clusteredForwardVertexShaderAsset)},
                                              {.name = kClusteredForwardFragmentShaderKey,
                                               .asset = assetManager_->Get(clusteredForwardFragmentShaderAsset)},
                                              {.name = kClusterCountOffsetComputeShaderKey,
                                               .asset = assetManager_->Get(clusterCountOffsetComputeShaderAsset)},
                                              {.name = kClusterLightIndicesComputeShaderKey,
                                               .asset = assetManager_->Get(clusterLightIndicesComputeShaderAsset)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = kDepthImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = depthImageFormat_,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .views = {ImageViewCreateInfo{.viewName = kDepthImageView,
                                      .format = depthImageFormat_,
                                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1}}}}};

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TEXCOORD, AccessorType::VEC2);
    sceneConfig.attributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TANGENT, AccessorType::VEC4);

    materialManager_ = std::make_unique<MaterialManager>(*resources_, cmdPool_, queue_);
    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 1.0f, 7.0f), aspectRatio, 45.0f,
                                                  GetParamFloat(AppSettings::CameraNearPlane),
                                                  GetParamFloat(AppSettings::CameraFarPlane));

    // Materials
    const auto wallStoneTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneTexturePath);
    materialManager_->LoadTexture(kWallStoneTexture, kMainSampler, assetManager_->Get(wallStoneTextureAsset));
    const auto wallStoneNormalTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneNormalTexturePath);
    materialManager_->LoadTexture(kWallStoneNormalTexture, kMainSampler,
                                  assetManager_->Get(wallStoneNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    MeshMaterialData defaultMaterial{};
    defaultMaterial.diffuseMap = materialManager_->GetTextureId(kWallStoneTexture);
    defaultMaterial.normalMap = materialManager_->GetTextureId(kWallStoneNormalTexture);

    // Add scene objects and lights
    std::uint32_t index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject).WithPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    for (const auto& modelPos: modelPositions) {
        // Lights
        if (lightPositions_.size() < MAX_LIGHT_COUNT) {
            lightPositions_.emplace_back(modelPos, 1.0f);
            lightColors_.emplace_back(GenerateRandomColor(0.1f, 1.0f));
            continue;
        }

        // Objects
        const auto value = GenerateRandomValue(0U, 1U);
        rootObjectBuilder.AddChild(
                SceneObjectBuilder(*scene_, kCubeObject + std::to_string(index))
                        .WithBuiltinMesh(value == 0 ? BuiltinMeshType::CUBE : BuiltinMeshType::SPHERE)
                        .WithMaterial(defaultMaterial)
                        .WithPosition(modelPos)
                        .WithScale(glm::vec3{2.0f}));

        index++;
    }

    const auto& rootObject = rootObjectBuilder.Build();
    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = materialManager_->GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 6 + combinedImageSamplerCount,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = materialManager_->GetDescriptorImageInfos();

    std::vector<VkDescriptorBufferInfo> storagePointLightBufferInfos;
    storagePointLightBufferInfos.emplace_back(resources_->GetBuffer(kPointLightStorageBuffer)->GetHandle(), 0,
                                              VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageClusterHeaderBufferInfos;
    storageClusterHeaderBufferInfos.emplace_back(resources_->GetBuffer(kClusterHeaderStorageBuffer)->GetHandle(), 0,
                                                 VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageGlobalLightIndexBufferInfos;
    storageGlobalLightIndexBufferInfos.emplace_back(resources_->GetBuffer(kGlobalLightIndexStorageBuffer)->GetHandle(),
                                                    0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageGlobalCounterBufferInfos;
    storageGlobalCounterBufferInfos.emplace_back(resources_->GetBuffer(kGlobalCounterStorageBuffer)->GetHandle(), 0,
                                                 VK_WHOLE_SIZE);

    BufferWriteRequest transformStorageBufferRequest;
    transformStorageBufferRequest.descriptorSetName = kMainDescSet;
    transformStorageBufferRequest.bindingIndex = 0;
    transformStorageBufferRequest.buffers = storageTransformBufferInfos;
    transformStorageBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest transformMaterialBufferRequest;
    transformMaterialBufferRequest.descriptorSetName = kMainDescSet;
    transformMaterialBufferRequest.bindingIndex = 1;
    transformMaterialBufferRequest.buffers = storageMaterialBufferInfos;
    transformMaterialBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = kMainDescSet;
    textureUpdateRequest.bindingIndex = 2;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest pointLightBufferRequest;
    pointLightBufferRequest.descriptorSetName = kMainDescSet;
    pointLightBufferRequest.bindingIndex = 3;
    pointLightBufferRequest.buffers = storagePointLightBufferInfos;
    pointLightBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest clusterHeaderBufferRequest;
    clusterHeaderBufferRequest.descriptorSetName = kMainDescSet;
    clusterHeaderBufferRequest.bindingIndex = 4;
    clusterHeaderBufferRequest.buffers = storageClusterHeaderBufferInfos;
    clusterHeaderBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest globalLightIndexBufferRequest;
    globalLightIndexBufferRequest.descriptorSetName = kMainDescSet;
    globalLightIndexBufferRequest.bindingIndex = 5;
    globalLightIndexBufferRequest.buffers = storageGlobalLightIndexBufferInfos;
    globalLightIndexBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest globalCounterBufferRequest;
    globalCounterBufferRequest.descriptorSetName = kMainDescSet;
    globalCounterBufferRequest.bindingIndex = 6;
    globalCounterBufferRequest.buffers = storageGlobalCounterBufferInfos;
    globalCounterBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {transformStorageBufferRequest, transformMaterialBufferRequest, pointLightBufferRequest,
                                clusterHeaderBufferRequest, globalLightIndexBufferRequest, globalCounterBufferRequest},
        .imageWriteRequests = {textureUpdateRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
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

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    forwardRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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

    if (!forwardRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for forward shading)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange mvpForwardPushConstant;
    mvpForwardPushConstant.offset = 0;
    mvpForwardPushConstant.size = sizeof(ForwardPipelinePushConstants);
    mvpForwardPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    std::vector blendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    forwardPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)},
                                                           {mvpForwardPushConstant});

    if (!forwardPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for forward pipeline)!");
    }

    forwardPassPipeline_ =
            device_->CreateGraphicsPipeline(forwardPipelineLayout_, forwardRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kClusteredForwardVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kClusteredForwardFragmentShaderKey)->GetHandle();
                });
                builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
                    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
                    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
                    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
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

    if (!forwardPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for forward pipeline)!");
    }

    VkPushConstantRange computePushConstant;
    computePushConstant.offset = 0;
    computePushConstant.size = sizeof(ComputePipelinePushConstants);
    computePushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    countOffsetPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {computePushConstant});

    if (!countOffsetPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for count/offset compute)!");
    }


    countOffsetPipeline_ = device_->CreateComputePipeline(countOffsetPipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(kClusterCountOffsetComputeShaderKey)->GetHandle();
        });
    });

    if (!countOffsetPipeline_) {
        throw std::runtime_error("Failed to create compute pipeline (for count/offset compute)!");
    }

    lightIndicesPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {computePushConstant});

    if (!lightIndicesPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for light indices compute)!");
    }


    lightIndicesPipeline_ = device_->CreateComputePipeline(lightIndicesPipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(kClusterLightIndicesComputeShaderKey)->GetHandle();
        });
    });

    if (!lightIndicesPipeline_) {
        throw std::runtime_error("Failed to create compute pipeline (for light indices compute)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    const auto& depthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer =
                device_->CreateFramebuffer(forwardRenderPass_, {swapImage, depthImageView}, [&](auto& builder) {
                    builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
                });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
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
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    clearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Reset global counter buffer
    {
        currentCmdBuffer->FillBuffer(resources_->GetBuffer(kGlobalCounterStorageBuffer), 0, sizeof(uint32_t), 0);
    }

    // Barrier: For using global counter buffer from compute shader
    {
        const auto globalCounterStorageBuffer = resources_->GetBuffer(kGlobalCounterStorageBuffer);
        const auto barrier = globalCounterStorageBuffer->CreateBufferMemoryBarrier(
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
                                          {barrier});
    }

    // Compute Pass: Compute light count/offset per cluster
    {
        currentCmdBuffer->BindPipeline(countOffsetPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, countOffsetPipelineLayout_, 0, descSets);

        ComputePipelinePushConstants computePushConstants{};
        computePushConstants.proj = camera_->GetProjectionMatrix();
        computePushConstants.screenSizeAndPlanes =
                glm::vec4(currentWindowWidth_, currentWindowHeight_, GetParamFloat(AppSettings::CameraNearPlane),
                          GetParamFloat(AppSettings::CameraFarPlane));
        computePushConstants.lightCount = lightPositions_.size();
        currentCmdBuffer->PushConstants(countOffsetPipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(computePushConstants), &computePushConstants);
        currentCmdBuffer->Dispatch(totalClusterCount_, 1, 1);
    }

    // Barrier: For using cluster header buffer from another compute shader and fragment shader
    {
        const auto clusterHeaderBuffer = resources_->GetBuffer(kClusterHeaderStorageBuffer);
        const auto barrier =
                clusterHeaderBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {barrier});
    }

    // Compute Pass: Compute light indices per cluster
    {
        currentCmdBuffer->BindPipeline(lightIndicesPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, lightIndicesPipelineLayout_, 0, descSets);

        ComputePipelinePushConstants computePushConstants{};
        computePushConstants.proj = camera_->GetProjectionMatrix();
        computePushConstants.screenSizeAndPlanes =
                glm::vec4(currentWindowWidth_, currentWindowHeight_, GetParamFloat(AppSettings::CameraNearPlane),
                          GetParamFloat(AppSettings::CameraFarPlane));
        computePushConstants.lightCount = lightPositions_.size();
        currentCmdBuffer->PushConstants(lightIndicesPipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(computePushConstants), &computePushConstants);
        currentCmdBuffer->Dispatch(totalClusterCount_, 1, 1);
    }

    // Barrier: For using global light index buffer from fragment shader
    {
        const auto globalLightIndexBuffer = resources_->GetBuffer(kGlobalLightIndexStorageBuffer);
        const auto barrier = globalLightIndexBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
                                                                               VK_ACCESS_SHADER_READ_BIT);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          {}, {barrier});
    }

    // Render Pass: Forward shading
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = forwardRenderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        // Draw scene objects
        currentCmdBuffer->BindPipeline(forwardPassPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable()) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                ForwardPipelinePushConstants meshPushConstants{};
                meshPushConstants.objectId = sceneObject.GetObjectId();
                meshPushConstants.tilesX = CeilDiv(currentWindowWidth_, TILE_SIZE_X);
                meshPushConstants.view = camera_->GetViewMatrix();
                meshPushConstants.projection = camera_->GetProjectionMatrix();
                meshPushConstants.nearPlane = GetParamFloat(AppSettings::CameraNearPlane);
                meshPushConstants.farPlane = GetParamFloat(AppSettings::CameraFarPlane);
                currentCmdBuffer->PushConstants(forwardPipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(meshPushConstants), &meshPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    std::vector<PointLightData> pointLightInfos;
    for (auto i = 0U; i < lightPositions_.size(); ++i) {
        PointLightData pointLightData{};
        pointLightData.lightPosition = camera_->GetViewMatrix() * lightPositions_[i];
        pointLightData.lightColorRadius = glm::vec4(lightColors_[i], GetParamFloat(AppSettings::LightRadius));
        pointLightInfos.push_back(pointLightData);
    }
    resources_->SetBuffer(kPointLightStorageBuffer, pointLightInfos.data(),
                          pointLightInfos.size() * sizeof(PointLightData));
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
} // namespace examples::real_time_lighting::lighting_architectures::clustered_forward_unlimited
