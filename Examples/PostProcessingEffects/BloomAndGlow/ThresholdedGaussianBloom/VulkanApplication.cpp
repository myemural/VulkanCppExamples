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
#include "ModelLoader.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::post_processing_effects::bloom_and_glow::thresholded_gaussian_bloom
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationBloomAndGlow(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationBloomAndGlow::Init()) {
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
    ApplicationBloomAndGlow::PreUpdate();

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
    resourceCreateInfo.buffers = {{kLightStorageBuffer,
                                   static_cast<std::uint32_t>(sizeof(PointLightGpuData) * pointLights.size()),
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto geometryVertexShaderAsset = assetManager_->Load<ShaderAsset>(kGeometryVertexShaderFile);
    const auto geometryFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kGeometryFragmentShaderFile);
    const auto fullscreenVertexShaderAsset = assetManager_->Load<ShaderAsset>(kFullscreenVertexShaderFile);
    const auto lightFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kLightFragmentShaderFile);
    const auto brightPassFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kBrightPassFragmentShaderFile);
    const auto blurFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kBlurFragmentShaderFile);
    const auto compositeFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kCompositeFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kGeometryVertexShaderKey, .asset = assetManager_->Get(geometryVertexShaderAsset)},
                    {.name = kGeometryFragmentShaderKey, .asset = assetManager_->Get(geometryFragmentShaderAsset)},
                    {.name = kFullscreenVertexShaderKey, .asset = assetManager_->Get(fullscreenVertexShaderAsset)},
                    {.name = kLightFragmentShaderKey, .asset = assetManager_->Get(lightFragmentShaderAsset)},
                    {.name = kBrightPassFragmentShaderKey, .asset = assetManager_->Get(brightPassFragmentShaderAsset)},
                    {.name = kBlurFragmentShaderKey, .asset = assetManager_->Get(blurFragmentShaderAsset)},
                    {.name = kCompositeFragmentShaderKey, .asset = assetManager_->Get(compositeFragmentShaderAsset)}}};

    const VkExtent2D bloomExtent = GetBloomExtent();

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kPositionImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kPositionImageView, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kAlbedoImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kAlbedoImageView, .format = VK_FORMAT_R8G8B8A8_UNORM}}},
        ImageResourceCreateInfo{
            .name = kNormalImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kNormalImageView, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{.name = kRoughnessMetallicImage,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .format = VK_FORMAT_R8G8B8A8_UNORM,
                                .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
                                .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kRoughnessMetallicImageView,
                                                              .format = VK_FORMAT_R8G8B8A8_UNORM}}},
        ImageResourceCreateInfo{.name = kLightingOutputImage,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
                                .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kLightingOutputImageView,
                                                              .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kBloomImageA,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {bloomExtent.width, bloomExtent.height, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kBloomImageViewA, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kBloomImageB,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {bloomExtent.width, bloomExtent.height, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kBloomImageViewB, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
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
        SamplerResourceCreateInfo{.name = kLinearClampSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR},
                                  .addressModes = {.u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   .v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   .w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.primitiveStackCount = 48U;
    sceneConfig.primitiveSectorCount = 48U;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 3.5f, 11.0f), aspectRatio);

    // Register textures
    for (const auto& [textureName, texturePath, format]: textureAssets) {
        const auto asset = assetManager_->Load<TextureAsset>(texturePath);
        sceneImageStorage.StoreTexture(textureName, kMainSampler, assetManager_->Get(asset), format);
    }

    // Create and register scene materials
    for (const auto& mat: materials) {
        Material material{};
        material.albedoColor = mat.albedoColor;
        material.albedoMap = mat.albedoTextureName.empty() ? -1 : sceneImageStorage.GetTextureId(mat.albedoTextureName);
        material.metallic = mat.metallic;
        material.metallicMap =
                mat.metallicTextureName.empty() ? -1 : sceneImageStorage.GetTextureId(mat.metallicTextureName);
        material.roughness = mat.roughness;
        material.roughnessMap =
                mat.roughnessTextureName.empty() ? -1 : sceneImageStorage.GetTextureId(mat.roughnessTextureName);
        material.uvScale = mat.uvScale;
        material.normalMap = mat.normalTextureName.empty() ? -1 : sceneImageStorage.GetTextureId(mat.normalTextureName);
        materialRegistry_[mat.materialName] = material;
    }

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);

    // Room parts (floor and walls)
    std::uint32_t index = 0;
    for (const auto& [meshType, position, eulerAngles, scale, materialName]: roomParts) {
        if (!materialRegistry_.contains(materialName)) {
            throw std::runtime_error("Material not found: " + materialName);
        }

        const Material& material = materialRegistry_.at(materialName);

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kRoomObject + std::to_string(index++))
                                           .WithBuiltinMesh(meshType)
                                           .WithMaterial(material)
                                           .WithPosition(position)
                                           .WithEulerAngles(eulerAngles)
                                           .WithScale(scale));
    }

    // Basic shapes inside the room
    index = 0;
    for (const auto& [meshType, position, eulerAngles, scale, materialName]: propObjects) {
        if (!materialRegistry_.contains(materialName)) {
            throw std::runtime_error("Material not found: " + materialName);
        }

        const Material& material = materialRegistry_.at(materialName);

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kPropObject + std::to_string(index++))
                                           .WithBuiltinMesh(meshType)
                                           .WithMaterial(material)
                                           .WithPosition(position)
                                           .WithEulerAngles(eulerAngles)
                                           .WithScale(scale));
    }

    const Material& lightSphereMaterial = materialRegistry_.at(kLightObjectMaterial);
    for (auto i = 0U; i < pointLights.size(); ++i) {
        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kLightSphereObject + std::to_string(i))
                                           .WithTag(kLightObjectGroup)
                                           .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                           .WithMaterial(lightSphereMaterial)
                                           .WithPosition(pointLights[i].lightPosition)
                                           .WithScale(glm::vec3(kLightSphereRadius * 2.0f)));
    }

    const auto& rootObject = rootObjectBuilder.Build();
    scene_->AddRootObject(rootObject);

    // Assign light sphere indices after build scene
    for (auto i = 0U; i < pointLights.size(); ++i) {
        const auto lightSphereObject = scene_->FindObjectByName(kLightSphereObject + std::to_string(i));
        lightSphereIndices_[lightSphereObject->GetObjectId()] = static_cast<std::int32_t>(i);
    }
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 6,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 10}},
        .layouts =
                {{.name = kMainDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                 {.name = kLightDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                 {.name = kBrightPassDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}},
                 {.name = kBlurDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}},
                 {.name = kCompositePassDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}}},
        .descriptorSets = {
            {.name = kMainDescSet, .layoutName = kMainDescSetLayout},
            {.name = kLightDescSet, .layoutName = kLightDescSetLayout},
            {.name = kBrightPassDescSet, .layoutName = kBrightPassDescSetLayout},
            {.name = kBlurHorizontalDescSet, .layoutName = kBlurDescSetLayout},
            {.name = kBlurVerticalDescSet, .layoutName = kBlurDescSetLayout},
            {.name = kCompositePassDescSet, .layoutName = kCompositePassDescSetLayout},
        }};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageLightBufferInfos;
    storageLightBufferInfos.emplace_back(resources_->GetBuffer(kLightStorageBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

    std::vector<VkDescriptorImageInfo> positionImageInfos;
    positionImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                    resources_->GetImageView(kPositionImage, kPositionImageView)->GetHandle(),
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> albedoImageInfos;
    albedoImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                  resources_->GetImageView(kAlbedoImage, kAlbedoImageView)->GetHandle(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> normalImageInfos;
    normalImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                  resources_->GetImageView(kNormalImage, kNormalImageView)->GetHandle(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> roughnessMetallicImageInfos;
    roughnessMetallicImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kRoughnessMetallicImage, kRoughnessMetallicImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> lightingOutputImageInfos;
    lightingOutputImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kLightingOutputImage, kLightingOutputImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> emissiveMaskImageInfos;
    emissiveMaskImageInfos.emplace_back(
            resources_->GetSampler(kLinearClampSampler)->GetHandle(),
            resources_->GetImageView(kRoughnessMetallicImage, kRoughnessMetallicImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> bloomImageImageInfosA;
    bloomImageImageInfosA.emplace_back(resources_->GetSampler(kLinearClampSampler)->GetHandle(),
                                       resources_->GetImageView(kBloomImageA, kBloomImageViewA)->GetHandle(),
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> bloomImageImageInfosB;
    bloomImageImageInfosB.emplace_back(resources_->GetSampler(kLinearClampSampler)->GetHandle(),
                                       resources_->GetImageView(kBloomImageB, kBloomImageViewB)->GetHandle(),
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

    ImageWriteRequest positionUpdateRequest;
    positionUpdateRequest.descriptorSetName = kLightDescSet;
    positionUpdateRequest.bindingIndex = 0;
    positionUpdateRequest.images = positionImageInfos;
    positionUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest albedoUpdateRequest;
    albedoUpdateRequest.descriptorSetName = kLightDescSet;
    albedoUpdateRequest.bindingIndex = 1;
    albedoUpdateRequest.images = albedoImageInfos;
    albedoUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest normalUpdateRequest;
    normalUpdateRequest.descriptorSetName = kLightDescSet;
    normalUpdateRequest.bindingIndex = 2;
    normalUpdateRequest.images = normalImageInfos;
    normalUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest roughnessMetallicUpdateRequest;
    roughnessMetallicUpdateRequest.descriptorSetName = kLightDescSet;
    roughnessMetallicUpdateRequest.bindingIndex = 3;
    roughnessMetallicUpdateRequest.images = roughnessMetallicImageInfos;
    roughnessMetallicUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest lightUpdateRequest;
    lightUpdateRequest.descriptorSetName = kLightDescSet;
    lightUpdateRequest.bindingIndex = 4;
    lightUpdateRequest.buffers = storageLightBufferInfos;
    lightUpdateRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest brightPassLightingUpdateRequest;
    brightPassLightingUpdateRequest.descriptorSetName = kBrightPassDescSet;
    brightPassLightingUpdateRequest.bindingIndex = 0;
    brightPassLightingUpdateRequest.images = lightingOutputImageInfos;
    brightPassLightingUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest brightPassEmissiveUpdateRequest;
    brightPassEmissiveUpdateRequest.descriptorSetName = kBrightPassDescSet;
    brightPassEmissiveUpdateRequest.bindingIndex = 1;
    brightPassEmissiveUpdateRequest.images = emissiveMaskImageInfos;
    brightPassEmissiveUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest horizontalBlurPassBloomUpdateRequest;
    horizontalBlurPassBloomUpdateRequest.descriptorSetName = kBlurHorizontalDescSet;
    horizontalBlurPassBloomUpdateRequest.bindingIndex = 0;
    horizontalBlurPassBloomUpdateRequest.images = bloomImageImageInfosA;
    horizontalBlurPassBloomUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest verticalBlurPassBloomUpdateRequest;
    verticalBlurPassBloomUpdateRequest.descriptorSetName = kBlurVerticalDescSet;
    verticalBlurPassBloomUpdateRequest.bindingIndex = 0;
    verticalBlurPassBloomUpdateRequest.images = bloomImageImageInfosB;
    verticalBlurPassBloomUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositePassLightingUpdateRequest;
    compositePassLightingUpdateRequest.descriptorSetName = kCompositePassDescSet;
    compositePassLightingUpdateRequest.bindingIndex = 0;
    compositePassLightingUpdateRequest.images = lightingOutputImageInfos;
    compositePassLightingUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositePassBloomUpdateRequest;
    compositePassBloomUpdateRequest.descriptorSetName = kCompositePassDescSet;
    compositePassBloomUpdateRequest.bindingIndex = 1;
    compositePassBloomUpdateRequest.images = bloomImageImageInfosA;
    compositePassBloomUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {transformStorageBufferRequest, transformMaterialBufferRequest, lightUpdateRequest},
        .imageWriteRequests = {textureUpdateRequest, positionUpdateRequest, albedoUpdateRequest, normalUpdateRequest,
                               roughnessMetallicUpdateRequest, brightPassLightingUpdateRequest,
                               brightPassEmissiveUpdateRequest, horizontalBlurPassBloomUpdateRequest,
                               verticalBlurPassBloomUpdateRequest, compositePassLightingUpdateRequest,
                               compositePassBloomUpdateRequest}};

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

    window_->OnKey([&](const KeyEvent& event) {
        if (event.action != GLFW_PRESS) {
            return;
        }

        constexpr float kExposureStep = 0.5f;
        constexpr float kThresholdStep = 0.2f;
        constexpr float kSoftKneeStep = 0.1f;
        constexpr float kIntensityStep = 0.1f;
        constexpr float kSigmaStep = 0.5f;
        constexpr std::uint32_t kMaxBlurIterations = 8U;

        switch (event.key) {
            case GLFW_KEY_SPACE:
                isBloomEnabled_ = !isBloomEnabled_;
                std::cout << "Bloom enabled: " << (isBloomEnabled_ ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_1:
                brightPassSource_ = kBrightPassSourceThreshold;
                std::cout << "Bright-pass source: Luminance Threshold" << std::endl;
                break;
            case GLFW_KEY_2:
                brightPassSource_ = kBrightPassSourceEmissive;
                std::cout << "Bright-pass source: Emissive Mask" << std::endl;
                break;
            case GLFW_KEY_B:
                debugView_ = (debugView_ + 1U) % kDebugViewCount;
                std::cout << "Debug View: " << (debugView_ == kDebugViewFinal ? "Final" : "Bloom only") << std::endl;
                break;
            case GLFW_KEY_Q:
                exposure_ = std::min(exposure_ + kExposureStep, 5.0f);
                std::cout << "Current exposure: " << exposure_ << std::endl;
                break;
            case GLFW_KEY_E:
                exposure_ = std::max(exposure_ - kExposureStep, -5.0f);
                std::cout << "Current exposure: " << exposure_ << std::endl;
                break;
            case GLFW_KEY_R:
                bloomThreshold_ = std::min(bloomThreshold_ + kThresholdStep, 5.0f);
                std::cout << "Current bloom threshold: " << bloomThreshold_ << std::endl;
                break;
            case GLFW_KEY_F:
                bloomThreshold_ = std::max(bloomThreshold_ - kThresholdStep, 0.0f);
                std::cout << "Current bloom threshold: " << bloomThreshold_ << std::endl;
                break;
            case GLFW_KEY_T:
                bloomSoftKnee_ = std::min(bloomSoftKnee_ + kSoftKneeStep, 1.0f);
                std::cout << "Current soft knee: " << bloomSoftKnee_ << std::endl;
                break;
            case GLFW_KEY_G:
                bloomSoftKnee_ = std::max(bloomSoftKnee_ - kSoftKneeStep, 0.0f);
                std::cout << "Current soft knee: " << bloomSoftKnee_ << std::endl;
                break;
            case GLFW_KEY_Y:
                bloomIntensity_ = std::min(bloomIntensity_ + kIntensityStep, 3.0f);
                std::cout << "Current bloom intensity: " << bloomIntensity_ << std::endl;
                break;
            case GLFW_KEY_H:
                bloomIntensity_ = std::max(bloomIntensity_ - kIntensityStep, 0.0f);
                std::cout << "Current bloom intensity: " << bloomIntensity_ << std::endl;
                break;
            case GLFW_KEY_U:
                blurIterationCount_ = std::min(blurIterationCount_ + 1U, kMaxBlurIterations);
                std::cout << "Blur iteration count: " << blurIterationCount_ << std::endl;
                break;
            case GLFW_KEY_J:
                blurIterationCount_ = blurIterationCount_ > 0U ? blurIterationCount_ - 1 : 0U;
                std::cout << "Blur iteration count: " << blurIterationCount_ << std::endl;
                break;
            case GLFW_KEY_Z:
                blurSigma_ = std::min(blurSigma_ + kSigmaStep, 5.0f);
                std::cout << "Current blur sigma: " << blurSigma_ << std::endl;
                break;
            case GLFW_KEY_X:
                blurSigma_ = std::max(blurSigma_ - kSigmaStep, 0.5f);
                std::cout << "Current blur sigma: " << blurSigma_ << std::endl;
                break;
            default:
                break;
        }
    });
}

void VulkanApplication::CreateRenderPass()
{
    std::vector geometryColorAttachments{VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                         VkAttachmentReference{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                         VkAttachmentReference{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                         VkAttachmentReference{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkAttachmentReference depthAttachmentRef{4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    geometryRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([&](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               })
                .AddAttachment([](auto& attachmentCreateInfo) {
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
                    attachmentCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                })
                .AddAttachment([](auto& attachmentCreateInfo) {
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
                    subpassCreateInfo.colorAttachmentCount = geometryColorAttachments.size();
                    subpassCreateInfo.pColorAttachments = geometryColorAttachments.data();
                    subpassCreateInfo.pDepthStencilAttachment = &depthAttachmentRef;
                })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask =
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    dependency.dstStageMask =
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    dependency.dependencyFlags = 0;
                })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = 0;
                    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependency.dependencyFlags = 0;
                });
    });

    if (!geometryRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for geometry)!");
    }

    const auto addOffscreenDependencies = [](auto& builder) {
        builder.AddDependency([](auto& dependency) {
                   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                   dependency.dstSubpass = 0;
                   dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                   dependency.srcAccessMask = 0;
                   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                   dependency.dependencyFlags = 0;
               })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = 0;
                    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependency.dependencyFlags = 0;
                });
    };

    VkAttachmentReference lightingColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    lightingRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
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
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &lightingColorAttachment;
                });
        addOffscreenDependencies(builder);
    });

    if (!lightingRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for lighting)!");
    }

    VkAttachmentReference bloomColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    bloomRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &bloomColorAttachment;
                });
        addOffscreenDependencies(builder);
    });

    if (!bloomRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for bloom)!");
    }

    VkAttachmentReference presentColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    postProcessingRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                    subpassCreateInfo.pColorAttachments = &presentColorAttachment;
                })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.srcAccessMask = 0;
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dependencyFlags = 0;
                });
    });

    if (!postProcessingRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for post-processing)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange meshPushConstant;
    meshPushConstant.offset = 0;
    meshPushConstant.size = sizeof(MeshPushConstants);
    meshPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    geometryPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {meshPushConstant});

    if (!geometryPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    std::vector blendAttachments(4, colorBlendAttachment);

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    geometryPassPipeline_ =
            device_->CreateGraphicsPipeline(geometryPipelineLayout_, geometryRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kGeometryVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kGeometryFragmentShaderKey)->GetHandle();
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
                    blendStateCreateInfo.attachmentCount = blendAttachments.size();
                    blendStateCreateInfo.pAttachments = blendAttachments.data();
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                });
            });

    if (!geometryPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for geometry)!");
    }

    VkPushConstantRange lightingPushConstant;
    lightingPushConstant.offset = 0;
    lightingPushConstant.size = sizeof(LightingPushConstants);
    lightingPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    lightPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kLightDescSetLayout)},
                                                         {lightingPushConstant});

    if (!lightPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    lightPassPipeline_ = device_->CreateGraphicsPipeline(lightPipelineLayout_, lightingRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kFullscreenVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kLightFragmentShaderKey)->GetHandle();
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
            blendStateCreateInfo.pAttachments = blendAttachments.data();
        });
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        });
    });

    if (!lightPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for light)!");
    }

    const auto createFullscreenPipeline = [&](const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                              const std::shared_ptr<VulkanRenderPass>& renderPass,
                                              const std::string& fragmentShaderKey, const VkExtent2D& extent) {
        VkViewport passViewport{0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
        VkRect2D passScissor{0, 0, extent};

        return device_->CreateGraphicsPipeline(pipelineLayout, renderPass, [&](auto& builder) {
            builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStageCreateInfo.module = resources_->GetShaderModule(kFullscreenVertexShaderKey)->GetHandle();
            });
            builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStageCreateInfo.module = resources_->GetShaderModule(fragmentShaderKey)->GetHandle();
            });
            builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
                vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            });
            builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                viewportStateCreateInfo.viewportCount = 1;
                viewportStateCreateInfo.pViewports = &passViewport;
                viewportStateCreateInfo.scissorCount = 1;
                viewportStateCreateInfo.pScissors = &passScissor;
            });
            builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
                blendStateCreateInfo.attachmentCount = 1;
                blendStateCreateInfo.pAttachments = blendAttachments.data();
            });
            builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
                depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
            });
        });
    };

    const VkExtent2D bloomExtent = GetBloomExtent();
    const VkExtent2D fullExtent{.width = currentWindowWidth_, .height = currentWindowHeight_};

    // Bright pass (full-res HDR to half-res bloom A)
    VkPushConstantRange brightPassPushConstant;
    brightPassPushConstant.offset = 0;
    brightPassPushConstant.size = sizeof(BrightPassPushConstants);
    brightPassPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    brightPassPipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kBrightPassDescSetLayout)}, {brightPassPushConstant});

    if (!brightPassPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    brightPassPipeline_ = createFullscreenPipeline(brightPassPipelineLayout_, bloomRenderPass_,
                                                   kBrightPassFragmentShaderKey, bloomExtent);

    if (!brightPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for bright pass)!");
    }

    // Separable gaussian blur (direction is selected with push constants)
    VkPushConstantRange blurPassPushConstant;
    blurPassPushConstant.offset = 0;
    blurPassPushConstant.size = sizeof(BlurPushConstants);
    blurPassPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    blurPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kBlurDescSetLayout)},
                                                        {blurPassPushConstant});

    if (!blurPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    blurPassPipeline_ =
            createFullscreenPipeline(blurPipelineLayout_, bloomRenderPass_, kBlurFragmentShaderKey, bloomExtent);

    if (!blurPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for blur pass)!");
    }

    // Composite and tone mapping
    VkPushConstantRange compositePassPushConstant;
    compositePassPushConstant.offset = 0;
    compositePassPushConstant.size = sizeof(CompositePushConstants);
    compositePassPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    compositePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kCompositePassDescSetLayout)}, {compositePassPushConstant});

    if (!compositePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    compositePassPipeline_ = createFullscreenPipeline(compositePipelineLayout_, postProcessingRenderPass_,
                                                      kCompositeFragmentShaderKey, fullExtent);

    if (!compositePassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for composite pass)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Geometry framebuffer
    const auto& positionImageView = resources_->GetImageView(kPositionImage, kPositionImageView);
    const auto& albedoImageView = resources_->GetImageView(kAlbedoImage, kAlbedoImageView);
    const auto& normalImageView = resources_->GetImageView(kNormalImage, kNormalImageView);
    const auto& roughnessMetallicImageView =
            resources_->GetImageView(kRoughnessMetallicImage, kRoughnessMetallicImageView);
    const auto& depthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);
    geometryFramebuffer_ = device_->CreateFramebuffer(
            geometryRenderPass_,
            {positionImageView, albedoImageView, normalImageView, roughnessMetallicImageView, depthImageView},
            [&](auto& builder) { builder.SetDimensions(currentWindowWidth_, currentWindowHeight_); });

    if (!geometryFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for geometry)!");
    }

    // Lighting framebuffer
    const auto& lightingOutputImageView = resources_->GetImageView(kLightingOutputImage, kLightingOutputImageView);
    lightingFramebuffer_ =
            device_->CreateFramebuffer(lightingRenderPass_, {lightingOutputImageView}, [&](auto& builder) {
                builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
            });

    if (!lightingFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for lighting)!");
    }

    // Bloom framebuffers (half-res ping-pong targets)
    const VkExtent2D bloomExtent = GetBloomExtent();
    const auto& bloomImageViewA = resources_->GetImageView(kBloomImageA, kBloomImageViewA);
    const auto& bloomImageViewB = resources_->GetImageView(kBloomImageB, kBloomImageViewB);

    bloomFramebufferA_ = device_->CreateFramebuffer(bloomRenderPass_, {bloomImageViewA}, [&](auto& builder) {
        builder.SetDimensions(bloomExtent.width, bloomExtent.height);
    });

    bloomFramebufferB_ = device_->CreateFramebuffer(bloomRenderPass_, {bloomImageViewB}, [&](auto& builder) {
        builder.SetDimensions(bloomExtent.width, bloomExtent.height);
    });

    if (!bloomFramebufferA_ || !bloomFramebufferB_) {
        throw std::runtime_error("Failed to create framebuffer (for bloom)!");
    }

    // Present framebuffers (post-processing)
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(postProcessingRenderPass_, {swapImage}, [&](auto& builder) {
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
void VulkanApplication::RecordFullscreenPass(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer,
                                             const std::shared_ptr<VulkanRenderPass>& renderPass,
                                             const std::shared_ptr<VulkanFramebuffer>& framebuffer,
                                             const VkExtent2D& extent,
                                             const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                             const std::shared_ptr<VulkanPipeline>& pipeline,
                                             const std::string& descriptorSetName,
                                             const void* pushConstantData,
                                             const std::uint32_t pushConstantSize) const
{
    VkClearValue clearValue{};
    clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    cmdBuffer->BeginRenderPass(
            [&](auto& beginInfo) {
                beginInfo.renderPass = renderPass->GetHandle();
                beginInfo.framebuffer = framebuffer->GetHandle();
                beginInfo.renderArea.offset = {0, 0};
                beginInfo.renderArea.extent = extent;
                beginInfo.clearValueCount = 1;
                beginInfo.pClearValues = &clearValue;
            },
            VK_SUBPASS_CONTENTS_INLINE);

    const std::vector descSets{resources_->GetDescriptorSet(descriptorSetName)};
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descSets);
    cmdBuffer->BindPipeline(pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
    cmdBuffer->PushConstants(pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstantSize, pushConstantData);

    // Draw fullscreen quad
    cmdBuffer->Draw(6, 1, 0, 0);

    cmdBuffer->EndRenderPass();
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 5> geometryPassClearValues{};
    geometryPassClearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};                              // Position
    geometryPassClearValues[1].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor); // Albedo
    geometryPassClearValues[2].color = {{0.0f, 0.0f, 0.0f, 0.0f}};                              // Normal
    geometryPassClearValues[3].color = {{0.0f, 0.0f, 0.0f, 0.0f}};                              // Roughness/Metallic
    geometryPassClearValues[4].depthStencil = {1.0f, 0};                                        // Depth

    std::array<VkClearValue, 1> lightPassClearValues{};
    lightPassClearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};                                 // Not important

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Geometry Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = geometryRenderPass_->GetHandle();
                    beginInfo.framebuffer = geometryFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = geometryPassClearValues.size();
                    beginInfo.pClearValues = geometryPassClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector cubeDescSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout_, 0, cubeDescSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        // Draw scene objects
        currentCmdBuffer->BindPipeline(geometryPassPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable()) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                MeshPushConstants meshPushConstants{};
                meshPushConstants.objectId = sceneObject.GetObjectId();
                meshPushConstants.view = camera_->GetViewMatrix();
                meshPushConstants.projection = camera_->GetProjectionMatrix();

                if (sceneObject.GetTag() == kLightObjectGroup) {
                    meshPushConstants.lightIndex = lightSphereIndices_[sceneObject.GetObjectId()];
                } else {
                    meshPushConstants.lightIndex = -1;
                }

                currentCmdBuffer->PushConstants(geometryPipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(meshPushConstants), &meshPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // Light Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = lightingRenderPass_->GetHandle();
                    beginInfo.framebuffer = lightingFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = lightPassClearValues.size();
                    beginInfo.pClearValues = lightPassClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector lightDescSets{resources_->GetDescriptorSet(kLightDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, lightPipelineLayout_, 0, lightDescSets);
        currentCmdBuffer->BindPipeline(lightPassPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        LightingPushConstants lightingPushConstants{};
        lightingPushConstants.lightCount = static_cast<std::uint32_t>(pointLights.size());
        lightingPushConstants.lightSphereRadius = kLightSphereRadius;
        currentCmdBuffer->PushConstants(lightPipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(lightingPushConstants), &lightingPushConstants);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    const VkExtent2D bloomExtent = GetBloomExtent();
    const VkExtent2D fullExtent{.width = currentWindowWidth_, .height = currentWindowHeight_};

    // Bright Pass
    {
        BrightPassPushConstants brightPassPushConstants{};
        brightPassPushConstants.invOutputSize =
                glm::vec2(1.0f / static_cast<float>(bloomExtent.width), 1.0f / static_cast<float>(bloomExtent.height));
        brightPassPushConstants.exposure = exposure_;
        brightPassPushConstants.threshold = bloomThreshold_;
        brightPassPushConstants.softKnee = bloomSoftKnee_;
        brightPassPushConstants.source = brightPassSource_;

        RecordFullscreenPass(currentCmdBuffer, bloomRenderPass_, bloomFramebufferA_, bloomExtent,
                             brightPassPipelineLayout_, brightPassPipeline_, kBrightPassDescSet,
                             &brightPassPushConstants, sizeof(brightPassPushConstants));
    }

    // Separable Gaussian Blur Passes
    for (auto i = 0U; i < blurIterationCount_; ++i) {
        BlurPushConstants blurHorizontalPushConstants{};
        blurHorizontalPushConstants.texelStep = glm::vec2(1.0f / static_cast<float>(bloomExtent.width), 0.0f);
        blurHorizontalPushConstants.sigma = blurSigma_;

        RecordFullscreenPass(currentCmdBuffer, bloomRenderPass_, bloomFramebufferB_, bloomExtent, blurPipelineLayout_,
                             blurPassPipeline_, kBlurHorizontalDescSet, &blurHorizontalPushConstants,
                             sizeof(blurHorizontalPushConstants));

        BlurPushConstants blurVerticalPushConstants{};
        blurVerticalPushConstants.texelStep = glm::vec2(0.0f, 1.0f / static_cast<float>(bloomExtent.height));
        blurVerticalPushConstants.sigma = blurSigma_;

        RecordFullscreenPass(currentCmdBuffer, bloomRenderPass_, bloomFramebufferA_, bloomExtent, blurPipelineLayout_,
                             blurPassPipeline_, kBlurVerticalDescSet, &blurVerticalPushConstants,
                             sizeof(blurVerticalPushConstants));
    }

    // Composite Pass
    {
        CompositePushConstants compositePushConstants{};
        compositePushConstants.exposure = exposure_;
        compositePushConstants.bloomIntensity = bloomIntensity_;
        compositePushConstants.isBloomEnabled = isBloomEnabled_ ? 1U : 0U;
        compositePushConstants.debugView = debugView_;

        RecordFullscreenPass(currentCmdBuffer, postProcessingRenderPass_, presentFramebuffers_[currentImageIndex],
                             fullExtent, compositePipelineLayout_, compositePassPipeline_, kCompositePassDescSet,
                             &compositePushConstants, sizeof(compositePushConstants));
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    // Covert point light position from world-space to view-space
    std::vector<PointLightGpuData> pointLightInfos;
    for (const auto& pointLight: pointLights) {
        PointLightGpuData pointLightData{};
        pointLightData.lightPosition = camera_->GetViewMatrix() * pointLight.lightPosition;
        pointLightData.lightColorIntensity = pointLight.lightColorIntensity;
        pointLightInfos.push_back(pointLightData);
    }
    resources_->SetBuffer(kLightStorageBuffer, pointLightInfos.data(),
                          pointLightInfos.size() * sizeof(PointLightGpuData));
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

VkExtent2D VulkanApplication::GetBloomExtent() const
{
    return {.width = std::max(currentWindowWidth_ / kBloomDownScaleFactor, 1U),
            .height = std::max(currentWindowHeight_ / kBloomDownScaleFactor, 1U)};
}

} // namespace examples::post_processing_effects::bloom_and_glow::thresholded_gaussian_bloom
