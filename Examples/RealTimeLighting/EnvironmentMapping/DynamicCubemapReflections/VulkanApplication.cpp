/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "TimeUtils.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
{
using namespace constants;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationEnvironmentMapping(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationEnvironmentMapping::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        CreateInitialResources();
        BuildScene();
        CreateAndUpdateDescriptorSets();

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
        CreateDefaultFramebuffers(resources_->GetImageView(kDepthImage, kDepthImageView));
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
    ApplicationEnvironmentMapping::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.shaders = {
        .basePath = SHADERS_DIR,
        .shaderType = SHADER_TYPE,
        .modules = {{.name = kMainVertexShaderKey, .fileName = kMainVertexShaderFile},
                    {.name = kSceneObjectsFragmentShaderKey, .fileName = kSceneObjectsFragmentShaderFile},
                    {.name = kSkyboxVertexShaderKey, .fileName = kSkyboxVertexShaderFile},
                    {.name = kSkyboxFragmentShaderKey, .fileName = kSkyboxFragmentShaderFile},
                    {.name = kReflectionFragmentShaderKey, .fileName = kReflectionFragmentShaderFile}}};

    resourceCreateInfo
            .images = {ImageResourceCreateInfo{
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
                           .name = kReflectionCubemapImage,
                           .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           .createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                           .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                           .dimensions = {CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION, 1},
                           .arrayLayers = 6,
                           .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                           .views =
                                   {
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageView,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
                                                           .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewRight,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewLeft,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewTop,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format =
                                                                   VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 2, 1}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewBottom,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format =
                                                                   VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3, 1}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewBack,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format =
                                                                   VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 4, 1}},
                                       ImageViewCreateInfo{.viewName = kReflectionCubemapImageViewFront,
                                                           .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                           .format =
                                                                   VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           .subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 5, 1}},
                                   }},
                       ImageResourceCreateInfo{
                           .name = kReflectionDepthImage,
                           .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           .format = depthImageFormat_,
                           .dimensions = {CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION, 1},
                           .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                           .views = {ImageViewCreateInfo{.viewName = kReflectionDepthImageView,
                                                         .format = depthImageFormat_,
                                                         .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                                              .baseMipLevel = 0,
                                                                              .levelCount = 1,
                                                                              .baseArrayLayer = 0,
                                                                              .layerCount = 1}}}}};

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
    sceneConfig.attributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TEXCOORD, AccessorType::VEC2);
    sceneConfig.attributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TANGENT, AccessorType::VEC4);
    sceneConfig.currentMaterialSystem = MaterialSystem::PHONG_TEXTURED;

    materialManager_ = std::make_unique<MaterialManager>(*resources_, cmdPool_, queue_, ASSETS_DIR);
    scene_ = std::make_unique<SceneManager>(*resources_, *materialManager_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    scene_->AddPerspectiveCamera(kCameraObject, glm::vec3(0.0f, 1.0f, 7.0f), aspectRatio);
    camera_ = std::dynamic_pointer_cast<PerspectiveCamera>(scene_->GetActiveCamera());
    cubemapCamera_ = std::make_unique<PerspectiveCamera>(kReflectiveObjectPosition, 1.0f, 90.0f);

    // Materials
    materialManager_->LoadTexture(kWallStoneTexture, kMainSampler, kWallStoneTexturePath);
    materialManager_->LoadTexture(kWallStoneNormalTexture, kMainSampler, kWallStoneNormalTexturePath,
                                  VK_FORMAT_R8G8B8A8_UNORM);
    materialManager_->LoadCubemapTexture(kCubemapTexture, kSkyboxSampler, kCubemapRightTexturePath,
                                         kCubemapLeftTexturePath, kCubemapTopTexturePath, kCubemapBottomTexturePath,
                                         kCubemapBackTexturePath, kCubemapFrontTexturePath);

    const auto defaultMatName = kDefaultMaterial;
    materialManager_->CreatePhongTexturedMaterial(defaultMatName)
            .SetAmbientStrength(GetParamFloat(AppSettings::AmbientStrength))
            .SetSpecularStrength(GetParamFloat(AppSettings::SpecularStrength))
            .SetShininess(GetParamFloat(AppSettings::Shininess))
            .SetDiffuseMap(kWallStoneTexture)
            .SetNormalMap(kWallStoneNormalTexture)
            .SetReflectivity(0.0f) // No reflection
            .Build();

    const auto reflectiveMatName = kReflectiveMaterial;
    materialManager_->CreatePhongTexturedMaterial(reflectiveMatName)
            .SetAmbientStrength(GetParamFloat(AppSettings::AmbientStrength))
            .SetSpecularStrength(GetParamFloat(AppSettings::SpecularStrength))
            .SetShininess(GetParamFloat(AppSettings::Shininess))
            .SetDiffuseColor(glm::vec3(1.0f, 0.0f, 0.0f)) // Red diffuse color
            .SetReflectivity(0.9f)                        // High reflection
            .Build();

    // Add moving object
    for (auto i = 0U; i < MOVING_OBJECT_COUNT; ++i) {
        scene_->AddCube(kCubeObject + std::to_string(i), glm::vec3{-1.5f, -0.5f, 0.0f});
        scene_->SetMaterial(kCubeObject + std::to_string(i), defaultMatName);
    }

    // Add stationary objects
    scene_->AddPlane(kPlaneObject, glm::vec3{0.0f, -2.0f, 0.0f}, glm::vec3(0.0f), glm::vec3{3.0f});
    scene_->SetMaterial(kPlaneObject, defaultMatName);

    // Add reflective object
    scene_->AddSphere(kSphereObject, kReflectiveObjectPosition);
    scene_->SetMaterial(kSphereObject, reflectiveMatName);

    // Add skybox cube
    scene_->AddCube(kSkyboxCubeObject);
    scene_->AddToGroup(kSkyboxObjectGroup, {kSkyboxCubeObject});
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = materialManager_->GetTextureCount();
    const auto cubemapCount = materialManager_->GetCubemapTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2 + combinedImageSamplerCount + cubemapCount + 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + cubemapCount + 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kSkyboxDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kSkyboxDescSet, .layoutName = kSkyboxDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageBufferInfos;
    storageBufferInfos.emplace_back(scene_->GetStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = materialManager_->GetDescriptorImageInfos();

    auto cubemapImageInfos = materialManager_->GetCubemapDescriptorImageInfo(kCubemapTexture);

    std::vector<VkDescriptorImageInfo> reflectionImageInfos;
    reflectionImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kReflectionCubemapImage, kReflectionCubemapImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    BufferWriteRequest objectStorageBufferRequest;
    objectStorageBufferRequest.descriptorSetName = kMainDescSet;
    objectStorageBufferRequest.bindingIndex = 0;
    objectStorageBufferRequest.buffers = storageBufferInfos;
    objectStorageBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.descriptorSetName = kMainDescSet;
    lightUboRequest.bindingIndex = 1;
    lightUboRequest.buffers = lightUboInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = kMainDescSet;
    textureUpdateRequest.bindingIndex = 2;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest reflectionTextureUpdateRequest;
    reflectionTextureUpdateRequest.descriptorSetName = kMainDescSet;
    reflectionTextureUpdateRequest.bindingIndex = 3;
    reflectionTextureUpdateRequest.images = reflectionImageInfos;
    reflectionTextureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest cubemapUpdateRequest;
    cubemapUpdateRequest.descriptorSetName = kSkyboxDescSet;
    cubemapUpdateRequest.bindingIndex = 0;
    cubemapUpdateRequest.images = cubemapImageInfos;
    cubemapUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageBufferRequest, lightUboRequest},
        .imageWriteRequests = {textureUpdateRequest, reflectionTextureUpdateRequest, cubemapUpdateRequest}};

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

    reflectionRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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

    if (!reflectionRenderPass_) {
        throw std::runtime_error("Failed to create render pass (reflection)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MeshPushConstantsGpu);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {mvpPushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    pipelineLayoutSkybox_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kSkyboxDescSetLayout)}, {mvpPushConstant});

    if (!pipelineLayoutSkybox_) {
        throw std::runtime_error("Failed to create pipeline layout (for skybox)!");
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

    VkViewport viewportReflection{
        0, 0, static_cast<float>(CUBEMAP_RESOLUTION), static_cast<float>(CUBEMAP_RESOLUTION), 0.0f, 1.0f};
    VkRect2D scissorReflection{0, 0, CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION};

    reflectionScenePipeline_ =
            device_->CreateGraphicsPipeline(pipelineLayout_, reflectionRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kReflectionFragmentShaderKey)->GetHandle();
                });
                builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
                    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
                    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
                    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
                });
                builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                    viewportStateCreateInfo.viewportCount = 1;
                    viewportStateCreateInfo.pViewports = &viewportReflection;
                    viewportStateCreateInfo.scissorCount = 1;
                    viewportStateCreateInfo.pScissors = &scissorReflection;
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

    if (!reflectionScenePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for scene objects in reflection)!");
    }

    reflectionSkyboxPipeline_ =
            device_->CreateGraphicsPipeline(pipelineLayoutSkybox_, reflectionRenderPass_, [&](auto& builder) {
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
                    viewportStateCreateInfo.pViewports = &viewportReflection;
                    viewportStateCreateInfo.scissorCount = 1;
                    viewportStateCreateInfo.pScissors = &scissorReflection;
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

    if (!reflectionSkyboxPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for skybox in reflection)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    const auto& reflectionDepthImageView = resources_->GetImageView(kReflectionDepthImage, kReflectionDepthImageView);

    const std::vector<std::string> cubemapImageViewKeys = {
        kReflectionCubemapImageViewRight,  kReflectionCubemapImageViewLeft, kReflectionCubemapImageViewTop,
        kReflectionCubemapImageViewBottom, kReflectionCubemapImageViewBack, kReflectionCubemapImageViewFront};

    for (auto i = 0U; i < cubemapImageViewKeys.size(); ++i) {
        const auto& reflectionCubemapImageView =
                resources_->GetImageView(kReflectionCubemapImage, cubemapImageViewKeys[i]);

        cubemapReflectionFramebuffers_[i] = device_->CreateFramebuffer(
                reflectionRenderPass_, {reflectionCubemapImageView, reflectionDepthImageView},
                [&](auto& builder) { builder.SetDimensions(CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION); });

        if (!cubemapReflectionFramebuffers_[i]) {
            throw std::runtime_error("Failed to create cubemap reflection framebuffers!");
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

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    clearValues[1].depthStencil = {1.0f, 0};

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    const auto cubemapViewMatrices = cubemapCamera_->GetCubemapViewMatrices();
    auto cubemapProj = cubemapCamera_->GetProjectionMatrix();
    cubemapProj[1][1] *= -1; // Revert this projection flip for reflection render

    // RENDER PASS: Creating reflection cubemap image
    for (auto i = 0U; i < cubemapReflectionFramebuffers_.size(); ++i) {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = reflectionRenderPass_->GetHandle();
                    beginInfo.framebuffer = cubemapReflectionFramebuffers_[i]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(CUBEMAP_RESOLUTION, CUBEMAP_RESOLUTION);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        // Draw skybox
        {
            currentCmdBuffer->BindPipeline(reflectionSkyboxPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

            const auto& meshInfo = scene_->GetMesh(kSkyboxCubeObject);

            const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            const std::vector skyboxDescSets{resources_->GetDescriptorSet(kSkyboxDescSet)};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0,
                                                 skyboxDescSets);

            const auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                    cubemapViewMatrices[i], cubemapProj, glm::vec4(cubemapCamera_->GetPosition(), 1.0f));
            currentCmdBuffer->PushConstants(pipelineLayoutSkybox_,
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                            sizeof(meshPushConstants), &meshPushConstants);
            currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
        }

        // Draw reflected scene
        currentCmdBuffer->BindPipeline(reflectionScenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        for (const auto& [meshName, meshInfo]: scene_->GetAllMeshes()) {
            if (scene_->IsInGroup(meshName, kSkyboxObjectGroup) || meshName == kSphereObject) {
                continue;
            }

            const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            const std::vector cubeDescSets{resources_->GetDescriptorSet(kMainDescSet)};
            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, cubeDescSets);

            const auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                    cubemapViewMatrices[i], cubemapProj, glm::vec4(cubemapCamera_->GetPosition(), 1.0f));
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            0, sizeof(meshPushConstants), &meshPushConstants);
            currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
        }

        currentCmdBuffer->EndRenderPass();
    }

    // RENDER PASS: Drawing scene and reflection image
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
    // Draw skybox
    {
        currentCmdBuffer->BindPipeline(skyboxPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        const auto& meshInfo = scene_->GetMesh(kSkyboxCubeObject);

        const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

        const std::vector skyboxDescSets{resources_->GetDescriptorSet(kSkyboxDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0, skyboxDescSets);

        const auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                camera_->GetViewMatrix(), camera_->GetProjectionMatrix(), glm::vec4(camera_->GetPosition(), 1.0f));
        currentCmdBuffer->PushConstants(pipelineLayoutSkybox_,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(meshPushConstants), &meshPushConstants);
        currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
    }

    // Draw only scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    for (const auto& [meshName, meshInfo]: scene_->GetAllMeshes()) {
        if (scene_->IsInGroup(meshName, kSkyboxObjectGroup)) {
            continue;
        }

        const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

        const std::vector cubeDescSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, cubeDescSets);

        auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                camera_->GetViewMatrix(), camera_->GetProjectionMatrix(), glm::vec4(camera_->GetPosition(), 1.0f));
        currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(meshPushConstants), &meshPushConstants);
        currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    // Move the cubes around the reflective sphere
    const auto time = static_cast<float>(GetCurrentTime());

    for (int i = 0; i < MOVING_OBJECT_COUNT; ++i) {
        constexpr float speed = 1.0f;
        constexpr float radius = 4.0f;
        constexpr glm::vec3 center = kReflectiveObjectPosition;
        constexpr float angleStep = glm::two_pi<float>() / static_cast<float>(MOVING_OBJECT_COUNT);

        const float angle = time * speed + angleStep * static_cast<float>(i);

        glm::vec3 pos;
        pos.x = center.x + radius * std::cos(angle);
        pos.y = center.y;
        pos.z = center.z + radius * std::sin(angle);

        scene_->MoveObject(kCubeObject + std::to_string(i), pos);
    }

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

} // namespace examples::real_time_lighting::environment_mapping::dynamic_cubemap_reflections
