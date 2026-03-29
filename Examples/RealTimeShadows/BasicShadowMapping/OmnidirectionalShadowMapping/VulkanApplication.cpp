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
#include "TimeUtils.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_shadows::basic_shadow_mapping::omnidirectional_shadow_mapping
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationBasicShadowMapping(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationBasicShadowMapping::Init()) {
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
    ApplicationBasicShadowMapping::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
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
    const auto lightObjectsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kLightObjectsFragmentShaderFile);
    const auto shadowMapVertexShaderAsset = assetManager_->Load<ShaderAsset>(kShadowMapVertexShaderFile);
    const auto shadowMapFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kShadowMapFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
            {.name = kSceneFragmentShaderKey, .asset = assetManager_->Get(sceneFragmentShaderAsset)},
            {.name = kLightObjectsFragmentShaderKey, .asset = assetManager_->Get(lightObjectsFragmentShaderAsset)},
            {.name = kShadowMapVertexShaderKey, .asset = assetManager_->Get(shadowMapVertexShaderAsset)},
            {.name = kShadowMapFragmentShaderKey, .asset = assetManager_->Get(shadowMapFragmentShaderAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kShadowCubemapImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
            .format = depthImageFormat_,
            .dimensions = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1},
            .arrayLayers = 6,
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views =
                    {
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageView,
                                            .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 6}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewRight,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewLeft,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 1, 1}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewTop,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 2, 1}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewBottom,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 3, 1}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewBack,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 4, 1}},
                        ImageViewCreateInfo{.viewName = kShadowCubemapImageViewFront,
                                            .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                            .format = depthImageFormat_,
                                            .subresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 5, 1}},
                    }},
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
    sceneConfig.attributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TEXCOORD, AccessorType::VEC2);
    sceneConfig.attributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TANGENT, AccessorType::VEC4);
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 5.0f), aspectRatio);

    // Add camera for spotlight shadows
    lightCamera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f), 1.0f, 90.0f, 0.1f, kPointLightFarPlane);

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
    objectMaterial.ambientStrength = GetParamFloat(AppSettings::AmbientStrength);
    objectMaterial.shininess = GetParamFloat(AppSettings::Shininess);
    objectMaterial.specularStrength = GetParamFloat(AppSettings::SpecularStrength);
    objectMaterial.uvScale = 2.0f;
    objectMaterial.diffuseMap = woodFloorTextureId;
    objectMaterial.normalMap = woodFloorNormalTextureId;
    objectMaterial.flipNormals = 0;

    Material roomMaterial;
    roomMaterial.ambientStrength = GetParamFloat(AppSettings::AmbientStrength);
    roomMaterial.shininess = GetParamFloat(AppSettings::Shininess);
    roomMaterial.specularStrength = GetParamFloat(AppSettings::SpecularStrength);
    roomMaterial.uvScale = 10.0f;
    roomMaterial.diffuseMap = terracottaTextureId;
    roomMaterial.normalMap = terracottaNormalTextureId;
    roomMaterial.flipNormals = 1;

    int index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);
    for (const auto& pos: kObjectPositions) {
        BuiltinMeshType currentMeshType;
        switch (const std::uint32_t objectTypeValue = GenerateRandomValue(0U, 3U); objectTypeValue) {
            case 0U:
                currentMeshType = BuiltinMeshType::CUBE;
                break;
            case 1U:
                currentMeshType = BuiltinMeshType::CONE;
                break;
            case 2U:
                currentMeshType = BuiltinMeshType::SPHERE;
                break;
            default:
                currentMeshType = BuiltinMeshType::CYLINDER;
        }

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kObject + std::to_string(index++))
                                           .WithBuiltinMesh(currentMeshType)
                                           .WithMaterial(objectMaterial)
                                           .WithPosition(pos));
    }

    const auto& rootObject = rootObjectBuilder
                                     .AddChild(SceneObjectBuilder(*scene_, kRoomObject)
                                                       .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                                       .WithMaterial(roomMaterial)
                                                       .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                                       .WithScale(glm::vec3{20.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kLightObject)
                                                       .WithTag(kLightGroup)
                                                       .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                                       .WithMaterial(Material{})
                                                       .WithPosition(glm::vec3(0.0f))
                                                       .WithScale(glm::vec3{0.3f}))
                                     .Build();

    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 4 + combinedImageSamplerCount + 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
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
    shadowMapImageInfos.emplace_back(
            resources_->GetSampler(kShadowSampler)->GetHandle(),
            resources_->GetImageView(kShadowCubemapImage, kShadowCubemapImageView)->GetHandle(),
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

    lightPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, sceneRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kLightObjectsFragmentShaderKey)->GetHandle();
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

    if (!lightPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for lights)!");
    }

    VkPushConstantRange shadowMapPushConstant;
    shadowMapPushConstant.offset = 0;
    shadowMapPushConstant.size = sizeof(ShadowMapPushConstants);
    shadowMapPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    shadowPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kShadowMapDescSetLayout)},
                                                          {shadowMapPushConstant});

    if (!shadowPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for shadow mapping)!");
    }

    VkViewport shadowMapViewport{0,    0,   static_cast<float>(SHADOW_MAP_SIZE), static_cast<float>(SHADOW_MAP_SIZE),
                                 0.0f, 1.0f};
    VkRect2D shadowMapScissor{0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};

    shadowPipeline_ = device_->CreateGraphicsPipeline(shadowPipelineLayout_, shadowRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kShadowMapVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kShadowMapFragmentShaderKey)->GetHandle();
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
    const std::vector<std::string> cubemapImageViewKeys = {kShadowCubemapImageViewRight, kShadowCubemapImageViewLeft,
                                                           kShadowCubemapImageViewTop,   kShadowCubemapImageViewBottom,
                                                           kShadowCubemapImageViewBack,  kShadowCubemapImageViewFront};

    for (auto i = 0U; i < cubemapImageViewKeys.size(); ++i) {
        const auto& shadowCubemapImageView = resources_->GetImageView(kShadowCubemapImage, cubemapImageViewKeys[i]);

        shadowFramebuffers_[i] =
                device_->CreateFramebuffer(shadowRenderPass_, {shadowCubemapImageView}, [&](auto& builder) {
                    builder.SetDimensions(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
                });

        if (!shadowFramebuffers_[i]) {
            throw std::runtime_error("Failed to create framebuffer (for shadow mapping)s!");
        }
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

    const auto cubemapViewMatrices = lightCamera_->GetCubemapViewMatrices();
    auto cubemapProj = lightCamera_->GetProjectionMatrix();
    cubemapProj[1][1] *= -1; // Revert this projection flip for shadow map render

    // Create shadow map (cubemap) pass
    for (auto i = 0U; i < shadowFramebuffers_.size(); ++i) {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = shadowRenderPass_->GetHandle();
                    beginInfo.framebuffer = shadowFramebuffers_[i]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &shadowMapClearValue;
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kShadowMapDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        currentCmdBuffer->BindPipeline(shadowPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable() && sceneObject.GetTag() != kLightGroup) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                ShadowMapPushConstants shadowMapPushConstant{};
                shadowMapPushConstant.objectId = sceneObject.GetObjectId();

                const glm::mat4 lightProjection = cubemapProj;
                const glm::mat4 lightView = cubemapViewMatrices[i];
                shadowMapPushConstant.lightSpaceMatrix = lightProjection * lightView;
                shadowMapPushConstant.lightPos = glm::vec4(scene_->FindObjectByName(kLightObject)->GetPosition(), 1.0f);
                shadowMapPushConstant.farPlane = kPointLightFarPlane;
                currentCmdBuffer->PushConstants(shadowPipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
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
                if (sceneObject.GetTag() == kLightGroup) {
                    currentCmdBuffer->BindPipeline(lightPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
                }
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                ScenePushConstants scenePushConstant{};
                scenePushConstant.objectId = sceneObject.GetObjectId();
                scenePushConstant.view = camera_->GetViewMatrix();
                scenePushConstant.projection = camera_->GetProjectionMatrix();
                scenePushConstant.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
                scenePushConstant.farPlane = kPointLightFarPlane;
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
    const auto time = static_cast<float>(GetCurrentTime());
    constexpr float speed = 0.5f;
    constexpr float radius = 2.0f;
    constexpr auto center = glm::vec3(0.0f, 0.0f, 0.0f);
    const float angle = time * speed;

    glm::vec3 pos;
    pos.x = center.x + radius * std::cos(angle);
    pos.y = center.y;
    pos.z = center.z + radius * std::sin(angle);

    scene_->FindObjectByName(kLightObject)->SetPosition(pos);

    lightCamera_->SetPosition(pos);

    LightUbo lightUbo{};
    lightUbo.lightPosition = glm::vec4(pos, 1.0f);
    lightUbo.lightColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightColor), 1.0f);
    lightUbo.pointLightParams.x = GetParamFloat(AppSettings::ConstantFactor);
    lightUbo.pointLightParams.y = GetParamFloat(AppSettings::LinearFactor);
    lightUbo.pointLightParams.z = GetParamFloat(AppSettings::QuadraticFactor);
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));
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
} // namespace examples::real_time_shadows::basic_shadow_mapping::omnidirectional_shadow_mapping
