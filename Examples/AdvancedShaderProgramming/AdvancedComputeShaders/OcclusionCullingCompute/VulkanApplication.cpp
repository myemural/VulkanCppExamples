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

namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
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
        BuildScene();
        UploadObjectBounds();
        SetAndMapCullingResultsBuffer();
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

    UpdateLightUBO();
    RecordAllCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
    frameCounter_++;
}

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationAdvancedComputeShaders::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::PostUpdate()
{
    ApplicationAdvancedComputeShaders::PostUpdate();

    // Print stats related to draw calls and culled objects
    ReportStats();
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
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kObjectBoundsBuffer, kTotalObjectCount * sizeof(ObjectBounds),
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kCullingResultsBuffer, kCullingResBufferElementCount * sizeof(std::uint32_t),
                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto sceneObjectsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneObjectsFragmentShaderFile);
    const auto hizCopyComputeShaderAsset = assetManager_->Load<ShaderAsset>(kHizCopyComputeShaderFile);
    const auto hizReduceComputeShaderAsset = assetManager_->Load<ShaderAsset>(kHizReduceComputeShaderFile);
    const auto occlusionComputeShaderAsset = assetManager_->Load<ShaderAsset>(kOcclusionComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
            {.name = kSceneObjectsFragmentShaderKey, .asset = assetManager_->Get(sceneObjectsFragmentShaderAsset)},
            {.name = kHizCopyComputeShaderKey, .asset = assetManager_->Get(hizCopyComputeShaderAsset)},
            {.name = kHizReduceComputeShaderKey, .asset = assetManager_->Get(hizReduceComputeShaderAsset)},
            {.name = kOcclusionComputeShaderKey, .asset = assetManager_->Get(occlusionComputeShaderAsset)}}};

    hizMipCount_ = GetMipLevelCount(currentWindowWidth_, currentWindowHeight_);

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kDepthImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = depthImageFormat_,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kDepthImageView,
                                          .format = depthImageFormat_,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}},
        ImageResourceCreateInfo{
            .name = kHizImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R32_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .mipLevels = hizMipCount_,
            .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kHizImageViewAllMips,
                                          .format = VK_FORMAT_R32_SFLOAT,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = hizMipCount_,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}}};

    // Create separate image views for each mips of Hi-Z image
    for (auto i = 0U; i < hizMipCount_; ++i) {
        const std::string imageViewName = kHizImageViewMipPrefix + std::to_string(i);
        resourceCreateInfo.images->back().views.emplace_back(
                ImageViewCreateInfo{.viewName = imageViewName,
                                    .format = VK_FORMAT_R32_SFLOAT,
                                    .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, 1}});
    }

    resourceCreateInfo.samplers = {
        SamplerResourceCreateInfo{.name = kMainSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}},
        SamplerResourceCreateInfo{.name = kPointSampler,
                                  .filtering = {.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST},
                                  .addressModes = {.u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   .v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   .w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
                                  .lod = {.minLod = 0.0f, .maxLod = static_cast<float>(hizMipCount_)}}};

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
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 12.0f), aspectRatio);

    // Materials
    const auto wallStoneTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneTexturePath);
    const auto wallStoneTextureId =
            sceneImageStorage.StoreTexture(kWallStoneTexture, kMainSampler, assetManager_->Get(wallStoneTextureAsset));
    const auto wallStoneNormalTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneNormalTexturePath);
    const auto wallStoneNormalTextureId =
            sceneImageStorage.StoreTexture(kWallStoneNormalTexture, kMainSampler,
                                           assetManager_->Get(wallStoneNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    const auto tilesTextureAsset = assetManager_->Load<TextureAsset>(kTilesTexturePath);
    const auto tilesTextureId =
            sceneImageStorage.StoreTexture(kTilesTexture, kMainSampler, assetManager_->Get(tilesTextureAsset));
    const auto tilesNormalTextureAsset = assetManager_->Load<TextureAsset>(kTilesNormalTexturePath);
    const auto tilesNormalTextureId = sceneImageStorage.StoreTexture(
            kTilesNormalTexture, kMainSampler, assetManager_->Get(tilesNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    Material defaultMaterial;
    defaultMaterial.ambientStrength = kAmbientStrength;
    defaultMaterial.specularStrength = kSpecularStrength;
    defaultMaterial.shininess = kSpecularShininess;
    defaultMaterial.diffuseMap = wallStoneTextureId;
    defaultMaterial.normalMap = wallStoneNormalTextureId;
    defaultMaterial.uvScale = 1.0f;

    Material planeMaterial;
    planeMaterial.ambientStrength = kAmbientStrength;
    planeMaterial.specularStrength = kSpecularStrength;
    planeMaterial.shininess = kSpecularShininess;
    planeMaterial.diffuseMap = tilesTextureId;
    planeMaterial.normalMap = tilesNormalTextureId;
    planeMaterial.uvScale = 14.0f;

    // Add scene objects
    std::uint32_t index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);

    for (const auto& cubePos: cubePositions) {
        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kCubeObject + std::to_string(index))
                                           .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                           .WithMaterial(defaultMaterial)
                                           .WithPosition(cubePos)
                                           .WithScale(glm::vec3{kCubeScale}));

        index++;
    }

    for (const auto& planePos: planePositions) {
        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kPlaneOccluderObject + std::to_string(index))
                                           .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                           .WithMaterial(planeMaterial)
                                           .WithPosition(planePos)
                                           .WithScale(glm::vec3{kPlaneScale, 0.1f, kPlaneScale}));

        index++;
    }

    scene_->AddRootObject(rootObjectBuilder.Build());
}

void VulkanApplication::UploadObjectBounds() const
{
    std::vector<ObjectBounds> bounds{kTotalObjectCount};

    const auto storeBounds = [&](const std::string& objectName, const glm::vec3& position, const glm::vec3& scale) {
        const auto sceneObject = scene_->FindObjectByName(objectName);
        if (!sceneObject) {
            throw std::runtime_error("Scene object \"" + objectName + "\" not found!");
        }

        ObjectBounds objectBounds;
        objectBounds.center = glm::vec4(position, 1.0f);
        objectBounds.extent = glm::vec4(scale * 0.5f, 0.0f);

        bounds.at(sceneObject->GetObjectId()) = objectBounds;
    };

    // Store bounds for all scene objects
    int index = 0;
    for (const auto& cubePos: cubePositions) {
        storeBounds(kCubeObject + std::to_string(index), cubePos, glm::vec3{kCubeScale});
        index++;
    }

    for (const auto& planePos: planePositions) {
        storeBounds(kPlaneOccluderObject + std::to_string(index), planePos, glm::vec3{kPlaneScale, 0.1f, kPlaneScale});
        index++;
    }

    resources_->SetBuffer(kObjectBoundsBuffer, bounds.data(), bounds.size() * sizeof(ObjectBounds));
}

void VulkanApplication::SetAndMapCullingResultsBuffer()
{
    cullingResultsData_ = static_cast<std::uint32_t*>(resources_->MapAndGetBufferMemory(kCullingResultsBuffer));

    // Reset all values with 1, except first two values (frustum and occlusion count results)
    std::fill_n(cullingResultsData_, kCullingResBufferElementCount, 1U);
    cullingResultsData_[kFrustumCulledResultIndex] = 0U;
    cullingResultsData_[kOcclusionCulledResultIndex] = 0U;
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4},
                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 2}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                    {.name = kHizCopyDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}},
                    {.name = kHizReduceDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}},
                    {.name = kOcclusionDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kHizCopyDescSet, .layoutName = kHizCopyDescSetLayout},
                           {.name = kOcclusionDescSet, .layoutName = kOcclusionDescSetLayout}}};

    const auto reduceCount = hizMipCount_ - 1;
    for (auto i = 0U; i < reduceCount; ++i) {
        descriptorResourceCreateInfo.descriptorSets.emplace_back(kHizReduceDescSetPrefix + std::to_string(i),
                                                                 kHizReduceDescSetLayout);
        descriptorResourceCreateInfo.maxSets++;
    }

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

    std::vector<VkDescriptorImageInfo> depthSampledImageInfos;
    depthSampledImageInfos.emplace_back(resources_->GetSampler(kPointSampler)->GetHandle(),
                                        resources_->GetImageView(kDepthImage, kDepthImageView)->GetHandle(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> hizMipZeroStorageImageInfos;
    hizMipZeroStorageImageInfos.emplace_back(
            VK_NULL_HANDLE,
            resources_->GetImageView(kHizImage, kHizImageViewMipPrefix + std::to_string(0))->GetHandle(),
            VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> hizStorageSampledImageInfos;
    hizStorageSampledImageInfos.emplace_back(resources_->GetSampler(kPointSampler)->GetHandle(),
                                             resources_->GetImageView(kHizImage, kHizImageViewAllMips)->GetHandle(),
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorBufferInfo> objectBoundsBufferInfos;
    objectBoundsBufferInfos.emplace_back(resources_->GetBuffer(kObjectBoundsBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> cullingResultsBufferInfos;
    cullingResultsBufferInfos.emplace_back(resources_->GetBuffer(kCullingResultsBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

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

    ImageWriteRequest hizDepthSampledUpdateRequest;
    hizDepthSampledUpdateRequest.descriptorSetName = kHizCopyDescSet;
    hizDepthSampledUpdateRequest.bindingIndex = 0;
    hizDepthSampledUpdateRequest.images = depthSampledImageInfos;
    hizDepthSampledUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest hizMipZeroStorageUpdateRequest;
    hizMipZeroStorageUpdateRequest.descriptorSetName = kHizCopyDescSet;
    hizMipZeroStorageUpdateRequest.bindingIndex = 1;
    hizMipZeroStorageUpdateRequest.images = hizMipZeroStorageImageInfos;
    hizMipZeroStorageUpdateRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    BufferWriteRequest objectBoundsBufferRequest;
    objectBoundsBufferRequest.descriptorSetName = kOcclusionDescSet;
    objectBoundsBufferRequest.bindingIndex = 0;
    objectBoundsBufferRequest.buffers = objectBoundsBufferInfos;
    objectBoundsBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest cullingResultsBufferRequest;
    cullingResultsBufferRequest.descriptorSetName = kOcclusionDescSet;
    cullingResultsBufferRequest.bindingIndex = 1;
    cullingResultsBufferRequest.buffers = cullingResultsBufferInfos;
    cullingResultsBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    ImageWriteRequest hizStorageSampledUpdateRequest;
    hizStorageSampledUpdateRequest.descriptorSetName = kOcclusionDescSet;
    hizStorageSampledUpdateRequest.bindingIndex = 2;
    hizStorageSampledUpdateRequest.images = hizStorageSampledImageInfos;
    hizStorageSampledUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest, objectBoundsBufferRequest, cullingResultsBufferRequest},
        .imageWriteRequests = {textureUpdateRequest, hizDepthSampledUpdateRequest, hizMipZeroStorageUpdateRequest,
                               hizStorageSampledUpdateRequest}};

    for (auto i = 0U; i < reduceCount; ++i) {
        std::vector<VkDescriptorImageInfo> hizReduceSrcStorageImageInfos;
        hizReduceSrcStorageImageInfos.emplace_back(
                VK_NULL_HANDLE,
                resources_->GetImageView(kHizImage, kHizImageViewMipPrefix + std::to_string(i))->GetHandle(),
                VK_IMAGE_LAYOUT_GENERAL);

        std::vector<VkDescriptorImageInfo> hizReduceDstStorageImageInfos;
        hizReduceDstStorageImageInfos.emplace_back(
                VK_NULL_HANDLE,
                resources_->GetImageView(kHizImage, kHizImageViewMipPrefix + std::to_string(i + 1))->GetHandle(),
                VK_IMAGE_LAYOUT_GENERAL);

        ImageWriteRequest hizReduceSrcStorageUpdateRequest;
        hizReduceSrcStorageUpdateRequest.descriptorSetName = kHizReduceDescSetPrefix + std::to_string(i);
        hizReduceSrcStorageUpdateRequest.bindingIndex = 0;
        hizReduceSrcStorageUpdateRequest.images = hizReduceSrcStorageImageInfos;
        hizReduceSrcStorageUpdateRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        ImageWriteRequest hizReduceDstStorageUpdateRequest;
        hizReduceDstStorageUpdateRequest.descriptorSetName = kHizReduceDescSetPrefix + std::to_string(i);
        hizReduceDstStorageUpdateRequest.bindingIndex = 1;
        hizReduceDstStorageUpdateRequest.images = hizReduceDstStorageImageInfos;
        hizReduceDstStorageUpdateRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        descriptorSetUpdateInfo.imageWriteRequests.push_back(hizReduceSrcStorageUpdateRequest);
        descriptorSetUpdateInfo.imageWriteRequests.push_back(hizReduceDstStorageUpdateRequest);
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

        if (event.key == GLFW_KEY_F) {
            isFrustumCullingEnabled_ = !isFrustumCullingEnabled_;
            std::cout << "Frustum culling: " << (isFrustumCullingEnabled_ ? "ON" : "OFF") << std::endl;
        } else if (event.key == GLFW_KEY_O) {
            isOcclusionCullingEnabled_ = !isOcclusionCullingEnabled_;
            std::cout << "Occlusion culling: " << (isOcclusionCullingEnabled_ ? "ON" : "OFF") << std::endl;
        }
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
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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
    hizCopyComputePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kHizCopyDescSetLayout)});

    if (!hizCopyComputePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    hizCopyComputePipeline_ = device_->CreateComputePipeline(hizCopyComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kHizCopyComputeShaderKey)->GetHandle();
        });
    });

    hizReduceComputePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kHizReduceDescSetLayout)});

    if (!hizReduceComputePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    hizReduceComputePipeline_ = device_->CreateComputePipeline(hizReduceComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kHizReduceComputeShaderKey)->GetHandle();
        });
    });

    VkPushConstantRange occlusionPushConstants;
    occlusionPushConstants.offset = 0;
    occlusionPushConstants.size = sizeof(OcclusionPushConstants);
    occlusionPushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    occlusionComputePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kOcclusionDescSetLayout)}, {occlusionPushConstants});

    if (!occlusionComputePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    occlusionComputePipeline_ = device_->CreateComputePipeline(occlusionComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kOcclusionComputeShaderKey)->GetHandle();
        });
    });

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
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}
void VulkanApplication::RecordComputeCommandBuffers(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    const auto depthImage = resources_->GetImage(kDepthImage);
    const auto hizImage = resources_->GetImage(kHizImage);

    // Barriers for depth image to SHADER_READ_ONLY_OPTIMAL and Hi-Z image (and all mip levels) to GENERAL
    {
        const bool isFirstFrame = frameCounter_ == 0U;

        const auto depthToShaderReadBarrier = depthImage->CreateImageMemoryBarrier(
                isFirstFrame ? 0 : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                isFirstFrame ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});
        cmdBuffer->PipelineBarrier(isFirstFrame ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {depthToShaderReadBarrier});

        const auto hizImageToGeneralBarrier = hizImage->CreateImageMemoryBarrier(
                isFirstFrame ? 0 : VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                isFirstFrame ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL, VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, hizMipCount_, 0, 1});
        cmdBuffer->PipelineBarrier(isFirstFrame ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
                                                : VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {hizImageToGeneralBarrier});
    }

    // Compute Pass: Hi-Z Copy Pipeline (for only Mip 0)
    {
        cmdBuffer->BindPipeline(hizCopyComputePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{resources_->GetDescriptorSet(kHizCopyDescSet)};
        cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, hizCopyComputePipelineLayout_, 0, descSets);

        const auto groupCountX = CeilDiv(currentWindowWidth_, kHizBuildLocalSizeX);
        const auto groupCountY = CeilDiv(currentWindowHeight_, kHizBuildLocalSizeY);
        cmdBuffer->Dispatch(groupCountX, groupCountY, 1);
    }

    // Compute Pass: Hi-Z Reduce Pipeline (for other Mip levels)
    {
        cmdBuffer->BindPipeline(hizReduceComputePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);

        for (auto i = 0U; i + 1 < hizMipCount_; ++i) {
            // Barrier for changing read/write access masks for each mip level
            const auto hizImageFromWriteToReadBarrier = hizImage->CreateImageMemoryBarrier(
                    VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                    VK_IMAGE_LAYOUT_GENERAL, VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, 1});
            cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                       {hizImageFromWriteToReadBarrier});

            const std::vector descSets{resources_->GetDescriptorSet(kHizReduceDescSetPrefix + std::to_string(i))};
            cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, hizReduceComputePipelineLayout_, 0, descSets);

            const auto groupCountX = CeilDiv(std::max(currentWindowWidth_ >> (i + 1), 1U), kHizBuildLocalSizeX);
            const auto groupCountY = CeilDiv(std::max(currentWindowHeight_ >> (i + 1), 1U), kHizBuildLocalSizeY);
            cmdBuffer->Dispatch(groupCountX, groupCountY, 1);
        }
    }

    // Barriers for depth image to DEPTH_STENCIL_ATTACHMENT_OPTIMAL and Hi-Z image to SHADER_READ_ONLY_OPTIMAL
    {
        const auto depthToAttachmentBarrier = depthImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_READ_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                   {depthToAttachmentBarrier});

        const auto hizImageToShaderReadBarrier = hizImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, hizMipCount_, 0, 1});
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                   {hizImageToShaderReadBarrier});
    }

    // Compute Pass: Occlusion culling
    {
        // Clear visibility array on every occlusion culling compute call
        cmdBuffer->FillBuffer(resources_->GetBuffer(kCullingResultsBuffer), 0, sizeof(std::uint32_t) * 4, 0);

        cmdBuffer->BindPipeline(occlusionComputePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        const std::vector descSets{resources_->GetDescriptorSet(kOcclusionDescSet)};
        cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, occlusionComputePipelineLayout_, 0, descSets);

        OcclusionPushConstants pushConstants{};
        pushConstants.viewProjMatrix = camera_->GetProjectionMatrix() * camera_->GetViewMatrix();
        pushConstants.hizBaseSize = glm::vec2(currentWindowWidth_, currentWindowHeight_);
        pushConstants.hizMipCount = hizMipCount_;
        pushConstants.objectCount = kTotalObjectCount;
        pushConstants.frustumCullingEnabled = isFrustumCullingEnabled_ ? 1U : 0U;
        pushConstants.occlusionCullingEnabled = isOcclusionCullingEnabled_ ? 1U : 0U;
        cmdBuffer->PushConstants(occlusionComputePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                 sizeof(OcclusionPushConstants), &pushConstants);
        const auto groupCountX = CeilDiv(kTotalObjectCount, kCullingLocalSize);
        cmdBuffer->Dispatch(groupCountX, 1, 1);
    }
}

void VulkanApplication::RecordAllCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    clearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Record command buffers for calculating Hi-Z Pyramid and filling culling results data
    RecordComputeCommandBuffers(currentCmdBuffer);

    // Render Pass: Draw culled scene objects
    {
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

        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        // Get visibility array's base pointer
        const std::uint32_t* visibilityArray = cullingResultsData_ + kVisibilityArrayResultIndex;

        currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        drawnObjectCount_ = 0;
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (!sceneObject.HasRenderable() || visibilityArray[sceneObject.GetObjectId()] == 0U) {
                return;
            }

            ++drawnObjectCount_;

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
        });
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateLightUBO() const
{
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
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

void VulkanApplication::ReportStats()
{
    if (cullingResultsData_ == nullptr) {
        return;
    }

    const double currentTime = GetCurrentTime();
    if (currentTime - lastStatsReportTime_ < 1.0) {
        return;
    }
    lastStatsReportTime_ = currentTime;

    std::cout << "Draw calls(current/total): " << std::setw(4) << drawnObjectCount_ << " / " << kTotalObjectCount - 1
              << "  | Frustum culled objects: " << std::setw(4) << cullingResultsData_[kFrustumCulledResultIndex]
              << "  | Occlusion culled objects: " << std::setw(4) << cullingResultsData_[kOcclusionCulledResultIndex]
              << std::endl;
}

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
