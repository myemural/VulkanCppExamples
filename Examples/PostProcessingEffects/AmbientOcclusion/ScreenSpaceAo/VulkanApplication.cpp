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

namespace examples::post_processing_effects::ambient_occlusion::screen_space_ao
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationAmbientOcclusion(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationAmbientOcclusion::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        GenerateSsaoKernel();
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
    ApplicationAmbientOcclusion::PreUpdate();

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
    resourceCreateInfo.buffers = {{kSsaoKernelUniformBuffer, sizeof(SsaoKernelUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kLightUniformBuffer, sizeof(DirectionalLightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto geometryVertexShaderAsset = assetManager_->Load<ShaderAsset>(kGeometryVertexShaderFile);
    const auto geometryFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kGeometryFragmentShaderFile);
    const auto fullScreenQuadVertexShaderAsset = assetManager_->Load<ShaderAsset>(kFullScreenQuadVertexShaderFile);
    const auto ssaoFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSsaoFragmentShaderFile);
    const auto ssaoBlurFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSsaoBlurFragmentShaderFile);
    const auto lightFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kLightFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kGeometryVertexShaderKey, .asset = assetManager_->Get(geometryVertexShaderAsset)},
            {.name = kGeometryFragmentShaderKey, .asset = assetManager_->Get(geometryFragmentShaderAsset)},
            {.name = kFullScreenQuadVertexShaderKey, .asset = assetManager_->Get(fullScreenQuadVertexShaderAsset)},
            {.name = kSsaoFragmentShaderKey, .asset = assetManager_->Get(ssaoFragmentShaderAsset)},
            {.name = kSsaoBlurFragmentShaderKey, .asset = assetManager_->Get(ssaoBlurFragmentShaderAsset)},
            {.name = kLightFragmentShaderKey, .asset = assetManager_->Get(lightFragmentShaderAsset)}}};

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
        ImageResourceCreateInfo{
            .name = kSsaoImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8_UNORM,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kSsaoImageView, .format = VK_FORMAT_R8_UNORM}}},
        ImageResourceCreateInfo{
            .name = kSsaoBlurImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8_UNORM,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kSsaoBlurImageView, .format = VK_FORMAT_R8_UNORM}}},
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
        SamplerResourceCreateInfo{.name = kGBufferSampler,
                                  .filtering = {.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST},
                                  .addressModes = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE}},
        SamplerResourceCreateInfo{.name = kNoiseSampler,
                                  .filtering = {.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::GenerateSsaoKernel() const
{
    SsaoKernelUbo kernelUbo{};

    for (auto i = 0U; i < kSsaoKernelSize; ++i) {
        // Random direction in the tangent-space hemisphere (+Z)
        glm::vec3 sample{GenerateRandomValue(-1.0f, 1.0f), GenerateRandomValue(-1.0f, 1.0f),
                         GenerateRandomValue(0.1f, 1.0f)};
        sample = glm::normalize(sample) * GenerateRandomValue(0.1f, 1.0f);

        // Accelerating interpolation to keep more samples close to the origin
        const float t = static_cast<float>(i) / static_cast<float>(kSsaoKernelSize);
        sample *= glm::mix(0.1f, 1.0f, t * t);

        kernelUbo.samples[i] = glm::vec4(sample, 0.0f);
    }

    resources_->SetBuffer(kSsaoKernelUniformBuffer, &kernelUbo, sizeof(kernelUbo));
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
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 3.5f, 11.0f), aspectRatio);

    // Materials
    const auto wallStoneTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneTexturePath);
    const auto wallStoneTextureId =
            sceneImageStorage.StoreTexture(kWallStoneTexture, kMainSampler, assetManager_->Get(wallStoneTextureAsset));
    const auto wallStoneNormalTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneNormalTexturePath);
    const auto wallStoneNormalTextureId =
            sceneImageStorage.StoreTexture(kWallStoneNormalTexture, kMainSampler,
                                           assetManager_->Get(wallStoneNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    Material roomMaterial{};
    roomMaterial.diffuseMap = wallStoneTextureId;
    roomMaterial.normalMap = wallStoneNormalTextureId;

    Material propMaterial{};
    propMaterial.diffuseColor = glm::vec4(0.3f, 0.8f, 0.3f, 1.0f);

    // Noise texture used to rotate the sample kernel
    CreateSsaoNoiseTexture();

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);

    // Room parts (floor and walls)
    std::uint32_t index = 0;
    for (const auto& [meshType, position, scale, uvScale]: roomParts) {
        Material material = roomMaterial;
        material.uvScale = uvScale;

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kRoomObject + std::to_string(index++))
                                           .WithBuiltinMesh(meshType)
                                           .WithMaterial(material)
                                           .WithPosition(position)
                                           .WithScale(scale));
    }

    // Basic shapes inside the room
    index = 0;
    for (const auto& [meshType, position, scale, uvScale]: propObjects) {
        Material material = propMaterial;
        material.uvScale = uvScale;

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kPropObject + std::to_string(index++))
                                           .WithBuiltinMesh(meshType)
                                           .WithMaterial(material)
                                           .WithPosition(position)
                                           .WithScale(scale));
    }

    // Load and convert models
    const auto suzanneModelHandle = assetManager_->Load<GltfModelAsset>(kSuzanneModelPath);
    const auto suzanneModel = std::make_unique<GltfModelAsset>(assetManager_->Get(suzanneModelHandle));

    GltfToSceneObjectConverter converter{*scene_, kMainSampler};

    index = 0;
    for (const auto& [position, eulerAngles, scale]: suzanneInstances) {
        auto suzanneBuilder = converter.ConvertToBuilder(kSuzanneModelName + std::to_string(index++), *suzanneModel);

        rootObjectBuilder.AddChild(
                suzanneBuilder.WithPosition(position).WithEulerAngles(eulerAngles).WithScale(glm::vec3(scale)));
    }

    const auto& rootObject = rootObjectBuilder.Build();
    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateSsaoNoiseTexture() const
{
    TextureAsset noiseAsset{};
    noiseAsset.width = kSsaoNoiseDim;
    noiseAsset.height = kSsaoNoiseDim;
    noiseAsset.channels = 4;
    noiseAsset.data.resize(noiseAsset.width * noiseAsset.height * noiseAsset.channels);

    // Random rotation vectors around the tangent-space Z axis, packed into [0, 1] range
    for (auto i = 0U; i < kSsaoNoiseDim * kSsaoNoiseDim; ++i) {
        const glm::vec2 noise{GenerateRandomValue(-1.0f, 1.0f), GenerateRandomValue(-1.0f, 1.0f)};
        const auto x = (noise.x * 0.5f + 0.5f) * 255.0f;
        const auto y = (noise.y * 0.5f + 0.5f) * 255.0f;

        noiseAsset.data[i * 4 + 0] = static_cast<unsigned char>(x);
        noiseAsset.data[i * 4 + 1] = static_cast<unsigned char>(y);
        noiseAsset.data[i * 4 + 2] = 0;
        noiseAsset.data[i * 4 + 3] = 255;
    }

    scene_->GetGpuImageStorage().StoreTexture(kSsaoNoiseTexture, kNoiseSampler, noiseAsset, VK_FORMAT_R8G8B8A8_UNORM);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 4,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 9}},
        .layouts =
                {
                    {.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                    {.name = kSsaoDescSetLayout,
                     .bindings =
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                    {.name = kSsaoBlurDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kLightDescSetLayout,
                     .bindings =
                             {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                              {5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                },
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kSsaoDescSet, .layoutName = kSsaoDescSetLayout},
                           {.name = kSsaoBlurDescSet, .layoutName = kSsaoBlurDescSetLayout},
                           {.name = kLightDescSet, .layoutName = kLightDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> ssaoKernelUboInfos;
    ssaoKernelUboInfos.emplace_back(resources_->GetBuffer(kSsaoKernelUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();
    auto noiseImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfo(kSsaoNoiseTexture);

    std::vector<VkDescriptorImageInfo> positionImageInfos;
    positionImageInfos.emplace_back(resources_->GetSampler(kGBufferSampler)->GetHandle(),
                                    resources_->GetImageView(kPositionImage, kPositionImageView)->GetHandle(),
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> albedoImageInfos;
    albedoImageInfos.emplace_back(resources_->GetSampler(kGBufferSampler)->GetHandle(),
                                  resources_->GetImageView(kAlbedoImage, kAlbedoImageView)->GetHandle(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> normalImageInfos;
    normalImageInfos.emplace_back(resources_->GetSampler(kGBufferSampler)->GetHandle(),
                                  resources_->GetImageView(kNormalImage, kNormalImageView)->GetHandle(),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> ssaoImageInfos;
    ssaoImageInfos.emplace_back(resources_->GetSampler(kGBufferSampler)->GetHandle(),
                                resources_->GetImageView(kSsaoImage, kSsaoImageView)->GetHandle(),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> ssaoBlurImageInfos;
    ssaoBlurImageInfos.emplace_back(resources_->GetSampler(kGBufferSampler)->GetHandle(),
                                    resources_->GetImageView(kSsaoBlurImage, kSsaoBlurImageView)->GetHandle(),
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Geometry pass
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

    // SSAO pass
    ImageWriteRequest ssaoPositionUpdateRequest;
    ssaoPositionUpdateRequest.descriptorSetName = kSsaoDescSet;
    ssaoPositionUpdateRequest.bindingIndex = 0;
    ssaoPositionUpdateRequest.images = positionImageInfos;
    ssaoPositionUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest ssaoNormalUpdateRequest;
    ssaoNormalUpdateRequest.descriptorSetName = kSsaoDescSet;
    ssaoNormalUpdateRequest.bindingIndex = 1;
    ssaoNormalUpdateRequest.images = normalImageInfos;
    ssaoNormalUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest ssaoNoiseUpdateRequest;
    ssaoNoiseUpdateRequest.descriptorSetName = kSsaoDescSet;
    ssaoNoiseUpdateRequest.bindingIndex = 2;
    ssaoNoiseUpdateRequest.images = noiseImageInfos;
    ssaoNoiseUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest ssaoKernelUboRequest;
    ssaoKernelUboRequest.descriptorSetName = kSsaoDescSet;
    ssaoKernelUboRequest.bindingIndex = 3;
    ssaoKernelUboRequest.buffers = ssaoKernelUboInfos;
    ssaoKernelUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    // SSAO blur pass
    ImageWriteRequest ssaoBlurInputUpdateRequest;
    ssaoBlurInputUpdateRequest.descriptorSetName = kSsaoBlurDescSet;
    ssaoBlurInputUpdateRequest.bindingIndex = 0;
    ssaoBlurInputUpdateRequest.images = ssaoImageInfos;
    ssaoBlurInputUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    // Light pass
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

    ImageWriteRequest ssaoUpdateRequest;
    ssaoUpdateRequest.descriptorSetName = kLightDescSet;
    ssaoUpdateRequest.bindingIndex = 3;
    ssaoUpdateRequest.images = ssaoImageInfos;
    ssaoUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest ssaoBlurUpdateRequest;
    ssaoBlurUpdateRequest.descriptorSetName = kLightDescSet;
    ssaoBlurUpdateRequest.bindingIndex = 4;
    ssaoBlurUpdateRequest.images = ssaoBlurImageInfos;
    ssaoBlurUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest lightUboUpdateRequest;
    lightUboUpdateRequest.descriptorSetName = kLightDescSet;
    lightUboUpdateRequest.bindingIndex = 5;
    lightUboUpdateRequest.buffers = lightUboInfos;
    lightUboUpdateRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {transformStorageBufferRequest, transformMaterialBufferRequest, ssaoKernelUboRequest,
                                lightUboUpdateRequest},
        .imageWriteRequests = {textureUpdateRequest, ssaoPositionUpdateRequest, ssaoNormalUpdateRequest,
                               ssaoNoiseUpdateRequest, ssaoBlurInputUpdateRequest, positionUpdateRequest,
                               albedoUpdateRequest, normalUpdateRequest, ssaoUpdateRequest, ssaoBlurUpdateRequest}};

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

        constexpr auto kRadiusStep = 0.5f;
        constexpr auto kBiasStep = 0.01f;
        constexpr auto kPowerStep = 0.5f;

        switch (event.key) {
            case GLFW_KEY_0:
                debugMode_ = DebugMode::OFF;
                std::cout << "Debug mode changed: Off" << std::endl;
                break;
            case GLFW_KEY_1:
                debugMode_ = DebugMode::ALBEDO;
                std::cout << "Debug mode changed: Albedo" << std::endl;
                break;
            case GLFW_KEY_2:
                debugMode_ = DebugMode::NORMAL;
                std::cout << "Debug mode changed: Normal" << std::endl;
                break;
            case GLFW_KEY_3:
                debugMode_ = DebugMode::SSAO_RAW;
                std::cout << "Debug mode changed: SSAO Raw" << std::endl;
                break;
            case GLFW_KEY_4:
                debugMode_ = DebugMode::SSAO_BLURRED;
                std::cout << "Debug mode changed: SSAO Blurred" << std::endl;
                break;
            case GLFW_KEY_5:
                debugMode_ = DebugMode::AMBIENT_ONLY;
                std::cout << "Debug mode changed: Ambient Only" << std::endl;
                break;
            case GLFW_KEY_SPACE:
                isSsaoEnabled_ = !isSsaoEnabled_;
                std::cout << "SSAO enabled: " << (isSsaoEnabled_ ? "YES" : "NO") << std::endl;
                break;
            case GLFW_KEY_Z:
                ssaoRadius_ = std::max(ssaoRadius_ - kRadiusStep, 0.5f);
                std::cout << "New SSAO Radius: " << ssaoRadius_ << std::endl;
                break;
            case GLFW_KEY_X:
                ssaoRadius_ = std::min(ssaoRadius_ + kRadiusStep, 4.0f);
                std::cout << "New SSAO Radius: " << ssaoRadius_ << std::endl;
                break;
            case GLFW_KEY_C:
                ssaoBias_ = std::max(ssaoBias_ - kBiasStep, 0.0f);
                std::cout << "New SSAO Bias: " << ssaoBias_ << std::endl;
                break;
            case GLFW_KEY_V:
                ssaoBias_ = std::min(ssaoBias_ + kBiasStep, 0.25f);
                std::cout << "New SSAO Bias: " << ssaoBias_ << std::endl;
                break;
            case GLFW_KEY_B:
                ssaoPower_ = std::max(ssaoPower_ - kPowerStep, 0.5f);
                std::cout << "New SSAO Power: " << ssaoPower_ << std::endl;
                break;
            case GLFW_KEY_N:
                ssaoPower_ = std::min(ssaoPower_ + kPowerStep, 5.0f);
                std::cout << "New SSAO Power: " << ssaoPower_ << std::endl;
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
                                         VkAttachmentReference{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkAttachmentReference depthAttachmentRef{3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

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
                    subpassCreateInfo.colorAttachmentCount = 3;
                    subpassCreateInfo.pColorAttachments = geometryColorAttachments.data();
                    subpassCreateInfo.pDepthStencilAttachment = &depthAttachmentRef;
                })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = 0;
                    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                });
    });

    if (!geometryRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for geometry)!");
    }

    // This render pass is shared by the SSAO blur passes
    VkAttachmentReference occlusionColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    ssaoRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R8_UNORM;
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
                    subpassCreateInfo.pColorAttachments = &occlusionColorAttachment;
                })
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = 0;
                    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                });
    });

    if (!ssaoRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for SSAO)!");
    }

    VkAttachmentReference presentColorAttachment = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    lightRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                });
    });

    if (!lightRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for light)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    std::vector blendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

    VkPushConstantRange meshPushConstant;
    meshPushConstant.offset = 0;
    meshPushConstant.size = sizeof(MeshPushConstants);
    meshPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    geometryPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {meshPushConstant});

    if (!geometryPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

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

    VkPushConstantRange ssaoPushConstant;
    ssaoPushConstant.offset = 0;
    ssaoPushConstant.size = sizeof(SsaoPushConstants);
    ssaoPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    ssaoPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kSsaoDescSetLayout)}, {ssaoPushConstant});

    if (!ssaoPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    ssaoPassPipeline_ = device_->CreateGraphicsPipeline(ssaoPipelineLayout_, ssaoRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kFullScreenQuadVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSsaoFragmentShaderKey)->GetHandle();
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

    if (!ssaoPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for SSAO)!");
    }

    ssaoBlurPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kSsaoBlurDescSetLayout)});

    if (!ssaoBlurPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    ssaoBlurPassPipeline_ =
            device_->CreateGraphicsPipeline(ssaoBlurPipelineLayout_, ssaoRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kFullScreenQuadVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kSsaoBlurFragmentShaderKey)->GetHandle();
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

    if (!ssaoBlurPassPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for SSAO blur)!");
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

    lightPassPipeline_ = device_->CreateGraphicsPipeline(lightPipelineLayout_, lightRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kFullScreenQuadVertexShaderKey)->GetHandle();
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
}

void VulkanApplication::CreateFramebuffers()
{
    // Geometry framebuffer
    const auto& positionImageView = resources_->GetImageView(kPositionImage, kPositionImageView);
    const auto& albedoImageView = resources_->GetImageView(kAlbedoImage, kAlbedoImageView);
    const auto& normalImageView = resources_->GetImageView(kNormalImage, kNormalImageView);
    const auto& depthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);
    geometryFramebuffer_ = device_->CreateFramebuffer(
            geometryRenderPass_, {positionImageView, albedoImageView, normalImageView, depthImageView},
            [&](auto& builder) { builder.SetDimensions(currentWindowWidth_, currentWindowHeight_); });

    if (!geometryFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for geometry)!");
    }

    // SSAO framebuffer
    const auto& ssaoImageView = resources_->GetImageView(kSsaoImage, kSsaoImageView);
    ssaoFramebuffer_ = device_->CreateFramebuffer(ssaoRenderPass_, {ssaoImageView}, [&](auto& builder) {
        builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
    });

    if (!ssaoFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for SSAO)!");
    }

    // SSAO blur framebuffer
    const auto& ssaoBlurImageView = resources_->GetImageView(kSsaoBlurImage, kSsaoBlurImageView);
    ssaoBlurFramebuffer_ = device_->CreateFramebuffer(ssaoRenderPass_, {ssaoBlurImageView}, [&](auto& builder) {
        builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
    });

    if (!ssaoBlurFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for SSAO blur)!");
    }

    // Present framebuffers
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(lightRenderPass_, {swapImage}, [&](auto& builder) {
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
    std::array<VkClearValue, 4> geometryPassClearValues{};
    geometryPassClearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};                              // Position
    geometryPassClearValues[1].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor); // Albedo
    geometryPassClearValues[2].color = {{0.0f, 0.0f, 0.0f, 0.0f}};                              // Normal
    geometryPassClearValues[3].depthStencil = {1.0f, 0};                                        // Depth

    std::array<VkClearValue, 1> occlusionPassClearValues{};
    occlusionPassClearValues[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};                             // No occlusion

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

        const std::vector geometryDescSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPipelineLayout_, 0,
                                             geometryDescSets);
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
                currentCmdBuffer->PushConstants(geometryPipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(meshPushConstants), &meshPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // SSAO Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = ssaoRenderPass_->GetHandle();
                    beginInfo.framebuffer = ssaoFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = occlusionPassClearValues.size();
                    beginInfo.pClearValues = occlusionPassClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector ssaoDescSets{resources_->GetDescriptorSet(kSsaoDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipelineLayout_, 0, ssaoDescSets);
        currentCmdBuffer->BindPipeline(ssaoPassPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        SsaoPushConstants pushConstants{};
        pushConstants.projection = camera_->GetProjectionMatrix();
        pushConstants.radius = ssaoRadius_;
        pushConstants.bias = ssaoBias_;
        pushConstants.power = ssaoPower_;
        pushConstants.kernelSize = kSsaoKernelSize;
        currentCmdBuffer->PushConstants(ssaoPipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pushConstants),
                                        &pushConstants);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    // SSAO Blur Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = ssaoRenderPass_->GetHandle();
                    beginInfo.framebuffer = ssaoBlurFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = occlusionPassClearValues.size();
                    beginInfo.pClearValues = occlusionPassClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector blurDescSets{resources_->GetDescriptorSet(kSsaoBlurDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoBlurPipelineLayout_, 0, blurDescSets);
        currentCmdBuffer->BindPipeline(ssaoBlurPassPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    // Light Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = lightRenderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
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
        lightingPushConstants.debugMode = static_cast<std::uint32_t>(debugMode_);
        lightingPushConstants.ssaoEnabled = isSsaoEnabled_ ? 1U : 0U;
        lightingPushConstants.ambientStrength = kInitialAmbientStrength;
        currentCmdBuffer->PushConstants(lightPipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(lightingPushConstants), &lightingPushConstants);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    DirectionalLightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(glm::mat3(camera_->GetViewMatrix()) * kLightDirection, 0.0f);
    lightUbo.lightColor = glm::vec4(kLightColor, 1.0f);
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
} // namespace examples::post_processing_effects::ambient_occlusion::screen_space_ao
