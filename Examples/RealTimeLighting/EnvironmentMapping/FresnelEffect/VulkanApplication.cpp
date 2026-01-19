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
#include "VulkanShaderModule.h"

namespace examples::real_time_lighting::environment_mapping::fresnel_effect
{
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
        CreateDefaultFramebuffers(resources_->GetImageView(GetParamStr(AppConstants::DepthImage),
                                                           GetParamStr(AppConstants::DepthImageView)));
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

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{GetParamStr(AppConstants::LightUniformBuffer), sizeof(LightUbo),
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.shaders = {.basePath = SHADERS_DIR,
                                  .shaderType = SHADER_TYPE,
                                  .modules = {{.name = GetParamStr(AppConstants::MainVertexShaderKey),
                                               .fileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                                              {.name = GetParamStr(AppConstants::SceneObjectsFragmentShaderKey),
                                               .fileName = GetParamStr(AppConstants::SceneObjectsFragmentShaderFile)},
                                              {.name = GetParamStr(AppConstants::SkyboxVertexShaderKey),
                                               .fileName = GetParamStr(AppConstants::SkyboxVertexShaderFile)},
                                              {.name = GetParamStr(AppConstants::SkyboxFragmentShaderKey),
                                               .fileName = GetParamStr(AppConstants::SkyboxFragmentShaderFile)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = GetParamStr(AppConstants::DepthImage),
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = depthImageFormat_,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .views = {ImageViewCreateInfo{.viewName = GetParamStr(AppConstants::DepthImageView),
                                      .format = depthImageFormat_,
                                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1}}}}};

    resourceCreateInfo.samplers = {
        SamplerResourceCreateInfo{.name = GetParamStr(AppConstants::MainSampler),
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}},
        SamplerResourceCreateInfo{.name = GetParamStr(AppConstants::SkyboxSampler),
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
    scene_->AddPerspectiveCamera(GetParamStr(AppConstants::CameraObject), glm::vec3(0.0f, 0.0f, orbitDistance_),
                                 aspectRatio);
    camera_ = std::dynamic_pointer_cast<PerspectiveCamera>(scene_->GetActiveCamera());

    // Materials
    materialManager_->LoadTexture(GetParamStr(AppConstants::FloorTexture), GetParamStr(AppConstants::MainSampler),
                                  GetParamStr(AppConstants::FloorTexturePath));
    materialManager_->LoadTexture(GetParamStr(AppConstants::FloorNormalTexture), GetParamStr(AppConstants::MainSampler),
                                  GetParamStr(AppConstants::FloorNormalTexturePath), VK_FORMAT_R8G8B8A8_UNORM);
    materialManager_->LoadCubemapTexture(
            GetParamStr(AppConstants::CubemapTexture), GetParamStr(AppConstants::SkyboxSampler),
            GetParamStr(AppConstants::CubemapRightTexturePath), GetParamStr(AppConstants::CubemapLeftTexturePath),
            GetParamStr(AppConstants::CubemapTopTexturePath), GetParamStr(AppConstants::CubemapBottomTexturePath),
            GetParamStr(AppConstants::CubemapBackTexturePath), GetParamStr(AppConstants::CubemapFrontTexturePath));

    const auto defaultMatName = GetParamStr(AppConstants::DefaultMaterial);
    materialManager_->CreatePhongTexturedMaterial(defaultMatName)
            .SetAmbientStrength(GetParamFloat(AppSettings::AmbientStrength))
            .SetSpecularStrength(GetParamFloat(AppSettings::SpecularStrength))
            .SetShininess(GetParamFloat(AppSettings::Shininess))
            .SetDiffuseMap(GetParamStr(AppConstants::FloorTexture))
            .SetNormalMap(GetParamStr(AppConstants::FloorNormalTexture))
            .Build();

    // Add scene objects
    scene_->AddPlane(GetParamStr(AppConstants::PlaneObject), glm::vec3{0.0f, -1.0f, 0.0f});
    scene_->SetMaterial(GetParamStr(AppConstants::PlaneObject), defaultMatName);
    scene_->ScaleObject(GetParamStr(AppConstants::PlaneObject), glm::vec3{2.0f});

    // Add skybox cube
    scene_->AddCube(GetParamStr(AppConstants::SkyboxCubeObject));
    scene_->AddToGroup(GetParamStr(AppConstants::SkyboxObjectGroup), {GetParamStr(AppConstants::SkyboxCubeObject)});
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = materialManager_->GetTextureCount();
    const auto cubemapCount = materialManager_->GetCubemapTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2 + combinedImageSamplerCount + 2 * cubemapCount,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + 2 * cubemapCount}},
        .layouts = {{.name = GetParamStr(AppConstants::MainDescSetLayout),
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = GetParamStr(AppConstants::SkyboxDescSetLayout),
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = GetParamStr(AppConstants::MainDescSet),
                            .layoutName = GetParamStr(AppConstants::MainDescSetLayout)},
                           {.name = GetParamStr(AppConstants::SkyboxDescSet),
                            .layoutName = GetParamStr(AppConstants::SkyboxDescSetLayout)}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageBufferInfos;
    storageBufferInfos.emplace_back(scene_->GetStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(GetParamStr(AppConstants::LightUniformBuffer))->GetHandle(), 0,
                               VK_WHOLE_SIZE);

    auto descriptorImageInfos = materialManager_->GetDescriptorImageInfos();

    auto cubemapImageInfos = materialManager_->GetCubemapDescriptorImageInfo(GetParamStr(AppConstants::CubemapTexture));

    BufferWriteRequest objectStorageBufferRequest;
    objectStorageBufferRequest.descriptorSetName = GetParamStr(AppConstants::MainDescSet);
    objectStorageBufferRequest.bindingIndex = 0;
    objectStorageBufferRequest.buffers = storageBufferInfos;
    objectStorageBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.descriptorSetName = GetParamStr(AppConstants::MainDescSet);
    lightUboRequest.bindingIndex = 1;
    lightUboRequest.buffers = lightUboInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = GetParamStr(AppConstants::MainDescSet);
    textureUpdateRequest.bindingIndex = 2;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest cubemapReflectionUpdateRequest;
    cubemapReflectionUpdateRequest.descriptorSetName = GetParamStr(AppConstants::MainDescSet);
    cubemapReflectionUpdateRequest.bindingIndex = 3;
    cubemapReflectionUpdateRequest.images = cubemapImageInfos;
    cubemapReflectionUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest cubemapSkyboxUpdateRequest;
    cubemapSkyboxUpdateRequest.descriptorSetName = GetParamStr(AppConstants::SkyboxDescSet);
    cubemapSkyboxUpdateRequest.bindingIndex = 0;
    cubemapSkyboxUpdateRequest.images = cubemapImageInfos;
    cubemapSkyboxUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageBufferRequest, lightUboRequest},
        .imageWriteRequests = {textureUpdateRequest, cubemapReflectionUpdateRequest, cubemapSkyboxUpdateRequest}};

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
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MeshPushConstantsGpu);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))}, {mvpPushConstant});

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

    scenePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneObjectsFragmentShaderKey))->GetHandle();
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

    pipelineLayoutSkybox_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(GetParamStr(AppConstants::SkyboxDescSetLayout))}, {mvpPushConstant});

    if (!pipelineLayoutSkybox_) {
        throw std::runtime_error("Failed to create pipeline layout (for skybox)!");
    }

    skyboxPipeline_ = device_->CreateGraphicsPipeline(pipelineLayoutSkybox_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SkyboxVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SkyboxFragmentShaderKey))->GetHandle();
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

    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    // Draw skybox
    {
        currentCmdBuffer->BindPipeline(skyboxPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        const auto& meshInfo = scene_->GetMesh(GetParamStr(AppConstants::SkyboxCubeObject));

        const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

        const std::vector skyboxDescSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::SkyboxDescSet))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutSkybox_, 0, skyboxDescSets);

        const auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                scene_->GetViewMatrix(), scene_->GetProjectionMatrix(), glm::vec4(camera_->GetPosition(), 1.0f));
        currentCmdBuffer->PushConstants(pipelineLayoutSkybox_,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(meshPushConstants), &meshPushConstants);
        currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
    }

    // Draw only scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    for (const auto& [meshName, meshInfo]: scene_->GetAllMeshes()) {
        if (scene_->IsInGroup(meshName, GetParamStr(AppConstants::SkyboxObjectGroup))) {
            continue;
        }

        const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

        const std::vector cubeDescSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::MainDescSet))};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, cubeDescSets);

        const auto meshPushConstants = meshInfo.GenerateMeshPushConstantsGpu(
                scene_->GetViewMatrix(), scene_->GetProjectionMatrix(), glm::vec4(camera_->GetPosition(), 1.0f));
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
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightDirection), 1.0f);
    lightUbo.lightColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightColor), 1.0f);
    resources_->SetBuffer(GetParamStr(AppConstants::LightUniformBuffer), &lightUbo, sizeof(lightUbo));
}
} // namespace examples::real_time_lighting::environment_mapping::fresnel_effect
