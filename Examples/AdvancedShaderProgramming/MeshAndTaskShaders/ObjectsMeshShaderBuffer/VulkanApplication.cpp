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
#include "BuiltinPrimitives.h"
#include "MathUtils.h"
#include "ShaderLoader.h"
#include "VulkanHelpers.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::utility;
using namespace common::scene;
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
        InitModelMatricesAndCamera();

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

    UpdateSceneUBO();
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

void VulkanApplication::CreateResources() const
{
    ResourceDescriptor resourceCreateInfo;
    resourceCreateInfo.buffers = {{kPositionBuffer, kDefaultBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kNormalBuffer, kDefaultBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kIndexBuffer, kDefaultBufferSizeInBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kSceneUniformBuffer, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                   VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout}}};

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

void VulkanApplication::InitResources()
{
    std::vector<glm::vec3> allPositions;
    std::vector<glm::vec3> allNormals;
    std::vector<std::uint32_t> allIndices;

    auto appendMesh = [&](const MeshPrimitive& mesh) -> GpuMesh {
        const auto& posAccessor = mesh.attributes.at(AttributeType::POSITION);
        const auto& normalAccessor = mesh.attributes.at(AttributeType::NORMAL);
        const auto& indexAccessor = mesh.indices;

        const auto* posData = reinterpret_cast<const glm::vec3*>(posAccessor.bufferView.data);
        const auto* normalData = reinterpret_cast<const glm::vec3*>(normalAccessor.bufferView.data);
        const auto* indexData16 = reinterpret_cast<const std::uint16_t*>(indexAccessor.bufferView.data);

        GpuMesh gpuMesh;
        gpuMesh.vertexOffset = static_cast<std::uint32_t>(allPositions.size());
        gpuMesh.vertexCount = static_cast<std::uint32_t>(posAccessor.count);
        gpuMesh.indexOffset = static_cast<std::uint32_t>(allIndices.size());
        gpuMesh.primitiveCount = static_cast<std::uint32_t>(indexAccessor.count / 3); // Triangle count

        allPositions.insert(allPositions.end(), posData, posData + posAccessor.count);
        allNormals.insert(allNormals.end(), normalData, normalData + normalAccessor.count);

        // Extend indices from std::uint16_t to std::uint32_t
        allIndices.reserve(allIndices.size() + indexAccessor.count);
        for (std::size_t i = 0; i < indexAccessor.count; ++i) {
            allIndices.push_back(static_cast<std::uint32_t>(indexData16[i]));
        }

        return gpuMesh;
    };

    // Generate only cube and sphere meshes
    gpuMeshes_[BuiltinMeshType::CUBE] = appendMesh(CubePrimitive(1.0f).GetMeshPrimitive());
    gpuMeshes_[BuiltinMeshType::SPHERE] = appendMesh(SpherePrimitive(1.0f, 32U, 32U).GetMeshPrimitive());

    resources_->SetBuffer(kPositionBuffer, allPositions.data(), allPositions.size() * sizeof(glm::vec3));
    resources_->SetBuffer(kNormalBuffer, allNormals.data(), allNormals.size() * sizeof(glm::vec3));
    resources_->SetBuffer(kIndexBuffer, allIndices.data(), allIndices.size() * sizeof(std::uint32_t));

    UpdateDescriptorSets();
}

void VulkanApplication::InitModelMatricesAndCamera()
{
    const auto& randomObjectPositions = GenerateRandomPositions(kObjectCount, glm::vec3(-12.0f, -10.0f, -12.0f),
                                                                glm::vec3(12.0f, 10.0f, 4.0f), 2.0f);

    // Create scene objects
    for (auto i = 0U; i < kObjectCount; ++i) {
        const auto& currentObjectPos = randomObjectPositions[i];
        const float currentObjectRotY = GenerateRandomValue(-90.0f, 90.0f);

        auto model = glm::mat4(1.0f);
        model = glm::translate(model, currentObjectPos);
        model = glm::rotate(model, glm::radians(currentObjectRotY), glm::vec3(0.0f, 1.0f, 0.0f));

        // Assign random mesh type to the current object
        const auto typeValue = GenerateRandomValue(0U, 1U);
        BuiltinMeshType meshType = typeValue == 0U ? BuiltinMeshType::CUBE : BuiltinMeshType::SPHERE;
        sceneObjects_.emplace_back(meshType, model);
    }

    // Create camera
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
    mvpPushConstant.size = sizeof(MeshPushConstants);
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
    std::vector<VkDescriptorBufferInfo> positionBufferInfos;
    positionBufferInfos.emplace_back(resources_->GetBuffer(kPositionBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> normalBufferInfos;
    normalBufferInfos.emplace_back(resources_->GetBuffer(kNormalBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> indexBufferInfos;
    indexBufferInfos.emplace_back(resources_->GetBuffer(kIndexBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> sceneUboInfos;
    sceneUboInfos.emplace_back(resources_->GetBuffer(kSceneUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    BufferWriteRequest positionRequest;
    positionRequest.descriptorSetName = kMainDescSet;
    positionRequest.bindingIndex = 0;
    positionRequest.buffers = positionBufferInfos;
    positionRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest normalRequest;
    normalRequest.descriptorSetName = kMainDescSet;
    normalRequest.bindingIndex = 1;
    normalRequest.buffers = normalBufferInfos;
    normalRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest indexRequest;
    indexRequest.descriptorSetName = kMainDescSet;
    indexRequest.bindingIndex = 2;
    indexRequest.buffers = indexBufferInfos;
    indexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest sceneUboRequest;
    sceneUboRequest.descriptorSetName = kMainDescSet;
    sceneUboRequest.bindingIndex = 3;
    sceneUboRequest.buffers = sceneUboInfos;
    sceneUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {positionRequest, normalRequest, indexRequest, sceneUboRequest}};

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
    const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
    currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, descSets);

    // Draw scene objects via mesh/task pipeline
    for (const auto& [meshType, modelMatrix]: sceneObjects_) {
        const GpuMesh& gpuMesh = gpuMeshes_.at(meshType);

        MeshPushConstants pc{};
        pc.model = modelMatrix;
        pc.invModel = glm::inverse(modelMatrix);
        pc.vertexOffset = gpuMesh.vertexOffset;
        pc.vertexCount = gpuMesh.vertexCount;
        pc.indexOffset = gpuMesh.indexOffset;
        pc.primitiveCount = gpuMesh.primitiveCount;
        currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(pc), &pc);

        const std::uint32_t workgroupCount = CeilDiv(gpuMesh.primitiveCount, kTrianglesPerWorkgroup);
        currentCmdBuffer->DrawMeshTasksEXT(workgroupCount, 1, 1);
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneUBO() const
{
    SceneUbo sceneUbo{};
    sceneUbo.view = camera_->GetViewMatrix();
    sceneUbo.proj = camera_->GetProjectionMatrix();
    sceneUbo.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
    sceneUbo.lightDirection = glm::vec4(kLightDirection, 0.0f);
    sceneUbo.lightColor = glm::vec4(kLightColor, 1.0f);
    sceneUbo.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sceneUbo.specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sceneUbo.ambientStrength = 0.05f;
    sceneUbo.shininess = 128.0f;
    sceneUbo.specularStrength = 0.5f;
    resources_->SetBuffer(kSceneUniformBuffer, &sceneUbo, sizeof(sceneUbo));
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
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
