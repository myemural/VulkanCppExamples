/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
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

namespace examples::real_time_lighting::transparency_techniques::depth_peeling_transparency
{
using namespace constants;
using namespace common::scene;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationTransparencyTechniques(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationTransparencyTechniques::Init()) {
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
    ApplicationTransparencyTechniques::PreUpdate();

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
    layerCount_ = GetParamU32(AppSettings::PeelLayerCount);

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto sceneVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSceneVertexShaderFile);
    const auto opaqueFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kOpaqueFragmentShaderFile);
    const auto depthPeelingFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kDepthPeelingFragmentShaderFile);
    const auto compositionVertexShaderAsset = assetManager_->Load<ShaderAsset>(kCompositionVertexShaderFile);
    const auto compositionFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kCompositionFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
            {.name = kOpaqueFragmentShaderKey, .asset = assetManager_->Get(opaqueFragmentShaderAsset)},
            {.name = kDepthPeelingFragmentShaderKey, .asset = assetManager_->Get(depthPeelingFragmentShaderAsset)},
            {.name = kCompositionVertexShaderKey, .asset = assetManager_->Get(compositionVertexShaderAsset)},
            {.name = kCompositionFragmentShaderKey, .asset = assetManager_->Get(compositionFragmentShaderAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kOpaqueColorImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kOpaqueColorImageView, .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kDepthOpaqueImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = depthImageFormat_,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kDepthOpaqueImageView,
                                          .format = depthImageFormat_,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}}};

    for (auto i = 0U; i < layerCount_; ++i) {
        const std::string depthPeelColorImageName = kDepthPeelColorImage + std::to_string(i);
        const std::string depthPeelColorImageViewName = kDepthPeelColorImageView + std::to_string(i);
        const std::string depthPeelImageName = kDepthPeelImage + std::to_string(i);
        const std::string depthPeelImageViewName = kDepthPeelImageView + std::to_string(i);
        const std::string colorPeelImageName = kColorPeelImage + std::to_string(i);
        const std::string colorPeelImageViewName = kColorPeelImageView + std::to_string(i);

        resourceCreateInfo.images->push_back(ImageResourceCreateInfo{
            .name = depthPeelColorImageName,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R32_SFLOAT,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = depthPeelColorImageViewName, .format = VK_FORMAT_R32_SFLOAT}}});

        resourceCreateInfo.images->push_back(ImageResourceCreateInfo{
            .name = depthPeelImageName,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = depthImageFormat_,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = depthPeelImageViewName,
                                          .format = depthImageFormat_,
                                          .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}});

        resourceCreateInfo.images->push_back(
                ImageResourceCreateInfo{.name = colorPeelImageName,
                                        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
                                        .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                        .views = {ImageViewCreateInfo{.viewName = colorPeelImageViewName,
                                                                      .format = VK_FORMAT_R16G16B16A16_SFLOAT}}});
    }

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST}}};

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
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 2.0f, 7.0f), aspectRatio);

    // Add scene objects
    std::uint32_t index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);
    for (const auto& modelPos: modelPositions) {
        MeshMaterialData defaultMaterial{};
        defaultMaterial.diffuseColor = glm::vec4(GenerateRandomColor(0.1f, 1.0f), 1.0f);
        defaultMaterial.ambientStrength = GetParamFloat(AppSettings::AmbientStrength);
        defaultMaterial.shininess = GetParamFloat(AppSettings::Shininess);
        defaultMaterial.specularStrength = GetParamFloat(AppSettings::SpecularStrength);
        defaultMaterial.opacity = 1.0f;

        if (const auto value = GenerateRandomValue(0U, 1U); value == 0) {
            rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kCubeObject + std::to_string(index))
                                               .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                               .WithMaterial(defaultMaterial)
                                               .WithPosition(modelPos)
                                               .WithScale(glm::vec3{2.0f}));

        } else {
            defaultMaterial.opacity = GenerateRandomValue(0.1f, 0.8f);
            rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kPlaneObject + std::to_string(index))
                                               .WithTag(kTransparentObjectGroup)
                                               .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                               .WithMaterial(defaultMaterial)
                                               .WithPosition(modelPos)
                                               .WithEulerAngles(glm::vec3(90.0f, 0.0f, 0.0f))
                                               .WithScale(glm::vec3{4.0f}));
        }

        index++;
    }

    const auto& rootObject = rootObjectBuilder.Build();
    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 4 + 6 * layerCount_,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 + 2 * layerCount_},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 + layerCount_},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 * layerCount_ + 1}},
        .layouts =
                {{.name = kMainDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                 {.name = kPeelingDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}},
                 {.name = kCompositionDescSetLayout,
                  .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                               {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, layerCount_, VK_SHADER_STAGE_FRAGMENT_BIT,
                                nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kCompositionDescSet, .layoutName = kCompositionDescSetLayout}}};

    for (auto i = 0U; i < layerCount_; ++i) {
        const std::string descSetName = kPeelingDescSet + std::to_string(i);
        descriptorResourceCreateInfo.descriptorSets.emplace_back(descSetName, kPeelingDescSetLayout);
    }

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorImageInfo> compositionColorImageInfos;
    compositionColorImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kOpaqueColorImage, kOpaqueColorImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    BufferWriteRequest opaqueTransformBufferRequest;
    opaqueTransformBufferRequest.descriptorSetName = kMainDescSet;
    opaqueTransformBufferRequest.bindingIndex = 0;
    opaqueTransformBufferRequest.buffers = storageTransformBufferInfos;
    opaqueTransformBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest opaqueMaterialBufferRequest;
    opaqueMaterialBufferRequest.descriptorSetName = kMainDescSet;
    opaqueMaterialBufferRequest.bindingIndex = 1;
    opaqueMaterialBufferRequest.buffers = storageMaterialBufferInfos;
    opaqueMaterialBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.descriptorSetName = kMainDescSet;
    lightUboRequest.bindingIndex = 2;
    lightUboRequest.buffers = lightUboInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest compositionColorImageUpdateRequest;
    compositionColorImageUpdateRequest.descriptorSetName = kCompositionDescSet;
    compositionColorImageUpdateRequest.bindingIndex = 0;
    compositionColorImageUpdateRequest.images = compositionColorImageInfos;
    compositionColorImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {opaqueTransformBufferRequest, opaqueMaterialBufferRequest, lightUboRequest},
        .imageWriteRequests = {compositionColorImageUpdateRequest}};

    std::vector<VkDescriptorImageInfo> depthPeelingColorImageInfos;
    for (auto i = 0U; i < layerCount_; ++i) {
        const std::string currentPeelingColorImageName = kColorPeelImage + std::to_string(i);
        const std::string currentPeelingColorImageViewName = kColorPeelImageView + std::to_string(i);
        const auto currentPeelingColorImageView =
                resources_->GetImageView(currentPeelingColorImageName, currentPeelingColorImageViewName);
        depthPeelingColorImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                                 currentPeelingColorImageView->GetHandle(),
                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        const std::string descSetName = kPeelingDescSet + std::to_string(i);

        std::vector<VkDescriptorImageInfo> depthPeelingDepthImageInfos;
        const auto currentDepthOpaqueImageView = resources_->GetImageView(kDepthOpaqueImage, kDepthOpaqueImageView);
        std::vector<VkDescriptorImageInfo> depthPeelingOpaqueDepthImageInfos;
        depthPeelingOpaqueDepthImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                                       currentDepthOpaqueImageView->GetHandle(),
                                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        if (i == 0U) {
            depthPeelingDepthImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                                     currentDepthOpaqueImageView->GetHandle(),
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        } else {
            const std::string currentPeelingDepthImageName = kDepthPeelColorImage + std::to_string(i - 1);
            const std::string currentPeelingDepthImageViewName = kDepthPeelColorImageView + std::to_string(i - 1);
            const auto currentPeelingDepthImageView =
                    resources_->GetImageView(currentPeelingDepthImageName, currentPeelingDepthImageViewName);
            depthPeelingDepthImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                                     currentPeelingDepthImageView->GetHandle(),
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }


        BufferWriteRequest transparentTransformBufferRequest;
        transparentTransformBufferRequest.descriptorSetName = descSetName;
        transparentTransformBufferRequest.bindingIndex = 0;
        transparentTransformBufferRequest.buffers = storageTransformBufferInfos;
        transparentTransformBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        BufferWriteRequest transparentMaterialBufferRequest;
        transparentMaterialBufferRequest.descriptorSetName = descSetName;
        transparentMaterialBufferRequest.bindingIndex = 1;
        transparentMaterialBufferRequest.buffers = storageMaterialBufferInfos;
        transparentMaterialBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        BufferWriteRequest transparentLightUboRequest;
        transparentLightUboRequest.descriptorSetName = descSetName;
        transparentLightUboRequest.bindingIndex = 2;
        transparentLightUboRequest.buffers = lightUboInfos;
        transparentLightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        ImageWriteRequest peelingDepthImageUpdateRequest;
        peelingDepthImageUpdateRequest.descriptorSetName = descSetName;
        peelingDepthImageUpdateRequest.bindingIndex = 3;
        peelingDepthImageUpdateRequest.images = depthPeelingDepthImageInfos;
        peelingDepthImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        ImageWriteRequest peelingOpaqueDepthImageUpdateRequest;
        peelingOpaqueDepthImageUpdateRequest.descriptorSetName = descSetName;
        peelingOpaqueDepthImageUpdateRequest.bindingIndex = 4;
        peelingOpaqueDepthImageUpdateRequest.images = depthPeelingOpaqueDepthImageInfos;
        peelingOpaqueDepthImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        descriptorSetUpdateInfo.bufferWriteRequests.push_back(transparentTransformBufferRequest);
        descriptorSetUpdateInfo.bufferWriteRequests.push_back(transparentMaterialBufferRequest);
        descriptorSetUpdateInfo.bufferWriteRequests.push_back(transparentLightUboRequest);
        descriptorSetUpdateInfo.imageWriteRequests.push_back(peelingDepthImageUpdateRequest);
        descriptorSetUpdateInfo.imageWriteRequests.push_back(peelingOpaqueDepthImageUpdateRequest);
    }

    ImageWriteRequest compositionPeelingColorUpdateRequest;
    compositionPeelingColorUpdateRequest.descriptorSetName = kCompositionDescSet;
    compositionPeelingColorUpdateRequest.bindingIndex = 1;
    compositionPeelingColorUpdateRequest.images = depthPeelingColorImageInfos;
    compositionPeelingColorUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    descriptorSetUpdateInfo.imageWriteRequests.push_back(compositionPeelingColorUpdateRequest);

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
    VkAttachmentReference opaqueColorAttachmentRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference opaqueDepthAttachmentRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    opaqueRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                .AddAttachment([&](auto& attachmentCreateInfo) {
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
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &opaqueColorAttachmentRef;
                    subpassCreateInfo.pDepthStencilAttachment = &opaqueDepthAttachmentRef;
                });
    });

    if (!opaqueRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for transparency accumulation)!");
    }

    std::array peelColorAttachmentRefs = {VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                          VkAttachmentReference{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
    VkAttachmentReference peelDepthAttachmentRef = {2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    peelRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = VK_FORMAT_R32_SFLOAT;
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
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = peelColorAttachmentRefs.size();
                    subpassCreateInfo.pColorAttachments = peelColorAttachmentRefs.data();
                    subpassCreateInfo.pDepthStencilAttachment = &peelDepthAttachmentRef;
                });
    });

    if (!peelRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for transparency accumulation)!");
    }

    VkAttachmentReference compositeColorAttachment{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    compositeRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                    subpassCreateInfo.pColorAttachments = &compositeColorAttachment;
                });
    });

    if (!compositeRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for composition)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange meshPushConstant;
    meshPushConstant.offset = 0;
    meshPushConstant.size = sizeof(MeshPushConstants);
    meshPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    opaquePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {meshPushConstant});

    if (!opaquePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for opaque objects)!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    VkPipelineColorBlendAttachmentState opaqueBlend{};
    opaqueBlend.blendEnable = VK_FALSE;
    opaqueBlend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    opaquePipeline_ = device_->CreateGraphicsPipeline(opaquePipelineLayout_, opaqueRenderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kOpaqueFragmentShaderKey)->GetHandle();
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
            blendStateCreateInfo.pAttachments = &opaqueBlend;
        });
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!opaquePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for opaque objects)!");
    }

    depthPeelingPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kPeelingDescSetLayout)}, {meshPushConstant});

    if (!depthPeelingPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for transparent objects)!");
    }

    std::array peelingPipelineColorBlends{
        VkPipelineColorBlendAttachmentState{.blendEnable = VK_FALSE,
                                            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT},
        VkPipelineColorBlendAttachmentState{.blendEnable = VK_FALSE, .colorWriteMask = VK_COLOR_COMPONENT_R_BIT}};

    depthPeelingPipeline_ =
            device_->CreateGraphicsPipeline(depthPeelingPipelineLayout_, peelRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kDepthPeelingFragmentShaderKey)->GetHandle();
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
                    blendStateCreateInfo.attachmentCount = peelingPipelineColorBlends.size();
                    blendStateCreateInfo.pAttachments = peelingPipelineColorBlends.data();
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                });
            });

    if (!depthPeelingPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for transparency accumulation)!");
    }

    VkPushConstantRange compositionPushConstant;
    compositionPushConstant.offset = 0;
    compositionPushConstant.size = sizeof(CompositionPushConstants);
    compositionPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    compositePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kCompositionDescSetLayout)}, {compositionPushConstant});

    if (!compositePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for composition)!");
    }

    VkPipelineColorBlendAttachmentState compositePipelineColorBlend{};
    compositePipelineColorBlend.blendEnable = VK_FALSE;
    compositePipelineColorBlend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    compositePipeline_ =
            device_->CreateGraphicsPipeline(compositePipelineLayout_, compositeRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kCompositionVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kCompositionFragmentShaderKey)->GetHandle();
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
                    blendStateCreateInfo.pAttachments = &compositePipelineColorBlend;
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
                });
            });

    if (!compositePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for composition)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Opaque framebuffer
    const auto& opaqueColorImageView = resources_->GetImageView(kOpaqueColorImage, kOpaqueColorImageView);
    const auto& opaqueDepthImageView = resources_->GetImageView(kDepthOpaqueImage, kDepthOpaqueImageView);

    opaqueFramebuffer_ = device_->CreateFramebuffer(
            opaqueRenderPass_, {opaqueColorImageView, opaqueDepthImageView},
            [&](auto& builder) { builder.SetDimensions(currentWindowWidth_, currentWindowHeight_); });

    if (!opaqueFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer for opaque objects!");
    }

    // Depth peeling framebuffers
    peelFramebuffers_.resize(layerCount_);
    for (uint32_t layer = 0; layer < layerCount_; ++layer) {
        const auto& colorView = resources_->GetImageView(kColorPeelImage + std::to_string(layer),
                                                         kColorPeelImageView + std::to_string(layer));
        const auto& depthColorView = resources_->GetImageView(kDepthPeelColorImage + std::to_string(layer),
                                                              kDepthPeelColorImageView + std::to_string(layer));
        const auto& depthView = resources_->GetImageView(kDepthPeelImage + std::to_string(layer),
                                                         kDepthPeelImageView + std::to_string(layer));

        peelFramebuffers_[layer] =
                device_->CreateFramebuffer(peelRenderPass_, {colorView, depthColorView, depthView}, [&](auto& builder) {
                    builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
                });

        if (!peelFramebuffers_[layer]) {
            throw std::runtime_error("Failed to create framebuffer for depth peeling layer " + std::to_string(layer));
        }
    }

    // Present framebuffers
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(compositeRenderPass_, {swapImage}, [&](auto& builder) {
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
    std::array<VkClearValue, 2> opaqueClearValues{};
    opaqueClearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    opaqueClearValues[1].depthStencil = {1.0f, 0};

    std::array<VkClearValue, 3> peelClearValues{};
    peelClearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
    peelClearValues[1].color = {1.0f, 0.0f, 0.0f, 0.0f};
    peelClearValues[2].depthStencil = {1.0f, 0};

    VkClearValue compositeClear{};
    compositeClear.color = {0.0f, 0.0f, 0.0f, 0.0f};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    // Opaque Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = opaqueRenderPass_->GetHandle();
                    beginInfo.framebuffer = opaqueFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = opaqueClearValues.size();
                    beginInfo.pClearValues = opaqueClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, opaquePipelineLayout_, 0, descSets);

        // Draw opaque objects
        currentCmdBuffer->BindPipeline(opaquePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        scene_->Traverse([&](const SceneObject& sceneObject) {
            if (sceneObject.HasRenderable() && sceneObject.GetTag() != kTransparentObjectGroup) {
                const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                MeshPushConstants meshPushConstants{};
                meshPushConstants.objectId = sceneObject.GetObjectId();
                meshPushConstants.view = camera_->GetViewMatrix();
                meshPushConstants.projection = camera_->GetProjectionMatrix();
                meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
                currentCmdBuffer->PushConstants(opaquePipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(meshPushConstants), &meshPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // Depth peeling pass
    {
        for (uint32_t layer = 0; layer < layerCount_; ++layer) {
            auto& currentPeelFramebuffer = peelFramebuffers_[layer];
            currentCmdBuffer->BeginRenderPass(
                    [&](auto& beginInfo) {
                        beginInfo.renderPass = peelRenderPass_->GetHandle();
                        beginInfo.framebuffer = currentPeelFramebuffer->GetHandle();
                        beginInfo.renderArea.offset = {0, 0};
                        beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                        beginInfo.clearValueCount = peelClearValues.size();
                        beginInfo.pClearValues = peelClearValues.data();
                    },
                    VK_SUBPASS_CONTENTS_INLINE);


            const std::vector descSets{resources_->GetDescriptorSet(kPeelingDescSet + std::to_string(layer))};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, depthPeelingPipelineLayout_, 0,
                                                 descSets);

            currentCmdBuffer->BindPipeline(depthPeelingPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
            scene_->Traverse([&](const SceneObject& sceneObject) {
                if (sceneObject.HasRenderable() && sceneObject.GetTag() == kTransparentObjectGroup) {
                    const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
                    currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
                    currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

                    MeshPushConstants meshPushConstants{};
                    meshPushConstants.objectId = sceneObject.GetObjectId();
                    meshPushConstants.view = camera_->GetViewMatrix();
                    meshPushConstants.projection = camera_->GetProjectionMatrix();
                    meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
                    meshPushConstants.layerIndex = layer;
                    currentCmdBuffer->PushConstants(depthPeelingPipelineLayout_,
                                                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                    sizeof(meshPushConstants), &meshPushConstants);
                    currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
                }
            });

            currentCmdBuffer->EndRenderPass();
        }
    }

    // Composite pass
    {

        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = compositeRenderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &compositeClear;
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector compositeDescSets{resources_->GetDescriptorSet(kCompositionDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipelineLayout_, 0,
                                             compositeDescSets);
        currentCmdBuffer->BindPipeline(compositePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        CompositionPushConstants compositionPushConstants{};
        compositionPushConstants.layerCount = layerCount_;
        currentCmdBuffer->PushConstants(compositePipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(compositionPushConstants), &compositionPushConstants);

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
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightDirection), 1.0f);
    lightUbo.lightColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightColor), 1.0f);
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
} // namespace examples::real_time_lighting::transparency_techniques::depth_peeling_transparency
