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
#include "GltfToSceneObjectConverter.h"
#include "ModelAsset.h"
#include "ModelLoader.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_shadows::shadow_filtering_and_softening::percentage_closer_filtering
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params)
    : ApplicationShadowFilteringAndSoftening(std::move(params))
{
}

bool VulkanApplication::Init()
{
    if (!ApplicationShadowFilteringAndSoftening::Init()) {
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
    ApplicationShadowFilteringAndSoftening::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
    assetManager_->RegisterLoader<GltfModelAsset>(std::make_unique<ModelLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto sceneVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSceneVertexShaderFile);
    const auto sceneFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneFragmentShaderFile);
    const auto shadowMapVertexShaderAsset = assetManager_->Load<ShaderAsset>(kShadowMapVertexShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
                    {.name = kSceneFragmentShaderKey, .asset = assetManager_->Get(sceneFragmentShaderAsset)},
                    {.name = kShadowMapVertexShaderKey, .asset = assetManager_->Get(shadowMapVertexShaderAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kShadowMapImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = depthImageFormat_,
            .dimensions = {kShadowMapSize, kShadowMapSize, 1},
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kShadowMapImageView,
                                          .format = depthImageFormat_,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}},
        ImageResourceCreateInfo{
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
        SamplerResourceCreateInfo{.name = kMainSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}},
        SamplerResourceCreateInfo{.name = kShadowSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR},
                                  .addressModes = {.u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                                   .v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                                   .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 1.0f, 9.0f), aspectRatio);

    // Add camera for directional light
    lightCamera_ = std::make_shared<OrthographicCamera>(glm::vec3(0.0f), 1.0f, 40.0f, 0.1f, 50.0f);

    // Materials
    const auto woodFloorTextureAsset = assetManager_->Load<TextureAsset>(kWoodFloorTexturePath);
    const auto woodFloorTextureId =
            sceneImageStorage.StoreTexture(kWoodFloorTexture, kMainSampler, assetManager_->Get(woodFloorTextureAsset));
    const auto woodFloorNormalTextureAsset = assetManager_->Load<TextureAsset>(kWoodFloorNormalTexturePath);
    const auto woodFloorNormalTextureId =
            sceneImageStorage.StoreTexture(kWoodFloorNormalTexture, kMainSampler,
                                           assetManager_->Get(woodFloorNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);
    const auto terracottaTextureAsset = assetManager_->Load<TextureAsset>(kTerracottaTexturePath);
    const auto terracottaTextureId = sceneImageStorage.StoreTexture(kTerracottaTexture, kMainSampler,
                                                                    assetManager_->Get(terracottaTextureAsset));
    const auto terracottaNormalTextureAsset = assetManager_->Load<TextureAsset>(kTerracottaNormalTexturePath);
    const auto terracottaNormalTextureId =
            sceneImageStorage.StoreTexture(kTerracottaNormalTexture, kMainSampler,
                                           assetManager_->Get(terracottaNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    Material objectMaterial;
    objectMaterial.ambientStrength = kAmbientStrength;
    objectMaterial.shininess = kSpecularShininess;
    objectMaterial.specularStrength = kSpecularStrength;
    objectMaterial.uvScale = 2.0f;
    objectMaterial.diffuseMap = woodFloorTextureId;
    objectMaterial.normalMap = woodFloorNormalTextureId;

    Material floorMaterial;
    floorMaterial.ambientStrength = kAmbientStrength;
    floorMaterial.shininess = kSpecularShininess;
    floorMaterial.specularStrength = kSpecularStrength;
    floorMaterial.uvScale = 4.0f;
    floorMaterial.diffuseMap = terracottaTextureId;
    floorMaterial.normalMap = terracottaNormalTextureId;

    // Load and convert models
    const auto antiqueCameraModelHandle = assetManager_->Load<GltfModelAsset>(kAntiqueCameraModelPath);
    const auto antiqueCameraModel = std::make_unique<GltfModelAsset>(assetManager_->Get(antiqueCameraModelHandle));

    GltfToSceneObjectConverter converter{*scene_, kMainSampler};
    auto antiqueCameraModelSceneObjectBuilder =
            converter.ConvertToBuilder(kAntiqueCameraModelName, *antiqueCameraModel);

    const auto rootObject =
            SceneObjectBuilder(*scene_, kRootObject)
                    .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                    .AddChild(antiqueCameraModelSceneObjectBuilder.WithPosition(glm::vec3{0.0f, -2.0f, 0.0f})
                                      .WithScale(glm::vec3{0.5f}))
                    .AddChild(SceneObjectBuilder(*scene_, kCubeObject)
                                      .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                      .WithMaterial(objectMaterial)
                                      .WithPosition(glm::vec3{4.0f, -1.0f, 0.0f})
                                      .WithScale(glm::vec3{2.0f}))
                    .AddChild(SceneObjectBuilder(*scene_, kSphereObject)
                                      .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                      .WithMaterial(objectMaterial)
                                      .WithPosition(glm::vec3{4.0f, 1.5f, 0.0f})
                                      .WithScale(glm::vec3{2.0f}))
                    .AddChild(SceneObjectBuilder(*scene_, kConeObject)
                                      .WithBuiltinMesh(BuiltinMeshType::CONE)
                                      .WithMaterial(objectMaterial)
                                      .WithPosition(glm::vec3{-4.0f, 0.5f, 0.0f})
                                      .WithScale(glm::vec3{1.0f}))
                    .AddChild(SceneObjectBuilder(*scene_, kFloorObject)
                                      .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                      .WithMaterial(floorMaterial)
                                      .WithPosition(glm::vec3{0.0f, -2.0f, 0.0f})
                                      .WithScale(glm::vec3{30.0f}))
                    .Build();

    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kShadowMapDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kShadowMapDescSet, .layoutName = kShadowMapDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

    std::vector<VkDescriptorImageInfo> shadowMapImageInfos;
    shadowMapImageInfos.emplace_back(resources_->GetSampler(kShadowSampler)->GetHandle(),
                                     resources_->GetImageView(kShadowMapImage, kShadowMapImageView)->GetHandle(),
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    BufferWriteRequest objectStorageTransformBufferRequest;
    objectStorageTransformBufferRequest.descriptorSetName = kMainDescSet;
    objectStorageTransformBufferRequest.bindingIndex = 0;
    objectStorageTransformBufferRequest.buffers = storageTransformBufferInfos;
    objectStorageTransformBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest objectStorageMaterialBufferRequest;
    objectStorageMaterialBufferRequest.descriptorSetName = kMainDescSet;
    objectStorageMaterialBufferRequest.bindingIndex = 1;
    objectStorageMaterialBufferRequest.buffers = storageMaterialBufferInfos;
    objectStorageMaterialBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.descriptorSetName = kMainDescSet;
    lightUboRequest.bindingIndex = 2;
    lightUboRequest.buffers = lightUboInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = kMainDescSet;
    textureUpdateRequest.bindingIndex = 3;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest shadowMapTextureUpdateRequest;
    shadowMapTextureUpdateRequest.descriptorSetName = kMainDescSet;
    shadowMapTextureUpdateRequest.bindingIndex = 4;
    shadowMapTextureUpdateRequest.images = shadowMapImageInfos;
    shadowMapTextureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest shadowMapTransformBufferRequest;
    shadowMapTransformBufferRequest.descriptorSetName = kShadowMapDescSet;
    shadowMapTransformBufferRequest.bindingIndex = 0;
    shadowMapTransformBufferRequest.buffers = storageTransformBufferInfos;
    shadowMapTransformBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest, shadowMapTransformBufferRequest},
        .imageWriteRequests = {textureUpdateRequest, shadowMapTextureUpdateRequest}};

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

    sceneRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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

    if (!sceneRenderPass_) {
        throw std::runtime_error("Failed to create render pass for scene!");
    }

    VkAttachmentReference shadowDepthAttachmentRef{0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    shadowRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([&](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = depthImageFormat_;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 0;
                    subpassCreateInfo.pColorAttachments = nullptr;
                    subpassCreateInfo.pDepthStencilAttachment = &shadowDepthAttachmentRef;
                });
    });

    if (!shadowRenderPass_) {
        throw std::runtime_error("Failed to create render pass for shadow mapping!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange scenePushConstant;
    scenePushConstant.offset = 0;
    scenePushConstant.size = sizeof(ScenePushConstants);
    scenePushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {scenePushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    scenePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, sceneRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneFragmentShaderKey)->GetHandle();
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

    if (!scenePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for scene objects)!");
    }

    VkPushConstantRange shadowMapPushConstant;
    shadowMapPushConstant.offset = 0;
    shadowMapPushConstant.size = sizeof(ShadowMapPushConstants);
    shadowMapPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    shadowPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kShadowMapDescSetLayout)},
                                                          {shadowMapPushConstant});

    if (!shadowPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for shadow mapping)!");
    }

    VkViewport shadowMapViewport{0,    0,   static_cast<float>(kShadowMapSize), static_cast<float>(kShadowMapSize),
                                 0.0f, 1.0f};
    VkRect2D shadowMapScissor{0, 0, kShadowMapSize, kShadowMapSize};

    shadowPipeline_ = device_->CreateGraphicsPipeline(shadowPipelineLayout_, shadowRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kShadowMapVertexShaderKey)->GetHandle();
        });
        builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
            vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
        });
        builder.SetViewportState([&](auto& viewportStateCreateInfo) {
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = &shadowMapViewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = &shadowMapScissor;
        });
        builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
            rasterizationStateCreateInfo.depthBiasEnable = VK_TRUE;
            rasterizationStateCreateInfo.depthBiasConstantFactor = 0.15f;
            rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.5f;
            rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 0;
            blendStateCreateInfo.pAttachments = nullptr;
        });
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!shadowPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for shadow mapping)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Shadow map framebuffer
    const auto& shadowDepthImageView = resources_->GetImageView(kShadowMapImage, kShadowMapImageView);
    shadowFramebuffer_ = device_->CreateFramebuffer(shadowRenderPass_, {shadowDepthImageView}, [&](auto& builder) {
        builder.SetDimensions(kShadowMapSize, kShadowMapSize);
    });

    if (!shadowFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for shadow mapping)!");
    }

    // Present framebuffers
    const auto& depthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer =
                device_->CreateFramebuffer(sceneRenderPass_, {swapImage, depthImageView}, [&](auto& builder) {
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
    VkClearValue shadowMapClearValue{};
    shadowMapClearValue.depthStencil = {1.0f, 0};

    std::array<VkClearValue, 2> sceneClearValues{};
    sceneClearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    sceneClearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Create shadow map pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = shadowRenderPass_->GetHandle();
                    beginInfo.framebuffer = shadowFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(kShadowMapSize, kShadowMapSize);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &shadowMapClearValue;
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kShadowMapDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        currentCmdBuffer->BindPipeline(shadowPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable()) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                ShadowMapPushConstants shadowMapPushConstant{};
                shadowMapPushConstant.objectId = sceneObject.GetObjectId();

                const glm::mat4 lightProjection = lightCamera_->GetProjectionMatrix();
                const glm::mat4 lightView = lightCamera_->GetLightViewMatrix(kLightDirection, glm::vec3(0.0f));
                shadowMapPushConstant.lightSpaceMatrix = lightProjection * lightView;
                currentCmdBuffer->PushConstants(shadowPipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                sizeof(shadowMapPushConstant), &shadowMapPushConstant);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // Draw scene pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = sceneRenderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = sceneClearValues.size();
                    beginInfo.pClearValues = sceneClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable()) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                ScenePushConstants scenePushConstant{};
                scenePushConstant.objectId = sceneObject.GetObjectId();
                scenePushConstant.view = camera_->GetViewMatrix();
                scenePushConstant.projection = camera_->GetProjectionMatrix();
                scenePushConstant.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
                scenePushConstant.pcfKernelSize = currentKernelSize_;
                currentCmdBuffer->PushConstants(pipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(scenePushConstant), &scenePushConstant);
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
    const glm::mat4 lightProjection = lightCamera_->GetProjectionMatrix();
    const glm::mat4 lightView = lightCamera_->GetLightViewMatrix(kLightDirection, glm::vec3(0.0f));

    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
    lightUbo.lightColor = glm::vec4(kLightColor, 1.0f);
    lightUbo.lightSpaceMatrix = lightProjection * lightView;
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));
}

void VulkanApplication::ProcessInput()
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

    // Set PCF kernel size via numerical keys
    if (window_->IsKeyPressed(GLFW_KEY_0)) {
        currentKernelSize_ = 1; // 1x1 kernel
    }
    if (window_->IsKeyPressed(GLFW_KEY_1)) {
        currentKernelSize_ = 3; // 3x3 kernel
    }
    if (window_->IsKeyPressed(GLFW_KEY_2)) {
        currentKernelSize_ = 5; // 5x5 kernel
    }
    if (window_->IsKeyPressed(GLFW_KEY_3)) {
        currentKernelSize_ = 7; // 7x7 kernel
    }
    if (window_->IsKeyPressed(GLFW_KEY_4)) {
        currentKernelSize_ = 9; // 9x9 kernel
    }
}
} // namespace examples::real_time_shadows::shadow_filtering_and_softening::percentage_closer_filtering
