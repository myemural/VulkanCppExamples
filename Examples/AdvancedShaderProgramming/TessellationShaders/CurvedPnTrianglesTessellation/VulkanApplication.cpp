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
#include "ModelAsset.h"
#include "ModelLoader.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::tessellation_shaders::curved_pn_triangles_tessellation
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationTessellationShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationTessellationShaders::Init()) {
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

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<GltfModelAsset>(std::make_unique<ModelLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainTessControlShaderAsset = assetManager_->Load<ShaderAsset>(kMainTessControlShaderFile);
    const auto mainTessEvaluationShaderAsset = assetManager_->Load<ShaderAsset>(kMainTessEvaluationShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainTessControlShaderKey, .asset = assetManager_->Get(mainTessControlShaderAsset)},
                    {.name = kMainTessEvaluationShaderKey, .asset = assetManager_->Get(mainTessEvaluationShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
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
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

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

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, orbitDistance_), aspectRatio);

    // Load and convert models
    const auto suzanneModelHandle = assetManager_->Load<GltfModelAsset>(kSuzanneModelPath);
    const auto suzanneModel = std::make_unique<GltfModelAsset>(assetManager_->Get(suzanneModelHandle));

    GltfToSceneObjectConverter converter{*scene_, kMainSampler};
    auto suzanneModelSceneObjectBuilderLow = converter.ConvertToBuilder(kLowDetailSuzanneModelName, *suzanneModel);
    auto suzanneModelSceneObjectBuilderMedium =
            converter.ConvertToBuilder(kMediumDetailSuzanneModelName, *suzanneModel);
    auto suzanneModelSceneObjectBuilderHigh = converter.ConvertToBuilder(kHighDetailSuzanneModelName, *suzanneModel);

    const auto rootObject = SceneObjectBuilder(*scene_, kRootObject)
                                    .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                    .AddChild(suzanneModelSceneObjectBuilderLow.WithTag(kLowDetail)
                                                      .WithPosition(glm::vec3{-2.2f, 0.0f, 0.0f})
                                                      .WithScale(glm::vec3(0.8f)))
                                    .AddChild(suzanneModelSceneObjectBuilderMedium.WithTag(kMediumDetail)
                                                      .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                                      .WithScale(glm::vec3(0.8f)))
                                    .AddChild(suzanneModelSceneObjectBuilderHigh.WithTag(kHighDetail)
                                                      .WithPosition(glm::vec3{2.2f, 0.0f, 0.0f})
                                                      .WithScale(glm::vec3(0.8f)))
                                    .Build();

    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

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

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {.bufferWriteRequests = {objectStorageTransformBufferRequest,
                                                                                  objectStorageMaterialBufferRequest,
                                                                                  lightUboRequest}};

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
    });

    // For zooming to target
    window_->OnMouseScroll([this](const MouseScrollEvent& event) {
        constexpr auto minZoom = 0.2f;
        constexpr auto maxZoom = 90.0f;
        orbitDistance_ = glm::clamp(orbitDistance_ - static_cast<float>(event.deltaY) *
                                                             GetParamFloat(AppSettings::CameraZoomSpeed),
                                    minZoom, maxZoom);
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
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(MeshPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                              VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {pushConstant});

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
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainTessControlShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainTessEvaluationShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
        });
        builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
            vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();
        });
        builder.SetInputAssemblyState([&](auto& inputAssemblyStateCreateInfo) {
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        });
        builder.SetTessellationState([&](auto& tessellationStateCreateInfo) {
            tessellationStateCreateInfo.patchControlPoints = kPatchCount;
        });
        builder.SetViewportState([&](auto& viewportStateCreateInfo) {
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = &viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = &scissor;
        });
        builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
            rasterizationStateCreateInfo.polygonMode = params_.Get<VkPolygonMode>(AppSettings::PolygonMode);
            rasterizationStateCreateInfo.lineWidth = 1.0f;
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

    const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);
    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable()) {
            const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            MeshPushConstants meshPushConstants{};
            meshPushConstants.objectId = sceneObject.GetObjectId();
            meshPushConstants.view = camera_->GetViewMatrix();
            meshPushConstants.projection = camera_->GetProjectionMatrix();
            meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
            meshPushConstants.tessAlpha = 1.0f;

            // Set the tessellation level according to the detail groups
            if (sceneObject.GetTag() == kLowDetail) {
                meshPushConstants.tessLevel = 1.0f;
            } else if (sceneObject.GetTag() == kMediumDetail) {
                meshPushConstants.tessLevel = 3.0f;
            } else if (sceneObject.GetTag() == kHighDetail) {
                meshPushConstants.tessLevel = 8.0f;
            }

            currentCmdBuffer->PushConstants(pipelineLayout_,
                                            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                                                    VK_SHADER_STAGE_FRAGMENT_BIT,
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
    // Update orientation of the orbit camera in every frame
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

    // Update light UBO in every frame
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
    lightUbo.lightColor = glm::vec4(kLightColor, 1.0f);
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));
}

} // namespace examples::advanced_shader_programming::tessellation_shaders::curved_pn_triangles_tessellation
