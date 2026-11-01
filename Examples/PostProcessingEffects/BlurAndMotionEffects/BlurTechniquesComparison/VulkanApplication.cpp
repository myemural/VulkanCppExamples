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

namespace examples::post_processing_effects::blur_and_motion_effects::blur_techniques_comparison
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

    std::string GetDualKawaseMipViewName(const std::uint32_t mipLevel)
    {
        return kDualKawaseMipImageViewPrefix + std::to_string(mipLevel);
    }

    std::string GetDualKawaseMipDescSetName(const std::uint32_t mipLevel)
    {
        return kDualKawaseMipDescSetPrefix + std::to_string(mipLevel);
    }

} // namespace

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationBlurAndMotionEffects(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationBlurAndMotionEffects::Init()) {
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
    ApplicationBlurAndMotionEffects::PreUpdate();

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
    const auto boxBlurFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kBoxBlurFragmentShaderFile);
    const auto gaussianBlurFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kGaussianBlurFragmentShaderFile);
    const auto kawaseBlurFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kKawaseBlurFragmentShaderFile);
    const auto dualKawaseDownsampleFragmentShaderAsset =
            assetManager_->Load<ShaderAsset>(kDualKawaseDownsampleFragmentShaderFile);
    const auto dualKawaseUpsampleFragmentShaderAsset =
            assetManager_->Load<ShaderAsset>(kDualKawaseUpsampleFragmentShaderFile);
    const auto compositeFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kCompositeFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kGeometryVertexShaderKey, .asset = assetManager_->Get(geometryVertexShaderAsset)},
            {.name = kGeometryFragmentShaderKey, .asset = assetManager_->Get(geometryFragmentShaderAsset)},
            {.name = kFullscreenVertexShaderKey, .asset = assetManager_->Get(fullscreenVertexShaderAsset)},
            {.name = kLightFragmentShaderKey, .asset = assetManager_->Get(lightFragmentShaderAsset)},
            {.name = kBoxBlurFragmentShaderKey, .asset = assetManager_->Get(boxBlurFragmentShaderAsset)},
            {.name = kGaussianBlurFragmentShaderKey, .asset = assetManager_->Get(gaussianBlurFragmentShaderAsset)},
            {.name = kKawaseBlurFragmentShaderKey, .asset = assetManager_->Get(kawaseBlurFragmentShaderAsset)},
            {.name = kDualKawaseDownsampleFragmentShaderKey,
             .asset = assetManager_->Get(dualKawaseDownsampleFragmentShaderAsset)},
            {.name = kDualKawaseUpsampleFragmentShaderKey,
             .asset = assetManager_->Get(dualKawaseUpsampleFragmentShaderAsset)},
            {.name = kCompositeFragmentShaderKey, .asset = assetManager_->Get(compositeFragmentShaderAsset)}}};

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
            .name = kBlurImageA,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kBlurImageViewA, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kBlurImageB,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kBlurImageViewB, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
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


    const VkExtent2D dualKawaseExtent = GetDualKawaseMipExtent(0U);
    ImageResourceCreateInfo dualKawaseImageCreateInfo{
        .name = kDualKawaseImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .dimensions = {dualKawaseExtent.width, dualKawaseExtent.height, 1},
        .mipLevels = kDualKawaseMipCount,
        .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};

    for (auto mip = 0U; mip < kDualKawaseMipCount; ++mip) {
        dualKawaseImageCreateInfo.views.push_back(
                ImageViewCreateInfo{.viewName = GetDualKawaseMipViewName(mip),
                                    .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                    .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                         .baseMipLevel = mip,
                                                         .levelCount = 1,
                                                         .baseArrayLayer = 0,
                                                         .layerCount = 1}});
    }
    resourceCreateInfo.images->push_back(dualKawaseImageCreateInfo);

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
    DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 6 + kDualKawaseMipCount,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       combinedImageSamplerCount + 10 + kDualKawaseMipCount}},
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
                 {.name = kBlurSourceDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}},
                 {.name = kCompositePassDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}}},
        .descriptorSets = {
            {.name = kMainDescSet, .layoutName = kMainDescSetLayout},
            {.name = kLightDescSet, .layoutName = kLightDescSetLayout},
            {.name = kSceneColorDescSet, .layoutName = kBlurSourceDescSetLayout},
            {.name = kBlurImageDescSetA, .layoutName = kBlurSourceDescSetLayout},
            {.name = kBlurImageDescSetB, .layoutName = kBlurSourceDescSetLayout},
            {.name = kCompositePassDescSet, .layoutName = kCompositePassDescSetLayout},
        }};

    for (auto mip = 0U; mip < kDualKawaseMipCount; ++mip) {
        descriptorResourceCreateInfo.descriptorSets.push_back(
                {.name = GetDualKawaseMipDescSetName(mip), .layoutName = kBlurSourceDescSetLayout});
    }

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

    std::vector<VkDescriptorImageInfo> blurImageAInfos;
    blurImageAInfos.emplace_back(resources_->GetSampler(kLinearClampSampler)->GetHandle(),
                                 resources_->GetImageView(kBlurImageA, kBlurImageViewA)->GetHandle(),
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> blurImageBInfos;
    blurImageBInfos.emplace_back(resources_->GetSampler(kLinearClampSampler)->GetHandle(),
                                 resources_->GetImageView(kBlurImageB, kBlurImageViewB)->GetHandle(),
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

    ImageWriteRequest sceneColorLightingUpdateRequest;
    sceneColorLightingUpdateRequest.descriptorSetName = kSceneColorDescSet;
    sceneColorLightingUpdateRequest.bindingIndex = 0;
    sceneColorLightingUpdateRequest.images = lightingOutputImageInfos;
    sceneColorLightingUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest blurImageAUpdateRequest;
    blurImageAUpdateRequest.descriptorSetName = kBlurImageDescSetA;
    blurImageAUpdateRequest.bindingIndex = 0;
    blurImageAUpdateRequest.images = blurImageAInfos;
    blurImageAUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest blurImageBUpdateRequest;
    blurImageBUpdateRequest.descriptorSetName = kBlurImageDescSetB;
    blurImageBUpdateRequest.bindingIndex = 0;
    blurImageBUpdateRequest.images = blurImageBInfos;
    blurImageBUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositePassLightingUpdateRequest;
    compositePassLightingUpdateRequest.descriptorSetName = kCompositePassDescSet;
    compositePassLightingUpdateRequest.bindingIndex = 0;
    compositePassLightingUpdateRequest.images = lightingOutputImageInfos;
    compositePassLightingUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositePassBlurImageAUpdateRequest;
    compositePassBlurImageAUpdateRequest.descriptorSetName = kCompositePassDescSet;
    compositePassBlurImageAUpdateRequest.bindingIndex = 1;
    compositePassBlurImageAUpdateRequest.images = blurImageAInfos;
    compositePassBlurImageAUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositePassBlurImageBUpdateRequest;
    compositePassBlurImageBUpdateRequest.descriptorSetName = kCompositePassDescSet;
    compositePassBlurImageBUpdateRequest.bindingIndex = 2;
    compositePassBlurImageBUpdateRequest.images = blurImageBInfos;
    compositePassBlurImageBUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {transformStorageBufferRequest, transformMaterialBufferRequest, lightUpdateRequest},
        .imageWriteRequests = {textureUpdateRequest, positionUpdateRequest, albedoUpdateRequest, normalUpdateRequest,
                               roughnessMetallicUpdateRequest, sceneColorLightingUpdateRequest, blurImageAUpdateRequest,
                               blurImageBUpdateRequest, compositePassLightingUpdateRequest,
                               compositePassBlurImageAUpdateRequest, compositePassBlurImageBUpdateRequest}};

    for (auto mip = 0U; mip < kDualKawaseMipCount; ++mip) {
        std::vector<VkDescriptorImageInfo> mipImageInfos;
        mipImageInfos.emplace_back(
                resources_->GetSampler(kLinearClampSampler)->GetHandle(),
                resources_->GetImageView(kDualKawaseImage, GetDualKawaseMipViewName(mip))->GetHandle(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        descriptorSetUpdateInfo.imageWriteRequests.emplace_back(
                GetDualKawaseMipDescSetName(mip), 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, mipImageInfos);
    }

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

        switch (event.key) {
            case GLFW_KEY_1:
                blurType_ = BlurType::NONE;
                std::cout << "Selected blur technique: None (No Blur)" << std::endl;
                break;
            case GLFW_KEY_2:
                blurType_ = BlurType::BOX;
                std::cout << "Selected blur technique: Box Blur" << std::endl;
                break;
            case GLFW_KEY_3:
                blurType_ = BlurType::GAUSSIAN;
                std::cout << "Selected blur technique: Gaussian Blur" << std::endl;
                break;
            case GLFW_KEY_4:
                blurType_ = BlurType::KAWASE;
                std::cout << "Selected blur technique: Kawase Blur" << std::endl;
                break;
            case GLFW_KEY_5:
                blurType_ = BlurType::DUAL_KAWASE;
                std::cout << "Selected blur technique: Dual Kawase Blur" << std::endl;
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
                blurLevel_ = std::min(blurLevel_ + 1U, kMaxBlurLevel);
                std::cout << "Blur level: " << blurLevel_ << std::endl;
                break;
            case GLFW_KEY_F:
                blurLevel_ = std::max(blurLevel_ - 1U, kMinBlurLevel);
                std::cout << "Blur level: " << blurLevel_ << std::endl;
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

    VkAttachmentReference blurColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    blurRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([&](auto& attachmentCreateInfo) {
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
                    subpassCreateInfo.pColorAttachments = &blurColorAttachment;
                });
        addOffscreenDependencies(builder);
    });

    if (!blurRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for blur)!");
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

    constexpr std::array dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    const auto createFullscreenPipeline = [&](const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                              const std::shared_ptr<VulkanRenderPass>& renderPass,
                                              const std::string& fragmentShaderKey) {
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
                viewportStateCreateInfo.pViewports = nullptr;
                viewportStateCreateInfo.scissorCount = 1;
                viewportStateCreateInfo.pScissors = nullptr;
            });
            builder.SetDynamicState([&](auto& synamicStateCreateInfo) {
                synamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
                synamicStateCreateInfo.pDynamicStates = dynamicStates.data();
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

    // Blur pipelines
    VkPushConstantRange blurPassPushConstant;
    blurPassPushConstant.offset = 0;
    blurPassPushConstant.size = sizeof(BlurPassPushConstants);
    blurPassPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    blurPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kBlurSourceDescSetLayout)},
                                                        {blurPassPushConstant});

    if (!blurPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    boxBlurPipeline_ = createFullscreenPipeline(blurPipelineLayout_, blurRenderPass_, kBoxBlurFragmentShaderKey);
    gaussianBlurPipeline_ =
            createFullscreenPipeline(blurPipelineLayout_, blurRenderPass_, kGaussianBlurFragmentShaderKey);
    kawaseBlurPipeline_ = createFullscreenPipeline(blurPipelineLayout_, blurRenderPass_, kKawaseBlurFragmentShaderKey);
    dualKawaseDownsamplePipeline_ =
            createFullscreenPipeline(blurPipelineLayout_, blurRenderPass_, kDualKawaseDownsampleFragmentShaderKey);
    dualKawaseUpsamplePipeline_ =
            createFullscreenPipeline(blurPipelineLayout_, blurRenderPass_, kDualKawaseUpsampleFragmentShaderKey);

    if (!boxBlurPipeline_ || !gaussianBlurPipeline_ || !kawaseBlurPipeline_ || !dualKawaseDownsamplePipeline_ ||
        !dualKawaseUpsamplePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for blur)!");
    }

    // Composite pipeline
    VkPushConstantRange compositePassPushConstant;
    compositePassPushConstant.offset = 0;
    compositePassPushConstant.size = sizeof(CompositePushConstants);
    compositePassPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    compositePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kCompositePassDescSetLayout)}, {compositePassPushConstant});

    if (!compositePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    compositePassPipeline_ =
            createFullscreenPipeline(compositePipelineLayout_, postProcessingRenderPass_, kCompositeFragmentShaderKey);

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

    // Blur framebuffers, full-res ping-pong targets and one framebuffer per dual kawase mip level
    const auto& blurImageViewA = resources_->GetImageView(kBlurImageA, kBlurImageViewA);
    const auto& blurImageViewB = resources_->GetImageView(kBlurImageB, kBlurImageViewB);

    blurFramebufferA_ = device_->CreateFramebuffer(blurRenderPass_, {blurImageViewA}, [&](auto& builder) {
        builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
    });

    blurFramebufferB_ = device_->CreateFramebuffer(blurRenderPass_, {blurImageViewB}, [&](auto& builder) {
        builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
    });

    if (!blurFramebufferA_ || !blurFramebufferB_) {
        throw std::runtime_error("Failed to create framebuffer (for blur ping-pong targets)!");
    }

    for (auto mip = 0U; mip < kDualKawaseMipCount; ++mip) {
        const VkExtent2D mipExtent = GetDualKawaseMipExtent(mip);
        const auto& mipImageView = resources_->GetImageView(kDualKawaseImage, GetDualKawaseMipViewName(mip));

        const auto dualKawaseFramebuffer =
                device_->CreateFramebuffer(blurRenderPass_, {mipImageView}, [&](auto& builder) {
                    builder.SetDimensions(mipExtent.width, mipExtent.height);
                });

        if (!dualKawaseFramebuffer) {
            throw std::runtime_error("Failed to create framebuffer (for Dual Kawase blur)!");
        }

        dualKawaseFramebuffers_.push_back(dualKawaseFramebuffer);
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
    cmdBuffer->SetViewports(0, {VkViewport{0.0f, 0.0f, static_cast<float>(extent.width),
                                           static_cast<float>(extent.height), 0.0f, 1.0f}});
    cmdBuffer->SetScissors(0, {VkRect2D{{0, 0}, extent}});
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

    const VkExtent2D fullExtent{currentWindowWidth_, currentWindowHeight_};

    const auto toTexelSize = [](const VkExtent2D& extent) {
        return glm::vec2(1.0f / static_cast<float>(extent.width), 1.0f / static_cast<float>(extent.height));
    };

    // Records one blur pass: Reads the source descriptor set, writes the target framebuffer
    const auto recordBlurPass = [&](const std::shared_ptr<VulkanPipeline>& pipeline,
                                    const std::shared_ptr<VulkanFramebuffer>& framebuffer, const VkExtent2D& dstExtent,
                                    const std::string& srcDescSet, const VkExtent2D& srcExtent,
                                    const glm::vec2& direction, const float parameter) {
        BlurPassPushConstants pushConstants{};
        pushConstants.srcTexelSize = toTexelSize(srcExtent);
        pushConstants.dstTexelSize = toTexelSize(dstExtent);
        pushConstants.direction = direction;
        pushConstants.parameter = parameter;

        RecordFullscreenPass(currentCmdBuffer, blurRenderPass_, framebuffer, dstExtent, blurPipelineLayout_, pipeline,
                             srcDescSet, &pushConstants, sizeof(pushConstants));
    };

    // Blur Passes
    std::uint32_t resultImage = 0U; // 0: No blur, 1: Blur image A, 2: Blur image B
    const float blurParameter = GetBlurParameter();
    constexpr glm::vec2 horizontalDirection{1.0f, 0.0f};
    constexpr glm::vec2 verticalDirection{0.0f, 1.0f};

    switch (blurType_) {
        case BlurType::BOX:
        case BlurType::GAUSSIAN: {
            // Separable blur: Scene to A (horizontal), A to B (vertical)
            const auto& pipeline = blurType_ == BlurType::BOX ? boxBlurPipeline_ : gaussianBlurPipeline_;
            recordBlurPass(pipeline, blurFramebufferA_, fullExtent, kSceneColorDescSet, fullExtent, horizontalDirection,
                           blurParameter);
            recordBlurPass(pipeline, blurFramebufferB_, fullExtent, kBlurImageDescSetA, fullExtent, verticalDirection,
                           blurParameter);
            resultImage = 2U;
            break;
        }
        case BlurType::KAWASE: {
            // Iteration with growing offsets (0, 1, 2, ...): Scene to A, A to B, B to A and so on...
            const auto iterationCount = static_cast<std::uint32_t>(blurParameter);
            for (auto i = 0U; i < iterationCount; ++i) {
                const bool writesToA = (i % 2U) == 0U;
                const std::string srcDescSet = (i == 0)    ? kSceneColorDescSet
                                               : writesToA ? kBlurImageDescSetB
                                                           : kBlurImageDescSetA;
                recordBlurPass(kawaseBlurPipeline_, writesToA ? blurFramebufferA_ : blurFramebufferB_, fullExtent,
                               srcDescSet, fullExtent, glm::vec2(0.0f), static_cast<float>(i));
                resultImage = writesToA ? 1U : 2U;
            }
            break;
        }
        case BlurType::DUAL_KAWASE: {
            // Downsample chain
            const auto levelCount = static_cast<std::uint32_t>(blurParameter);
            for (auto mip = 0U; mip < levelCount; ++mip) {
                const bool isFirstPass = mip == 0;
                recordBlurPass(dualKawaseDownsamplePipeline_, dualKawaseFramebuffers_.at(mip),
                               GetDualKawaseMipExtent(mip),
                               isFirstPass ? std::string(kSceneColorDescSet) : GetDualKawaseMipDescSetName(mip - 1),
                               isFirstPass ? fullExtent : GetDualKawaseMipExtent(mip - 1), glm::vec2(0.0f), 1.0f);
            }

            // Upsample chain
            for (auto mip = levelCount - 1; mip > 0; --mip) {
                recordBlurPass(dualKawaseUpsamplePipeline_, dualKawaseFramebuffers_.at(mip - 1),
                               GetDualKawaseMipExtent(mip - 1), GetDualKawaseMipDescSetName(mip),
                               GetDualKawaseMipExtent(mip), glm::vec2(0.0f), 1.0f);
            }
            recordBlurPass(dualKawaseUpsamplePipeline_, blurFramebufferA_, fullExtent, GetDualKawaseMipDescSetName(0),
                           GetDualKawaseMipExtent(0), glm::vec2(0.0f), 1.0f);
            resultImage = 1U;
            break;
        }
        default:
            break;
    }

    // Composite Pass
    {
        CompositePushConstants compositePushConstants{};
        compositePushConstants.exposure = exposure_;
        compositePushConstants.resultImage = resultImage;

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

VkExtent2D VulkanApplication::GetDualKawaseMipExtent(const std::uint32_t mipLevel) const
{
    return {
        std::max(currentWindowWidth_ >> (mipLevel + 1), 1U),
        std::max(currentWindowHeight_ >> (mipLevel + 1), 1U),
    };
}

float VulkanApplication::GetBlurParameter() const
{
    const auto level = static_cast<float>(blurLevel_);
    switch (blurType_) {
        case BlurType::BOX:
            return 4.0f * level; // Radius in pixels (sigma of a box is "radius / sqrt(3)")
            break;
        case BlurType::GAUSSIAN:
            return 2.5f * level; // Sigma in pixels
            break;
        case BlurType::KAWASE:
            return level + 2.0f; // Iteration count
            break;
        case BlurType::DUAL_KAWASE:
            return level;        // Downsample/upsample level count
            break;
        default:
            return 0.0f;
    }
}

} // namespace examples::post_processing_effects::blur_and_motion_effects::blur_techniques_comparison
