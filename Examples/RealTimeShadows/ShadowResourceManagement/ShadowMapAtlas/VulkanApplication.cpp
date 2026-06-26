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

namespace examples::real_time_shadows::shadow_resource_management::shadow_map_atlas
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationShadowResourceManagement(std::move(params))
{
}

bool VulkanApplication::Init()
{
    if (!ApplicationShadowResourceManagement::Init()) {
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
    ApplicationShadowResourceManagement::PreUpdate();

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
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUboGpu), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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
            .name = kShadowMapAtlasImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = depthImageFormat_,
            .dimensions = {kShadowMapAtlasSize, kShadowMapAtlasSize, 1},
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kShadowMapAtlasImageView,
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
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 14.0f), aspectRatio);

    // Add camera for shadows
    constexpr auto kSpotLightCameraAngleMargin = 1.5f;
    constexpr auto kSpotLightCameraFov = (kOuterCutoffAngle + kSpotLightCameraAngleMargin) * 2.0f;
    lightCameras_.push_back(
            std::make_shared<OrthographicCamera>(glm::vec3(0.0f), 1.0f, 40.0f, 0.1f, kDirectionalLightFarPlane));
    lightCameras_.push_back(std::make_shared<PerspectiveCamera>(kSpotlight1InitPosition, 1.0f, kSpotLightCameraFov));
    lightCameras_.push_back(std::make_shared<PerspectiveCamera>(kSpotlight2InitPosition, 1.0f, kSpotLightCameraFov));
    lightCameras_.push_back(std::make_shared<PerspectiveCamera>(kSpotlight3InitPosition, 1.0f, kSpotLightCameraFov));

    // Rotate only for spotlights
    lightCameras_[1]->Rotate(0.0f, -90.0f);
    lightCameras_[2]->Rotate(0.0f, -90.0f);
    lightCameras_[3]->Rotate(0.0f, -90.0f);

    // Materials
    const auto woodFloorTextureAsset = assetManager_->Load<TextureAsset>(kWoodFloorTexturePath);
    const auto woodFloorTextureId =
            sceneImageStorage.StoreTexture(kWoodFloorTexture, kMainSampler, assetManager_->Get(woodFloorTextureAsset));
    const auto woodFloorNormalTextureAsset = assetManager_->Load<TextureAsset>(kWoodFloorNormalTexturePath);
    const auto woodFloorNormalTextureId =
            sceneImageStorage.StoreTexture(kWoodFloorNormalTexture, kMainSampler,
                                           assetManager_->Get(woodFloorNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    Material objectMaterial;
    objectMaterial.ambientStrength = kAmbientStrength;
    objectMaterial.shininess = kSpecularShininess;
    objectMaterial.specularStrength = kSpecularStrength;
    objectMaterial.uvScale = 2.0f;
    objectMaterial.diffuseMap = woodFloorTextureId;
    objectMaterial.normalMap = woodFloorNormalTextureId;

    Material planeMaterial;
    planeMaterial.diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 0.8f);
    planeMaterial.ambientStrength = kAmbientStrength;
    planeMaterial.shininess = kSpecularShininess;
    planeMaterial.specularStrength = kSpecularStrength;

    int index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);
    for (auto i = 0; i < 8; ++i) {
        const std::string rowStr = std::to_string(i);
        const auto zShift = -static_cast<float>(i * 2 - 1) + 7.0f;
        rootObjectBuilder
                .AddChild(SceneObjectBuilder(*scene_, kObject + std::to_string(index++))
                                  .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                  .WithMaterial(objectMaterial)
                                  .WithPosition(glm::vec3{-5.0f, -1.0f, zShift}))
                .AddChild(SceneObjectBuilder(*scene_, kObject + std::to_string(index++))
                                  .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                  .WithMaterial(objectMaterial)
                                  .WithPosition(glm::vec3{0.0f, -1.0f, zShift}))
                .AddChild(SceneObjectBuilder(*scene_, kObject + std::to_string(index++))
                                  .WithBuiltinMesh(BuiltinMeshType::CONE)
                                  .WithMaterial(objectMaterial)
                                  .WithPosition(glm::vec3{5.0f, -1.0f, zShift}));
    }

    const auto& rootObject = rootObjectBuilder
                                     .AddChild(SceneObjectBuilder(*scene_, kFloorObject)
                                                       .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                       .WithMaterial(planeMaterial)
                                                       .WithPosition(glm::vec3{0.0f, -2.5f, 0.0f})
                                                       .WithScale(glm::vec3{24.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSpotLightObject1)
                                                       .WithTag(kLightGroup)
                                                       .WithBuiltinMesh(BuiltinMeshType::CONE)
                                                       .WithMaterial(Material{})
                                                       .WithPosition(kSpotlight1InitPosition)
                                                       .WithScale(glm::vec3{0.3f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSpotLightObject2)
                                                       .WithTag(kLightGroup)
                                                       .WithBuiltinMesh(BuiltinMeshType::CONE)
                                                       .WithMaterial(Material{})
                                                       .WithPosition(kSpotlight2InitPosition)
                                                       .WithScale(glm::vec3{0.3f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSpotLightObject3)
                                                       .WithTag(kLightGroup)
                                                       .WithBuiltinMesh(BuiltinMeshType::CONE)
                                                       .WithMaterial(Material{})
                                                       .WithPosition(kSpotlight3InitPosition)
                                                       .WithScale(glm::vec3{0.3f}))
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
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 3}},
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

    std::vector<VkDescriptorImageInfo> shadowMapAtlasImageInfos;
    shadowMapAtlasImageInfos.emplace_back(
            resources_->GetSampler(kShadowSampler)->GetHandle(),
            resources_->GetImageView(kShadowMapAtlasImage, kShadowMapAtlasImageView)->GetHandle(),
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

    ImageWriteRequest shadowMapAtlasTextureUpdateRequest;
    shadowMapAtlasTextureUpdateRequest.descriptorSetName = kMainDescSet;
    shadowMapAtlasTextureUpdateRequest.bindingIndex = 4;
    shadowMapAtlasTextureUpdateRequest.images = shadowMapAtlasImageInfos;
    shadowMapAtlasTextureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest shadowMapTransformBufferRequest;
    shadowMapTransformBufferRequest.descriptorSetName = kShadowMapDescSet;
    shadowMapTransformBufferRequest.bindingIndex = 0;
    shadowMapTransformBufferRequest.buffers = storageTransformBufferInfos;
    shadowMapTransformBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest, shadowMapTransformBufferRequest},
        .imageWriteRequests = {textureUpdateRequest, shadowMapAtlasTextureUpdateRequest}};

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

    std::vector shadowMapDynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

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
            viewportStateCreateInfo.pViewports = nullptr;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = nullptr;
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
        builder.SetDynamicState([&](auto& dynamicStateCreateInfo) {
            dynamicStateCreateInfo.dynamicStateCount = shadowMapDynamicStates.size();
            dynamicStateCreateInfo.pDynamicStates = shadowMapDynamicStates.data();
        });
    });

    if (!shadowPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for shadow mapping)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Shadow map framebuffer
    const auto& shadowAtlasImageImageView = resources_->GetImageView(kShadowMapAtlasImage, kShadowMapAtlasImageView);
    shadowMapFramebuffer_ =
            device_->CreateFramebuffer(shadowRenderPass_, {shadowAtlasImageImageView}, [&](auto& builder) {
                builder.SetDimensions(kShadowMapAtlasSize, kShadowMapAtlasSize);
            });

    if (!shadowMapFramebuffer_) {
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
                    beginInfo.framebuffer = shadowMapFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(kShadowMapAtlasSize, kShadowMapAtlasSize);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &shadowMapClearValue;
                },
                VK_SUBPASS_CONTENTS_INLINE);


        const std::vector descSets{resources_->GetDescriptorSet(kShadowMapDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        currentCmdBuffer->BindPipeline(shadowPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        for (auto i = 0U; i < kLightProperties.size(); ++i) {
            const auto& shadowMapRect = kLightProperties.at(i).shadowMapAtlasRect;

            VkViewport shadowMapViewport{
                shadowMapRect.x, shadowMapRect.y, shadowMapRect.z, shadowMapRect.w, 0.0f, 1.0f};
            VkRect2D shadowMapScissor{static_cast<int32_t>(shadowMapRect.x), static_cast<int32_t>(shadowMapRect.y),
                                      static_cast<uint32_t>(shadowMapRect.z), static_cast<uint32_t>(shadowMapRect.w)};
            currentCmdBuffer->SetViewports(0, {shadowMapViewport});
            currentCmdBuffer->SetScissors(0, {shadowMapScissor});

            scene_->Traverse([&](const SceneObject& sceneObject) {
                if (sceneObject.HasRenderable() && sceneObject.GetTag() != kLightGroup) {
                    const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                    currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                    currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                    ShadowMapPushConstants shadowMapPushConstant{};
                    shadowMapPushConstant.objectId = sceneObject.GetObjectId();

                    const glm::mat4 lightProjection = lightCameras_.at(i)->GetProjectionMatrix();
                    glm::mat4 lightView;
                    if (kLightProperties.at(i).type == LIGHT_TYPE_DIRECTIONAL) {
                        lightView =
                                lightCameras_.at(i)->GetLightViewMatrix(kDirectionalLightDirection, glm::vec3(0.0f));
                    } else {
                        lightView = lightCameras_.at(i)->GetViewMatrix();
                    }

                    shadowMapPushConstant.lightSpaceMatrix = lightProjection * lightView;
                    currentCmdBuffer->PushConstants(shadowPipelineLayout_,
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                    sizeof(shadowMapPushConstant), &shadowMapPushConstant);
                    currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
                }
            });
        }

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
    // Move spotlight objects and their cameras
    const auto time = static_cast<float>(GetCurrentTime());
    constexpr float speed = 0.5f;
    constexpr float range = 7.0f;

    const glm::vec3 posSpotLight1{kSpotlight1InitPosition.x, kSpotlight1InitPosition.y, std::sin(time * speed) * range};
    scene_->FindObjectByName(kSpotLightObject1)->SetPosition(posSpotLight1);
    lightCameras_[1]->SetPosition(posSpotLight1);

    const glm::vec3 posSpotLight2{kSpotlight2InitPosition.x, kSpotlight2InitPosition.y, std::sin(time * speed) * range};
    scene_->FindObjectByName(kSpotLightObject2)->SetPosition(posSpotLight2);
    lightCameras_[2]->SetPosition(posSpotLight2);

    const glm::vec3 posSpotLight3{kSpotlight3InitPosition.x, kSpotlight3InitPosition.y, std::sin(time * speed) * range};
    scene_->FindObjectByName(kSpotLightObject3)->SetPosition(posSpotLight3);
    lightCameras_[3]->SetPosition(posSpotLight3);

    // Fill light data for GPU
    LightUboGpu lightUbo{};

    // Directional light
    const glm::mat4 directionalLightProj = lightCameras_[0]->GetProjectionMatrix();
    const glm::mat4 directionalLightView =
            lightCameras_[0]->GetLightViewMatrix(kDirectionalLightDirection, glm::vec3(0.0f));

    lightUbo.lightData[0].lightSpaceMatrix = directionalLightProj * directionalLightView;
    lightUbo.lightData[0].lightDirection = glm::vec4(kDirectionalLightDirection, 1.0f);
    lightUbo.lightData[0].lightColor = glm::vec4(kLightColor, 1.0f);
    lightUbo.lightData[0].lightParams.x = static_cast<float>(LIGHT_TYPE_DIRECTIONAL);
    lightUbo.lightData[0].shadowMapAtlasRect =
            kLightProperties[0].shadowMapAtlasRect / static_cast<float>(kShadowMapAtlasSize);

    // Spotlight 1
    const glm::mat4 spotLightProj1 = lightCameras_[1]->GetProjectionMatrix();
    const glm::mat4 spotLightView1 = lightCameras_[1]->GetViewMatrix();

    lightUbo.lightData[1].lightSpaceMatrix = spotLightProj1 * spotLightView1;
    lightUbo.lightData[1].lightPosition = glm::vec4(scene_->FindObjectByName(kSpotLightObject1)->GetPosition(), 1.0f);
    lightUbo.lightData[1].lightDirection = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    lightUbo.lightData[1].lightColor = glm::vec4(kLightColor, 1.0f);
    lightUbo.lightData[1].lightParams.x = static_cast<float>(LIGHT_TYPE_SPOT);
    lightUbo.lightData[1].lightParams.y = std::cos(glm::radians(kInnerCutoffAngle));
    lightUbo.lightData[1].lightParams.z = std::cos(glm::radians(kOuterCutoffAngle));
    lightUbo.lightData[1].shadowMapAtlasRect =
            kLightProperties[1].shadowMapAtlasRect / static_cast<float>(kShadowMapAtlasSize);

    // Spotlight 2
    const glm::mat4 spotLightProj2 = lightCameras_[2]->GetProjectionMatrix();
    const glm::mat4 spotLightView2 = lightCameras_[2]->GetViewMatrix();

    lightUbo.lightData[2].lightSpaceMatrix = spotLightProj2 * spotLightView2;
    lightUbo.lightData[2].lightPosition = glm::vec4(scene_->FindObjectByName(kSpotLightObject2)->GetPosition(), 1.0f);
    lightUbo.lightData[2].lightDirection = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    lightUbo.lightData[2].lightColor = glm::vec4(kLightColor, 1.0f);
    lightUbo.lightData[2].lightParams.x = static_cast<float>(LIGHT_TYPE_SPOT);
    lightUbo.lightData[2].lightParams.y = std::cos(glm::radians(kInnerCutoffAngle));
    lightUbo.lightData[2].lightParams.z = std::cos(glm::radians(kOuterCutoffAngle));
    lightUbo.lightData[2].shadowMapAtlasRect =
            kLightProperties[2].shadowMapAtlasRect / static_cast<float>(kShadowMapAtlasSize);

    // Spotlight 3
    const glm::mat4 spotLightProj3 = lightCameras_[3]->GetProjectionMatrix();
    const glm::mat4 spotLightView3 = lightCameras_[3]->GetViewMatrix();

    lightUbo.lightData[3].lightSpaceMatrix = spotLightProj3 * spotLightView3;
    lightUbo.lightData[3].lightPosition = glm::vec4(scene_->FindObjectByName(kSpotLightObject3)->GetPosition(), 1.0f);
    lightUbo.lightData[3].lightDirection = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    lightUbo.lightData[3].lightColor = glm::vec4(kLightColor, 1.0f);
    lightUbo.lightData[3].lightParams.x = static_cast<float>(LIGHT_TYPE_SPOT);
    lightUbo.lightData[3].lightParams.y = std::cos(glm::radians(kInnerCutoffAngle));
    lightUbo.lightData[3].lightParams.z = std::cos(glm::radians(kOuterCutoffAngle));
    lightUbo.lightData[3].shadowMapAtlasRect =
            kLightProperties[3].shadowMapAtlasRect / static_cast<float>(kShadowMapAtlasSize);

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
} // namespace examples::real_time_shadows::shadow_resource_management::shadow_map_atlas
