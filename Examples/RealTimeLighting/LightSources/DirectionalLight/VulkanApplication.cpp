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
#include "MathUtils.h"
#include "VulkanShaderModule.h"

namespace examples::real_time_lighting::light_sources::directional_light
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationLightSoruces(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationLightSoruces::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        CreateInitialResources();
        BuildScene();
        UpdateDescriptorSets();

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
    ApplicationLightSoruces::PreUpdate();

    // Process continuous inputs
    ProcessInput();
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
                                               .fileName = GetParamStr(AppConstants::SceneObjectsFragmentShaderFile)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.descriptors = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}},
        .layouts = {{.name = GetParamStr(AppConstants::MainDescSetLayout),
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}},
        .descriptorSets = {{.name = GetParamStr(AppConstants::MainDescSet),
                            .layoutName = GetParamStr(AppConstants::MainDescSetLayout)}}};

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

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);

    materialManager_ = std::make_unique<MaterialManager>(*resources_, cmdPool_, queue_, ASSETS_DIR);
    scene_ = std::make_unique<SceneManager>(*resources_, *materialManager_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    scene_->AddPerspectiveCamera(GetParamStr(AppConstants::CameraObject), glm::vec3(0.0f, 0.0f, 7.0f), aspectRatio);
    camera_ = std::dynamic_pointer_cast<PerspectiveCamera>(scene_->GetActiveCamera());

    // Materials
    const auto defaultMatName = GetParamStr(AppConstants::DefaultMaterial);
    materialManager_->CreatePhongMaterial(defaultMatName)
            .SetAmbientStrength(GetParamFloat(AppSettings::AmbientStrength))
            .SetSpecularStrength(GetParamFloat(AppSettings::SpecularStrength))
            .SetShininess(GetParamFloat(AppSettings::Shininess))
            .Build();

    auto& defaultMaterial = materialManager_->GetPhongMaterial(defaultMatName);

    // Add scene objects
    for (auto i = 0; i < 3; ++i) {
        const std::string rowStr = std::to_string(i);
        const auto zShift = static_cast<float>(i * 2 - 1);
        scene_->AddCube(GetParamStr(AppConstants::CubeObject) + rowStr, glm::vec3{-2.0f, -1.0f, zShift});
        defaultMaterial.diffuseColor = GenerateRandomColor(0.2f);
        scene_->SetMaterial(GetParamStr(AppConstants::CubeObject) + rowStr, defaultMatName);
        scene_->AddSphere(GetParamStr(AppConstants::SphereObject) + rowStr, glm::vec3{-0.5f, -1.0f, zShift});
        defaultMaterial.diffuseColor = GenerateRandomColor(0.2f);
        scene_->SetMaterial(GetParamStr(AppConstants::SphereObject) + rowStr, defaultMatName);
        scene_->AddCone(GetParamStr(AppConstants::ConeObject) + rowStr, glm::vec3{1.0f, -1.0f, zShift});
        defaultMaterial.diffuseColor = GenerateRandomColor(0.2f);
        scene_->SetMaterial(GetParamStr(AppConstants::ConeObject) + rowStr, defaultMatName);
        scene_->AddCylinder(GetParamStr(AppConstants::CylinderObject) + rowStr, glm::vec3{2.5f, -1.0f, zShift});
        defaultMaterial.diffuseColor = GenerateRandomColor(0.2f);
        scene_->SetMaterial(GetParamStr(AppConstants::CylinderObject) + rowStr, defaultMatName);
    }

    scene_->AddPlane(GetParamStr(AppConstants::PlaneObject), glm::vec3{0.0f, -2.0f, 0.0f}, glm::vec3(0.0f),
                     glm::vec3{4.0f});
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorBufferInfo> storageBufferInfos;
    storageBufferInfos.emplace_back(scene_->GetStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(GetParamStr(AppConstants::LightUniformBuffer))->GetHandle(), 0,
                               VK_WHOLE_SIZE);

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

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageBufferRequest, lightUboRequest}};

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
} // namespace examples::real_time_lighting::light_sources::directional_light
