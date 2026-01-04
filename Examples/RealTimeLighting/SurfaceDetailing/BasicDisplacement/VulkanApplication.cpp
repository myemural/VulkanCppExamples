/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>

#include <glm/ext/matrix_clip_space.hpp>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_lighting::surface_detailing::basic_displacement
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationSurfaceDetailing(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationSurfaceDetailing::Init()) {
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

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationSurfaceDetailing::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::CreateInitialResources() const
{
    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    const auto metalPatternTextureHandler = textureLoader.Load(GetParamStr(AppConstants::MetalPatternTexturePath));
    const auto metalPatternNormalTextureHandler =
            textureLoader.Load(GetParamStr(AppConstants::MetalPatternNormalTexturePath));
    const auto metalPatternHeightTextureHandler =
            textureLoader.Load(GetParamStr(AppConstants::MetalPatternHeightTexturePath));

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.Buffers = {{GetParamStr(AppConstants::LightUniformBuffer), sizeof(LightUbo),
                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    resourceCreateInfo.Shaders = {.BasePath = SHADERS_DIR,
                                  .ShaderType = SHADER_TYPE,
                                  .Modules = {{.Name = GetParamStr(AppConstants::MainVertexShaderKey),
                                               .FileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                                              {.Name = GetParamStr(AppConstants::SceneObjectsFragmentShaderKey),
                                               .FileName = GetParamStr(AppConstants::SceneObjectsFragmentShaderFile)}}};

    resourceCreateInfo.Images = {
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::MetalPatternImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = VK_FORMAT_R8G8B8A8_SRGB,
            .Dimensions = {metalPatternTextureHandler.Width, metalPatternTextureHandler.Height, 1},
            .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::MetalPatternImageView),
                                          .Format = VK_FORMAT_R8G8B8A8_SRGB}}},
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::MetalPatternNormalImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = VK_FORMAT_R8G8B8A8_UNORM,
            .Dimensions = {metalPatternNormalTextureHandler.Width, metalPatternNormalTextureHandler.Height, 1},
            .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::MetalPatternNormalImageView),
                                          .Format = VK_FORMAT_R8G8B8A8_UNORM}}},
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::MetalPatternHeightImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = VK_FORMAT_R8G8B8A8_UNORM,
            .Dimensions = {metalPatternHeightTextureHandler.Width, metalPatternHeightTextureHandler.Height, 1},
            .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::MetalPatternHeightImageView),
                                          .Format = VK_FORMAT_R8G8B8A8_UNORM}}},
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::DepthImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = depthImageFormat_,
            .Dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .Views = {ImageViewCreateInfo{.ViewName = GetParamStr(AppConstants::DepthImageView),
                                          .Format = depthImageFormat_,
                                          .SubresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                               .baseMipLevel = 0,
                                                               .levelCount = 1,
                                                               .baseArrayLayer = 0,
                                                               .layerCount = 1}}}}};

    resourceCreateInfo.Samplers = {
        {.Name = GetParamStr(AppConstants::MainSampler),
         .FilteringBehavior = {.MagFilter = VK_FILTER_LINEAR, .MinFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);

    // Set image resources
    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::MetalPatternImage),
                                    metalPatternTextureHandler);
    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::MetalPatternNormalImage),
                                    metalPatternNormalTextureHandler);
    resources_->SetImageFromTexture(cmdPool_, queue_, GetParamStr(AppConstants::MetalPatternHeightImage),
                                    metalPatternHeightTextureHandler);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.AttributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.AttributeLayout.emplace_back(AttributeType::TEXCOORD, AccessorType::VEC2);
    sceneConfig.AttributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);
    sceneConfig.AttributeLayout.emplace_back(AttributeType::TANGENT, AccessorType::VEC3);
    sceneConfig.CurrentMaterialSystem = MaterialSystem::PHONG_TEXTURED;
    sceneConfig.PrimitiveStackCount = 128U;
    sceneConfig.PrimitiveSectorCount = 128U;

    scene_ = std::make_unique<SceneManager>(*resources_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    scene_->AddPerspectiveCamera(GetParamStr(AppConstants::CameraObject), glm::vec3(0.0f, 1.0f, 7.0f), aspectRatio);
    camera_ = std::dynamic_pointer_cast<PerspectiveCamera>(scene_->GetActiveCamera());

    // Add texture registries
    scene_->RegisterTexture(GetParamStr(AppConstants::MetalPatternTexture),
                            resources_->GetSampler(GetParamStr(AppConstants::MainSampler)),
                            resources_->GetImageView(GetParamStr(AppConstants::MetalPatternImage),
                                                     GetParamStr(AppConstants::MetalPatternImageView)));
    scene_->RegisterTexture(GetParamStr(AppConstants::MetalPatternNormalTexture),
                            resources_->GetSampler(GetParamStr(AppConstants::MainSampler)),
                            resources_->GetImageView(GetParamStr(AppConstants::MetalPatternNormalImage),
                                                     GetParamStr(AppConstants::MetalPatternNormalImageView)));
    scene_->RegisterTexture(GetParamStr(AppConstants::MetalPatternHeightTexture),
                            resources_->GetSampler(GetParamStr(AppConstants::MainSampler)),
                            resources_->GetImageView(GetParamStr(AppConstants::MetalPatternHeightImage),
                                                     GetParamStr(AppConstants::MetalPatternHeightImageView)));

    // Create materials
    PhongTexturedMaterial material;
    material.ambientStrength = GetParamFloat(AppSettings::AmbientStrength);
    material.specularStrength = GetParamFloat(AppSettings::SpecularStrength);
    material.shininess = GetParamFloat(AppSettings::Shininess);
    material.diffuseMap = scene_->GetTextureId(GetParamStr(AppConstants::MetalPatternTexture));
    material.normalMap = scene_->GetTextureId(GetParamStr(AppConstants::MetalPatternNormalTexture));
    material.heightMap = scene_->GetTextureId(GetParamStr(AppConstants::MetalPatternHeightTexture));

    // Add scene objects
    scene_->AddSphere(GetParamStr(AppConstants::CubeObject), glm::vec3{-2.0f, -0.5f, 0.0f});
    scene_->SetObjectMaterial(GetParamStr(AppConstants::CubeObject), material);
    scene_->ScaleObject(GetParamStr(AppConstants::CubeObject), glm::vec3{2.0f});
    scene_->AddSphere(GetParamStr(AppConstants::SphereObject), glm::vec3{2.0f, -0.5f, 0.0f});
    scene_->SetObjectMaterial(GetParamStr(AppConstants::SphereObject), material);
    scene_->ScaleObject(GetParamStr(AppConstants::SphereObject), glm::vec3{2.0f});
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetRegisteredTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .MaxSets = 2 + combinedImageSamplerCount,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount}},
        .Layouts = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}},
        .DescriptorSets = {{.Name = GetParamStr(AppConstants::MainDescSet),
                            .LayoutName = GetParamStr(AppConstants::MainDescSetLayout)}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageBufferInfos;
    storageBufferInfos.emplace_back(scene_->GetStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(GetParamStr(AppConstants::LightUniformBuffer))->GetHandle(), 0,
                               VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetDescriptorImageInfos();

    BufferWriteRequest objectStorageBufferRequest;
    objectStorageBufferRequest.DescriptorSetName = GetParamStr(AppConstants::MainDescSet);
    objectStorageBufferRequest.BindingIndex = 0;
    objectStorageBufferRequest.Buffers = storageBufferInfos;
    objectStorageBufferRequest.Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.DescriptorSetName = GetParamStr(AppConstants::MainDescSet);
    lightUboRequest.BindingIndex = 1;
    lightUboRequest.Buffers = lightUboInfos;
    lightUboRequest.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.DescriptorSetName = GetParamStr(AppConstants::MainDescSet);
    textureUpdateRequest.BindingIndex = 2;
    textureUpdateRequest.Images = descriptorImageInfos;
    textureUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .BufferWriteRequests = {objectStorageBufferRequest, lightUboRequest},
        .ImageWriteRequests = {textureUpdateRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = static_cast<float>(currentWindowWidth_) / 2.0f;
    lastY_ = static_cast<float>(currentWindowHeight_) / 2.0f;

    window_->DisableCursor();

    window_->OnMouseMove([&](const MouseMoveEvent& event) {
        const auto xPos = static_cast<float>(event.X);
        const auto yPos = static_cast<float>(event.Y);

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

    VkSpecializationMapEntry entry{};
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    const std::uint32_t lightCount = scene_->GetRegisteredTextureCount();

    VkSpecializationInfo specInfo{};
    specInfo.mapEntryCount = 1;
    specInfo.pMapEntries = &entry;
    specInfo.dataSize = sizeof(uint32_t);
    specInfo.pData = &lightCount;

    scenePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))->GetHandle();
            shaderStageCreateInfo.pSpecializationInfo = &specInfo;
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    resources_->GetShaderModule(GetParamStr(AppConstants::SceneObjectsFragmentShaderKey))->GetHandle();
            shaderStageCreateInfo.pSpecializationInfo = &specInfo;
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

    const std::vector cubeDescSets{resources_->GetDescriptorSet(GetParamStr(AppConstants::MainDescSet))};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, cubeDescSets);
    const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

    // Draw only scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    for (const auto& [meshName, meshInfo]: scene_->GetAllMeshes()) {
        const auto [vertexOffsets, indexOffset, indexCount] = meshInfo.geometry;

        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

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
} // namespace examples::real_time_lighting::surface_detailing::basic_displacement
