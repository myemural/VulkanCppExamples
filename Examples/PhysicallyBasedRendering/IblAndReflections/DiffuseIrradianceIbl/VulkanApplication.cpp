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

namespace examples::physically_based_rendering::ibl_and_reflections::diffuse_irradiance_ibl
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationIblAndReflections(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationIblAndReflections::Init()) {
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
        CreateDefaultFramebuffers(resources_->GetImageView(kDepthImage, kDepthImageView));
        CreateCubemapFramebuffers();
        CreateCommandBuffers();
        ConvertEquirectangularToCubemap();
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

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
    assetManager_->RegisterLoader<TextureAssetHDR>(std::make_unique<TextureLoaderHDR>(ASSETS_DIR));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto sceneObjectsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneObjectsFragmentShaderFile);
    const auto skyboxVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSkyboxVertexShaderFile);
    const auto convertToCubemapFragmentShaderAsset =
            assetManager_->Load<ShaderAsset>(kConvertToCubemapFragmentShaderFile);
    const auto irradianceConvolutionFragmentShaderAsset =
            assetManager_->Load<ShaderAsset>(kIrradianceConvolutionFragmentShaderFile);
    const auto skyboxFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSkyboxFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
            {.name = kSceneObjectsFragmentShaderKey, .asset = assetManager_->Get(sceneObjectsFragmentShaderAsset)},
            {.name = kSkyboxVertexShaderKey, .asset = assetManager_->Get(skyboxVertexShaderAsset)},
            {.name = kConvertToCubemapFragmentShaderKey,
             .asset = assetManager_->Get(convertToCubemapFragmentShaderAsset)},
            {.name = kIrradianceConvolutionFragmentShaderKey,
             .asset = assetManager_->Get(irradianceConvolutionFragmentShaderAsset)},
            {.name = kSkyboxFragmentShaderKey, .asset = assetManager_->Get(skyboxFragmentShaderAsset)}}};

    resourceCreateInfo.images =
            {ImageResourceCreateInfo{
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
                 .name = kEnvironmentCubemapImage,
                 .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                 .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                 .dimensions = {kEnvironmentCubemapSize, kEnvironmentCubemapSize, 1},
                 .arrayLayers = 6,
                 .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                 .views =
                         {
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageView,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
                                                 .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewRight,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewLeft,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewTop,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format =
                                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 2, 1}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewBottom,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format =
                                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3, 1}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewBack,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format =
                                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 4, 1}},
                             ImageViewCreateInfo{.viewName = kEnvironmentCubemapImageViewFront,
                                                 .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                 .format =
                                                         VK_FORMAT_R32G32B32A32_SFLOAT,
                                                 .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 5, 1}},
                         }},
             ImageResourceCreateInfo{.name = kIrradianceCubemapImage,
                                     .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                                     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                     .dimensions = {kIrradianceCubemapSize, kIrradianceCubemapSize, 1},
                                     .arrayLayers = 6,
                                     .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                     .views = {
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageView,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewRight,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewLeft,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewTop,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 2, 1}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewBottom,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3, 1}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewBack,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 4, 1}},
                                         ImageViewCreateInfo{.viewName = kIrradianceCubemapImageViewFront,
                                                             .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                             .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                                             .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 5, 1}},
                                     }}};

    resourceCreateInfo.samplers = {
        SamplerResourceCreateInfo{.name = kMainSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}},
        SamplerResourceCreateInfo{.name = kSkyboxSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR},
                                  .addressModes = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE}}};

    CreateVulkanResources(resourceCreateInfo);
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
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, orbitDistance_), aspectRatio);

    const auto hdrEnvironmentTextureAsset = assetManager_->Load<TextureAssetHDR>(kHdrEnvironmentTexturePath);
    [[maybe_unused]] const auto hdrEnvironmentTextureId = sceneImageStorage.StoreTexture(
            kHdrEnvironmentTexture, kSkyboxSampler, assetManager_->Get(hdrEnvironmentTextureAsset));

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject).WithPosition(glm::vec3{0.0f, 0.0f, 0.0f});

    const std::vector spherePositions = {
        glm::vec3{-1.5f, 1.5f, 0.0f},
        glm::vec3{1.5f, 1.5f, 0.0f},
        glm::vec3{-1.5f, -1.5f, 0.0f},
        glm::vec3{1.5f, -1.5f, 0.0f},
    };

    const std::vector sphereRoughnessValues = {
        0.4f,
        0.9f,
        0.2f,
        1.0f,
    };

    const std::vector sphereMetallicValues = {
        0.9f,
        0.8f,
        0.1f,
        0.1f,
    };

    for (auto i = 0U; i < spherePositions.size(); ++i) {
        Material sphereMaterial;
        sphereMaterial.albedoColor = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
        sphereMaterial.roughness = sphereRoughnessValues.at(i);
        sphereMaterial.metallic = sphereMetallicValues.at(i);

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kSphereObject)
                                           .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                           .WithMaterial(sphereMaterial)
                                           .WithPosition(spherePositions.at(i))
                                           .WithScale(glm::vec3{2.0f}));
    }
    rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kSkyboxCubeObject)
                                       .WithTag(kSkyboxObjectGroup)
                                       .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                       .WithMaterial(Material{})
                                       .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f}));

    scene_->AddRootObject(rootObjectBuilder.Build());
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3 + combinedImageSamplerCount + 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
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
                    {.name = kSkyboxDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kConvertToCubemapDescSet, .layoutName = kSkyboxDescSetLayout},
                           {.name = kIrradianceConvolutionDescSet, .layoutName = kSkyboxDescSetLayout},
                           {.name = kSkyboxDescSet, .layoutName = kSkyboxDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

    auto hdrEnvironmentImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfo(kHdrEnvironmentTexture);

    std::vector<VkDescriptorImageInfo> environmentCubemapImageInfos;
    environmentCubemapImageInfos.emplace_back(
            resources_->GetSampler(kSkyboxSampler)->GetHandle(),
            resources_->GetImageView(kEnvironmentCubemapImage, kEnvironmentCubemapImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> irradianceCubemapImageInfos;
    irradianceCubemapImageInfos.emplace_back(
            resources_->GetSampler(kSkyboxSampler)->GetHandle(),
            resources_->GetImageView(kIrradianceCubemapImage, kIrradianceCubemapImageView)->GetHandle(),
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

    ImageWriteRequest irradianceMapUpdateRequest;
    irradianceMapUpdateRequest.descriptorSetName = kMainDescSet;
    irradianceMapUpdateRequest.bindingIndex = 4;
    irradianceMapUpdateRequest.images = irradianceCubemapImageInfos;
    irradianceMapUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest cubemapUpdateRequest;
    cubemapUpdateRequest.descriptorSetName = kConvertToCubemapDescSet;
    cubemapUpdateRequest.bindingIndex = 0;
    cubemapUpdateRequest.images = hdrEnvironmentImageInfos;
    cubemapUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest irradianceConvolutionUpdateRequest;
    irradianceConvolutionUpdateRequest.descriptorSetName = kIrradianceConvolutionDescSet;
    irradianceConvolutionUpdateRequest.bindingIndex = 0;
    irradianceConvolutionUpdateRequest.images = environmentCubemapImageInfos;
    irradianceConvolutionUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest skyboxUpdateRequest;
    skyboxUpdateRequest.descriptorSetName = kSkyboxDescSet;
    skyboxUpdateRequest.bindingIndex = 0;
    skyboxUpdateRequest.images = environmentCubemapImageInfos;
    skyboxUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest},
        .imageWriteRequests = {textureUpdateRequest, irradianceMapUpdateRequest, cubemapUpdateRequest,
                               irradianceConvolutionUpdateRequest, skyboxUpdateRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = static_cast<float>(currentWindowWidth_) / 2.0f;
    lastY_ = static_cast<float>(currentWindowHeight_) / 2.0f;

    window_->DisableCursor();

    // For moving yaw and pitch
    window_->OnMouseMove([this](const MouseMoveEvent& event) {
        const auto xPos = static_cast<float>(event.x);
        const auto yPos = static_cast<float>(event.y);

        if (firstMouseTriggered_) {
            lastX_ = xPos;
            lastY_ = yPos;
            firstMouseTriggered_ = false;
            return;
        }

        const float xOffset = xPos - lastX_;
        const float yOffset = lastY_ - yPos;

        lastX_ = xPos;
        lastY_ = yPos;

        const float sensitivity = GetParamFloat(AppSettings::MouseSensitivity) * static_cast<float>(deltaTime_);

        const float yawSensitivity = sensitivity;
        const float pitchSensitivity = sensitivity * 0.5f; // Should less sensitive than yaw

        orbitYaw_ += xOffset * yawSensitivity;
        orbitPitch_ += yOffset * pitchSensitivity;
        orbitPitch_ = glm::clamp(orbitPitch_, -89.0f, 89.0f);

        const float yawRad = glm::radians(orbitYaw_);
        const float pitchRad = glm::radians(orbitPitch_);

        glm::vec3 position;
        position.x = orbitTarget_.x + orbitDistance_ * cosf(pitchRad) * cosf(yawRad);
        position.y = orbitTarget_.y + orbitDistance_ * sinf(pitchRad);
        position.z = orbitTarget_.z + orbitDistance_ * cosf(pitchRad) * sinf(yawRad);

        camera_->SetPosition(position);

        // Look to target
        const glm::vec3 front = glm::normalize(orbitTarget_ - position);

        const float viewYaw = glm::degrees(atan2(front.z, front.x));
        const float viewPitch = glm::degrees(asin(front.y));

        camera_->SetRotation(viewYaw, viewPitch);
    });

    // For zooming to target
    window_->OnMouseScroll([this](const MouseScrollEvent& event) {
        constexpr auto minZoom = 0.2f;
        constexpr auto maxZoom = 90.0f;
        orbitDistance_ = glm::clamp(orbitDistance_ - static_cast<float>(event.deltaY) *
                                                             GetParamFloat(AppSettings::CameraZoomSpeed),
                                    minZoom, maxZoom);

        const float yawRad = glm::radians(orbitYaw_);
        const float pitchRad = glm::radians(orbitPitch_);

        glm::vec3 newPosition;
        newPosition.x = orbitTarget_.x + orbitDistance_ * cosf(pitchRad) * cosf(yawRad);
        newPosition.y = orbitTarget_.y + orbitDistance_ * sinf(pitchRad);
        newPosition.z = orbitTarget_.z + orbitDistance_ * cosf(pitchRad) * sinf(yawRad);

        camera_->SetPosition(newPosition);
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

    cubemapRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
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
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                });
    });

    if (!cubemapRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for creating cubemap)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(ScenePushConstants);
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

    VkViewport cubemapViewport{
        0, 0, static_cast<float>(kEnvironmentCubemapSize), static_cast<float>(kEnvironmentCubemapSize), 0.0f, 1.0f};
    VkRect2D cubemapScissor{0, 0, kEnvironmentCubemapSize, kEnvironmentCubemapSize};

    VkPushConstantRange skyboxPushConstant;
    skyboxPushConstant.offset = 0;
    skyboxPushConstant.size = sizeof(SkyboxPushConstants);
    skyboxPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutSkybox_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kSkyboxDescSetLayout)},
                                                          {skyboxPushConstant});

    if (!pipelineLayoutSkybox_) {
        throw std::runtime_error("Failed to create pipeline layout (for skybox)!");
    }

    convertToCubemapPipeline_ =
            device_->CreateGraphicsPipeline(pipelineLayoutSkybox_, cubemapRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kConvertToCubemapFragmentShaderKey)->GetHandle();
                });
                builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
                    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
                    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
                    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
                });
                builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                    viewportStateCreateInfo.viewportCount = 1;
                    viewportStateCreateInfo.pViewports = &cubemapViewport;
                    viewportStateCreateInfo.scissorCount = 1;
                    viewportStateCreateInfo.pScissors = &cubemapScissor;
                });
                builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
                    blendStateCreateInfo.attachmentCount = 1;
                    blendStateCreateInfo.pAttachments = &colorBlendAttachment;
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                });
            });

    if (!convertToCubemapPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for converting to cubemap)!");
    }

    VkViewport irradianceCubemapViewport{
        0, 0, static_cast<float>(kIrradianceCubemapSize), static_cast<float>(kIrradianceCubemapSize), 0.0f, 1.0f};
    VkRect2D irradianceCubemapScissor{0, 0, kIrradianceCubemapSize, kIrradianceCubemapSize};

    irradianceConvolutionPipeline_ =
            device_->CreateGraphicsPipeline(pipelineLayoutSkybox_, cubemapRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kIrradianceConvolutionFragmentShaderKey)->GetHandle();
                });
                builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
                    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
                    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
                    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
                });
                builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                    viewportStateCreateInfo.viewportCount = 1;
                    viewportStateCreateInfo.pViewports = &irradianceCubemapViewport;
                    viewportStateCreateInfo.scissorCount = 1;
                    viewportStateCreateInfo.pScissors = &irradianceCubemapScissor;
                });
                builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
                    blendStateCreateInfo.attachmentCount = 1;
                    blendStateCreateInfo.pAttachments = &colorBlendAttachment;
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                });
            });

    if (!irradianceConvolutionPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for convolution)!");
    }

    skyboxPipeline_ = device_->CreateGraphicsPipeline(pipelineLayoutSkybox_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxFragmentShaderKey)->GetHandle();
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
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        });
    });

    if (!skyboxPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for skybox)!");
    }
}

void VulkanApplication::CreateCubemapFramebuffers()
{
    const std::vector<std::string> cubemapImageViewKeys = {
        kEnvironmentCubemapImageViewRight,  kEnvironmentCubemapImageViewLeft, kEnvironmentCubemapImageViewTop,
        kEnvironmentCubemapImageViewBottom, kEnvironmentCubemapImageViewBack, kEnvironmentCubemapImageViewFront};

    for (auto i = 0U; i < cubemapImageViewKeys.size(); ++i) {
        const auto& environmentCubemapImageView =
                resources_->GetImageView(kEnvironmentCubemapImage, cubemapImageViewKeys[i]);

        environmentCubemapFramebuffers_[i] =
                device_->CreateFramebuffer(cubemapRenderPass_, {environmentCubemapImageView}, [&](auto& builder) {
                    builder.SetDimensions(kEnvironmentCubemapSize, kEnvironmentCubemapSize);
                });

        if (!environmentCubemapFramebuffers_[i]) {
            throw std::runtime_error("Failed to create cubemap framebuffers!");
        }
    }

    const std::vector<std::string> irradianceCubemapImageViewKeys = {
        kIrradianceCubemapImageViewRight,  kIrradianceCubemapImageViewLeft, kIrradianceCubemapImageViewTop,
        kIrradianceCubemapImageViewBottom, kIrradianceCubemapImageViewBack, kIrradianceCubemapImageViewFront};

    for (auto i = 0U; i < irradianceCubemapImageViewKeys.size(); ++i) {
        const auto& irradianceCubemapImageView =
                resources_->GetImageView(kIrradianceCubemapImage, irradianceCubemapImageViewKeys[i]);

        irradianceCubemapFramebuffers_[i] =
                device_->CreateFramebuffer(cubemapRenderPass_, {irradianceCubemapImageView}, [&](auto& builder) {
                    builder.SetDimensions(kIrradianceCubemapSize, kIrradianceCubemapSize);
                });

        if (!irradianceCubemapFramebuffers_[i]) {
            throw std::runtime_error("Failed to create cubemap framebuffers!");
        }
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::ConvertEquirectangularToCubemap()
{
    const auto cubemapCamera = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f), 1.0f, 90.0f);
    const auto cubemapViewMatrices = cubemapCamera->GetCubemapViewMatrices();
    auto cubemapProj = cubemapCamera->GetProjectionMatrix();
    cubemapProj[1][1] *= -1; // Revert this projection flip for reflection render

    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};

    const auto cmdBuffer = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    auto currentCmdBuffer = cmdBuffer.front();

    if (!currentCmdBuffer->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    // Convert equirectangular to cubemap
    for (uint32_t face = 0; face < environmentCubemapFramebuffers_.size(); face++) {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = cubemapRenderPass_->GetHandle();
                    beginInfo.framebuffer = environmentCubemapFramebuffers_[face]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(kEnvironmentCubemapSize, kEnvironmentCubemapSize);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        currentCmdBuffer->BindPipeline(convertToCubemapPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable() && sceneObject.GetTag() == kSkyboxObjectGroup) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                const std::vector skyboxDescSets{resources_->GetDescriptorSet(kConvertToCubemapDescSet)};
                currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0,
                                                     skyboxDescSets);

                SkyboxPushConstants skyboxPushConstants{};
                skyboxPushConstants.view = cubemapViewMatrices[face];
                skyboxPushConstants.projection = cubemapProj;
                currentCmdBuffer->PushConstants(pipelineLayoutSkybox_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                sizeof(skyboxPushConstants), &skyboxPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // Barrier for using environment cubemap in irradiance convolution
    {
        const auto environmentCubemapImage = resources_->GetImage(kEnvironmentCubemapImage);
        const auto environmentCubemapBarrier = environmentCubemapImage->CreateImageMemoryBarrier(
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {environmentCubemapBarrier});
    }

    // Calculate irradiance convolution cubemap
    for (uint32_t face = 0; face < irradianceCubemapFramebuffers_.size(); face++) {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = cubemapRenderPass_->GetHandle();
                    beginInfo.framebuffer = irradianceCubemapFramebuffers_[face]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(kIrradianceCubemapSize, kIrradianceCubemapSize);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        currentCmdBuffer->BindPipeline(irradianceConvolutionPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable() && sceneObject.GetTag() == kSkyboxObjectGroup) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                const std::vector skyboxDescSets{resources_->GetDescriptorSet(kIrradianceConvolutionDescSet)};
                currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0,
                                                     skyboxDescSets);

                SkyboxPushConstants skyboxPushConstants{};
                skyboxPushConstants.view = cubemapViewMatrices[face];
                skyboxPushConstants.projection = cubemapProj;
                currentCmdBuffer->PushConstants(pipelineLayoutSkybox_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                sizeof(skyboxPushConstants), &skyboxPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    queue_->Submit({currentCmdBuffer});
    queue_->WaitIdle();
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

    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    // Draw skybox
    {
        currentCmdBuffer->BindPipeline(skyboxPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable() && sceneObject.GetTag() == kSkyboxObjectGroup) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                const std::vector skyboxDescSets{resources_->GetDescriptorSet(kSkyboxDescSet)};
                currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0,
                                                     skyboxDescSets);

                SkyboxPushConstants skyboxPushConstants{};
                skyboxPushConstants.view = camera_->GetViewMatrix();
                skyboxPushConstants.projection = camera_->GetProjectionMatrix();
                currentCmdBuffer->PushConstants(pipelineLayoutSkybox_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                sizeof(skyboxPushConstants), &skyboxPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });
    }

    // Draw only scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() != kSkyboxObjectGroup) {
            const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);

            ScenePushConstants scenePushConstants{};
            scenePushConstants.objectId = sceneObject.GetObjectId();
            scenePushConstants.view = camera_->GetViewMatrix();
            scenePushConstants.projection = camera_->GetProjectionMatrix();
            scenePushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            0, sizeof(scenePushConstants), &scenePushConstants);
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
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
    lightUbo.lightColorAndIntensity = glm::vec4(kLightColor, kLightIntensity);
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));
}
} // namespace examples::physically_based_rendering::ibl_and_reflections::diffuse_irradiance_ibl
