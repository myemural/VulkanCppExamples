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

namespace examples::physically_based_rendering::area_lights::multiple_area_lights
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

namespace
{
    std::array<glm::vec4, 4> ComputePlaneCornersInWorldSpace(const glm::mat4& planeModelMatrix)
    {
        return {glm::vec4(planeModelMatrix * glm::vec4(-0.5f, 0.0f, -0.5f, 1.0f)),
                glm::vec4(planeModelMatrix * glm::vec4(0.5f, 0.0f, -0.5f, 1.0f)),
                glm::vec4(planeModelMatrix * glm::vec4(0.5f, 0.0f, 0.5f, 1.0f)),
                glm::vec4(planeModelMatrix * glm::vec4(-0.5f, 0.0f, 0.5f, 1.0f))};
    }
} // namespace

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationAreaLights(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationAreaLights::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        UploadLtcResourcesToGpu();
        BuildScene();
        CreateAndUpdateDescriptorSets();

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
        CreateDefaultFramebuffers(resources_->GetImageView(kDepthImage, kDepthImageView));
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
    ApplicationAreaLights::PreUpdate();

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
    resourceCreateInfo.buffers = {{kLightStorageBuffer, sizeof(LightBuffer), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto sceneObjectsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneObjectsFragmentShaderFile);
    const auto lightObjectsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kLightObjectsFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
            {.name = kSceneObjectsFragmentShaderKey, .asset = assetManager_->Get(sceneObjectsFragmentShaderAsset)},
            {.name = kLightObjectsFragmentShaderKey, .asset = assetManager_->Get(lightObjectsFragmentShaderAsset)}}};

    resourceCreateInfo.images = {
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
                                                               .layerCount = 1}}}},
        ImageResourceCreateInfo{
            .name = kLtc1Image,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .dimensions = {LTC_LUT_IMAGE_WIDTH, LTC_LUT_IMAGE_HEIGHT, 1},
            .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kLtc1ImageView, .format = VK_FORMAT_R32G32B32A32_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kLtc2Image,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .dimensions = {LTC_LUT_IMAGE_WIDTH, LTC_LUT_IMAGE_HEIGHT, 1},
            .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .views = {ImageViewCreateInfo{.viewName = kLtc2ImageView, .format = VK_FORMAT_R32G32B32A32_SFLOAT}}}};

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::UploadLtcResourcesToGpu() const
{
    TextureAsset ltcAsset;
    ltcAsset.data.resize(sizeof(kLtc1LutData));
    ltcAsset.width = LTC_LUT_IMAGE_WIDTH;
    ltcAsset.height = LTC_LUT_IMAGE_HEIGHT;
    ltcAsset.channels = LTC_LUT_IMAGE_CHANNELS;

    // Upload LTC1 and LTC2
    std::memcpy(ltcAsset.data.data(), kLtc1LutData, sizeof(kLtc1LutData));
    resources_->SetImageFromTexture(cmdPool_, queue_, kLtc1Image, ltcAsset);
    std::memcpy(ltcAsset.data.data(), kLtc2LutData, sizeof(kLtc2LutData));
    resources_->SetImageFromTexture(cmdPool_, queue_, kLtc2Image, ltcAsset);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;
    sceneConfig.primitiveStackCount = 64U;
    sceneConfig.primitiveSectorCount = 64U;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 11.0f), aspectRatio);

    // Materials
    const auto metalDamagedAlbedoTextureAsset = assetManager_->Load<TextureAsset>(kMetalDamagedAlbedoTexturePath);
    const auto metalDamagedAlbedoTextureId = sceneImageStorage.StoreTexture(
            kMetalDamagedAlbedoTexture, kMainSampler, assetManager_->Get(metalDamagedAlbedoTextureAsset));
    const auto metalDamagedRoughnessTextureAsset = assetManager_->Load<TextureAsset>(kMetalDamagedRoughnessTexturePath);
    const auto metalDamagedRoughnessTextureId = sceneImageStorage.StoreTexture(
            kMetalDamagedRoughnessTexture, kMainSampler, assetManager_->Get(metalDamagedRoughnessTextureAsset),
            VK_FORMAT_R8G8B8A8_UNORM);
    const auto metalDamagedMetallicTextureAsset = assetManager_->Load<TextureAsset>(kMetalDamagedMetallicTexturePath);
    const auto metalDamagedMetallicTextureId = sceneImageStorage.StoreTexture(
            kMetalDamagedMetallicTexture, kMainSampler, assetManager_->Get(metalDamagedMetallicTextureAsset),
            VK_FORMAT_R8G8B8A8_UNORM);
    const auto metalDamagedNormalTextureAsset = assetManager_->Load<TextureAsset>(kMetalDamagedNormalTexturePath);
    const auto metalDamagedNormalTextureId = sceneImageStorage.StoreTexture(
            kMetalDamagedNormalTexture, kMainSampler, assetManager_->Get(metalDamagedNormalTextureAsset),
            VK_FORMAT_R8G8B8A8_UNORM);

    const auto tilesAlbedoTextureAsset = assetManager_->Load<TextureAsset>(kTilesAlbedoTexturePath);
    const auto tilesAlbedoTextureId = sceneImageStorage.StoreTexture(
            kTilesAlbedoTexture, kMainSampler, assetManager_->Get(tilesAlbedoTextureAsset));
    const auto tilesRoughnessTextureAsset = assetManager_->Load<TextureAsset>(kTilesRoughnessTexturePath);
    const auto tilesRoughnessTextureId = sceneImageStorage.StoreTexture(
            kTilesRoughnessTexture, kMainSampler, assetManager_->Get(tilesRoughnessTextureAsset),
            VK_FORMAT_R8G8B8A8_UNORM);
    const auto tilesNormalTextureAsset = assetManager_->Load<TextureAsset>(kTilesNormalTexturePath);
    const auto tilesNormalTextureId = sceneImageStorage.StoreTexture(
            kTilesNormalTexture, kMainSampler, assetManager_->Get(tilesNormalTextureAsset),
            VK_FORMAT_R8G8B8A8_UNORM);

    Material floorMaterial;
    floorMaterial.metallic = 0.0f;
    floorMaterial.uvScale = 10.0f;
    floorMaterial.albedoMap = tilesAlbedoTextureId;
    floorMaterial.roughnessMap = tilesRoughnessTextureId;
    floorMaterial.normalMap = tilesNormalTextureId;

    Material metalDamagedMaterial;
    metalDamagedMaterial.uvScale = 2.0f;
    metalDamagedMaterial.albedoMap = metalDamagedAlbedoTextureId;
    metalDamagedMaterial.roughnessMap = metalDamagedRoughnessTextureId;
    metalDamagedMaterial.metallicMap = metalDamagedMetallicTextureId;
    metalDamagedMaterial.normalMap = metalDamagedNormalTextureId;

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject)
                                     .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                     .AddChild(SceneObjectBuilder(*scene_, kCubeObject1)
                                                       .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                                       .WithMaterial(metalDamagedMaterial)
                                                       .WithPosition(glm::vec3{-3.0f, -1.0f, -3.0f})
                                                       .WithScale(glm::vec3{2.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kCubeObject2)
                                                       .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                                       .WithMaterial(metalDamagedMaterial)
                                                       .WithPosition(glm::vec3{-3.0f, -1.0f, 3.0f})
                                                       .WithScale(glm::vec3{2.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSphereObject1)
                                                       .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                                       .WithMaterial(metalDamagedMaterial)
                                                       .WithPosition(glm::vec3{3.0f, -1.0f, -3.0f})
                                                       .WithScale(glm::vec3{2.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSphereObject2)
                                                       .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                                       .WithMaterial(metalDamagedMaterial)
                                                       .WithPosition(glm::vec3{3.0f, -1.0f, 3.0f})
                                                       .WithScale(glm::vec3{2.0f}))
                                     .AddChild(SceneObjectBuilder(*scene_, kFloorObject)
                                                       .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                       .WithMaterial(floorMaterial)
                                                       .WithPosition(glm::vec3{0.0f, -2.0f, 0.0f})
                                                       .WithScale(glm::vec3{25.0f}));

    for (const auto& lightMeshInfo: areaLightMeshInfo) {
        BuiltinMeshType currentMeshType = lightMeshInfo.type == AreaLightType::RECTANGULAR_AREA_LIGHT
                                                  ? BuiltinMeshType::PLANE
                                                  : BuiltinMeshType::SPHERE;

        Material areaLightMaterial;
        areaLightMaterial.albedoColor = glm::vec4(lightMeshInfo.color, 1.0f);

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, lightMeshInfo.name)
                                           .WithBuiltinMesh(currentMeshType)
                                           .WithTag(kLightGroup)
                                           .WithMaterial(areaLightMaterial)
                                           .WithPosition(lightMeshInfo.position)
                                           .WithEulerAngles(lightMeshInfo.rotation)
                                           .WithScale(lightMeshInfo.scale));
    }

    scene_->AddRootObject(rootObjectBuilder.Build());
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3 + combinedImageSamplerCount + 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 2}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightStorageBufferInfos;
    lightStorageBufferInfos.emplace_back(resources_->GetBuffer(kLightStorageBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

    std::vector<VkDescriptorImageInfo> ltc1ImageInfos;
    ltc1ImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                resources_->GetImageView(kLtc1Image, kLtc1ImageView)->GetHandle(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> ltc2ImageInfos;
    ltc2ImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                resources_->GetImageView(kLtc2Image, kLtc2ImageView)->GetHandle(),
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
    lightUboRequest.buffers = lightStorageBufferInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = kMainDescSet;
    textureUpdateRequest.bindingIndex = 3;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest textureLut1UpdateRequest;
    textureLut1UpdateRequest.descriptorSetName = kMainDescSet;
    textureLut1UpdateRequest.bindingIndex = 4;
    textureLut1UpdateRequest.images = ltc1ImageInfos;
    textureLut1UpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest textureLut2UpdateRequest;
    textureLut2UpdateRequest.descriptorSetName = kMainDescSet;
    textureLut2UpdateRequest.bindingIndex = 5;
    textureLut2UpdateRequest.images = ltc2ImageInfos;
    textureLut2UpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest},
        .imageWriteRequests = {textureUpdateRequest, textureLut1UpdateRequest, textureLut2UpdateRequest}};

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
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MeshPushConstants);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {mvpPushConstant});

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

    scenePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneObjectsFragmentShaderKey)->GetHandle();
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

    lightObjectPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
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

    if (!lightObjectPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for light objects)!");
    }
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

    const std::vector mainDescSets{resources_->GetDescriptorSet(kMainDescSet)};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, mainDescSets);
    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable()) {
            if (sceneObject.GetTag() == kLightGroup) {
                currentCmdBuffer->BindPipeline(lightObjectPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
            }
            const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            MeshPushConstants meshPushConstants{};
            meshPushConstants.objectId = sceneObject.GetObjectId();
            meshPushConstants.view = camera_->GetViewMatrix();
            meshPushConstants.projection = camera_->GetProjectionMatrix();
            meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            0, sizeof(meshPushConstants), &meshPushConstants);
            currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
        }
    });

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    LightBuffer lightBuffer{};
    std::uint32_t currentRectangularAreaLightIndex = 0;
    std::uint32_t currentSphereAreaLightIndex = 0;

    for (const auto& lightMeshInfo: areaLightMeshInfo) {
        if (lightMeshInfo.type == AreaLightType::RECTANGULAR_AREA_LIGHT) {
            const auto& lightRectObject = scene_->FindObjectByName(lightMeshInfo.name);
            const auto& lightWorldMat = lightRectObject->GetWorldMatrix();
            const auto areaLightRectCorners = ComputePlaneCornersInWorldSpace(lightWorldMat);

            RectangularAreaLightData areaLightData;
            areaLightData.lightColorAndIntensity = glm::vec4(lightMeshInfo.color, lightMeshInfo.intensity);
            areaLightData.lightRectCornerPos = areaLightRectCorners;
            areaLightData.isDoubleSided = lightMeshInfo.isDoubleSided;

            lightBuffer.rectangularAreaLights[currentRectangularAreaLightIndex++] = areaLightData;
        } else if (lightMeshInfo.type == AreaLightType::SPHERE_AREA_LIGHT) {
            SphereAreaLightData areaLightData{};
            areaLightData.lightColorAndIntensity = glm::vec4(lightMeshInfo.color, lightMeshInfo.intensity);
            areaLightData.lightPositionAndRadius = glm::vec4(lightMeshInfo.position, lightMeshInfo.scale.x / 2.0f);

            lightBuffer.sphereAreaLights[currentSphereAreaLightIndex++] = areaLightData;
        }
    }

    // Add one directional light
    lightBuffer.directionalLights[0].lightDirection =
            glm::vec4(params_.Get<glm::vec3>(AppSettings::DirectionalLightDirection), 1.0f);
    lightBuffer.directionalLights[0].lightColorAndIntensity =
            glm::vec4(params_.Get<glm::vec3>(AppSettings::DirectionalLightColor),
                      GetParamFloat(AppSettings::DirectionalLightIntensity));

    resources_->SetBuffer(kLightStorageBuffer, &lightBuffer, sizeof(lightBuffer));
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
} // namespace examples::physically_based_rendering::area_lights::multiple_area_lights
