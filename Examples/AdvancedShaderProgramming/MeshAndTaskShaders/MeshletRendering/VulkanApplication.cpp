/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>

#include <glm/ext/matrix_transform.hpp>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "MathUtils.h"
#include "ShaderLoader.h"
#include "VulkanHelpers.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationMeshAndTaskShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationMeshAndTaskShaders::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateResources();
        InitResources();
        InitTransformsAndCamera();

        InitInputSystem();

        CreateRenderPass();
        CreatePipeline();
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

    CalculateAndSetMvp();
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
    ApplicationMeshAndTaskShaders::PreUpdate();

    // Process continuous inputs
    ProcessInput();
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

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<GltfModelAsset>(std::make_unique<ModelLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateResources()
{
    // Load model
    const auto suzanneModelHandle = assetManager_->Load<GltfModelAsset>(kSuzanneDetailedModelPath);
    suzanneModelAsset_ = std::make_unique<GltfModelAsset>(assetManager_->Get(suzanneModelHandle));

    // Build the meshlet data for the mesh shader pipeline. This replaces the classic vertex/index buffer split with
    // a vertex pool plus three meshlet lookup buffers (vertex indices, triangle indices, meshlet descriptors) that the
    // mesh shader will pull from directly.
    suzanneMeshletData_ = suzanneModelAsset_->BuildMeshlets(0, 0);
    totalMeshletCount_ = static_cast<std::uint32_t>(suzanneMeshletData_.meshlets.size());

    // Mesh optimizer packs triangle-local indices as bytes (uint8_t) to save memory on the CPU side.
    // We expand them to uint32_t here so the mesh shader can read them straight out of a std430 storage buffer without
    // any bit-unpacking logic.
    meshletTrianglesExpanded_.assign(suzanneMeshletData_.meshletTriangles.begin(),
                                     suzanneMeshletData_.meshletTriangles.end());

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const auto vertexBufferSize =
            static_cast<std::uint32_t>(suzanneMeshletData_.vertices.size() * sizeof(VertexPos3Uv2));
    const auto meshletVertexIndicesSize =
            static_cast<std::uint32_t>(suzanneMeshletData_.meshletVertices.size() * sizeof(std::uint32_t));
    const auto meshletTriangleIndicesSize =
            static_cast<std::uint32_t>(meshletTrianglesExpanded_.size() * sizeof(std::uint32_t));
    const auto meshletDescriptorBufferSize =
            static_cast<std::uint32_t>(suzanneMeshletData_.meshlets.size() * sizeof(MeshletDescriptor));

    resourceCreateInfo.buffers = {
        {kMeshletVertexBuffer, vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletVertexIndicesBuffer, meshletVertexIndicesSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletTriangleIndicesBuffer, meshletTriangleIndicesSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletDescriptorBuffer, meshletDescriptorBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainMeshShaderAsset = assetManager_->Load<ShaderAsset>(kMainMeshShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainMeshShaderKey, .asset = assetManager_->Get(mainMeshShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.descriptors = {
        .maxSets = 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSetLayout, .layoutName = kMainDescSetLayout}}};

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

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitResources() const
{
    resources_->SetBuffer(kMeshletVertexBuffer, suzanneMeshletData_.vertices.data(),
                          static_cast<std::uint32_t>(suzanneMeshletData_.vertices.size() * sizeof(VertexPos3Uv2)));
    resources_->SetBuffer(
            kMeshletVertexIndicesBuffer, suzanneMeshletData_.meshletVertices.data(),
            static_cast<std::uint32_t>(suzanneMeshletData_.meshletVertices.size() * sizeof(std::uint32_t)));
    resources_->SetBuffer(kMeshletTriangleIndicesBuffer, meshletTrianglesExpanded_.data(),
                          static_cast<std::uint32_t>(meshletTrianglesExpanded_.size() * sizeof(std::uint32_t)));
    resources_->SetBuffer(kMeshletDescriptorBuffer, suzanneMeshletData_.meshlets.data(),
                          static_cast<std::uint32_t>(suzanneMeshletData_.meshlets.size() * sizeof(MeshletDescriptor)));

    UpdateDescriptorSets();
}

void VulkanApplication::InitTransformsAndCamera()
{
    const auto& randomObjectPositions =
            GenerateRandomPositions(kObjectCount, glm::vec3(-12.0f, -10.0f, -12.0f), glm::vec3(12.0f, 10.0f, 4.0f), 3.0f);

    for (auto i = 0U; i < kObjectCount; ++i) {
        objectPositions_[i] = randomObjectPositions[i];
        objectRotationsY_[i] = GenerateRandomValue(-90.0f, 90.0f);
    }

    float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 8.0f), aspectRatio);
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

void VulkanApplication::CreatePipeline()
{
    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MvpData);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT;

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

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainMeshShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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


    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorBufferInfo> storageMeshletVertexBufferInfos;
    storageMeshletVertexBufferInfos.emplace_back(resources_->GetBuffer(kMeshletVertexBuffer)->GetHandle(), 0,
                                                 VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMeshletVertexIndicesBufferInfos;
    storageMeshletVertexIndicesBufferInfos.emplace_back(resources_->GetBuffer(kMeshletVertexIndicesBuffer)->GetHandle(),
                                                        0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMeshletTriangleIndicesBufferInfos;
    storageMeshletTriangleIndicesBufferInfos.emplace_back(
            resources_->GetBuffer(kMeshletTriangleIndicesBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMeshletDescriptorBufferInfos;
    storageMeshletDescriptorBufferInfos.emplace_back(resources_->GetBuffer(kMeshletDescriptorBuffer)->GetHandle(), 0,
                                                     VK_WHOLE_SIZE);

    BufferWriteRequest bufferMeshletVertexRequest;
    bufferMeshletVertexRequest.descriptorSetName = kMainDescSetLayout;
    bufferMeshletVertexRequest.bindingIndex = 0;
    bufferMeshletVertexRequest.buffers = storageMeshletVertexBufferInfos;
    bufferMeshletVertexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletVertexIndicesRequest;
    bufferMeshletVertexIndicesRequest.descriptorSetName = kMainDescSetLayout;
    bufferMeshletVertexIndicesRequest.bindingIndex = 1;
    bufferMeshletVertexIndicesRequest.buffers = storageMeshletVertexIndicesBufferInfos;
    bufferMeshletVertexIndicesRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletTriangleIndicesRequest;
    bufferMeshletTriangleIndicesRequest.descriptorSetName = kMainDescSetLayout;
    bufferMeshletTriangleIndicesRequest.bindingIndex = 2;
    bufferMeshletTriangleIndicesRequest.buffers = storageMeshletTriangleIndicesBufferInfos;
    bufferMeshletTriangleIndicesRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletDescriptorRequest;
    bufferMeshletDescriptorRequest.descriptorSetName = kMainDescSetLayout;
    bufferMeshletDescriptorRequest.bindingIndex = 3;
    bufferMeshletDescriptorRequest.buffers = storageMeshletDescriptorBufferInfos;
    bufferMeshletDescriptorRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {bufferMeshletVertexRequest, bufferMeshletVertexIndicesRequest,
                                bufferMeshletTriangleIndicesRequest, bufferMeshletDescriptorRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
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
    currentCmdBuffer->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    const std::vector descSets{resources_->GetDescriptorSet(kMainDescSetLayout)};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);

    for (auto& mvp: mvpData_) {
        currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(MvpData), &mvp);
        currentCmdBuffer->DrawMeshTasksEXT(totalMeshletCount_, 1, 1);
    }


    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    for (size_t i = 0; i < kObjectCount; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, objectPositions_[i]);
        model = glm::rotate(model, glm::radians(objectRotationsY_[i]), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));

        const glm::mat4 view = camera_->GetViewMatrix();
        glm::mat4 proj = camera_->GetProjectionMatrix();

        mvpData_[i].mvpMatrix = proj * view * model;
    }
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
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::meshlet_rendering
