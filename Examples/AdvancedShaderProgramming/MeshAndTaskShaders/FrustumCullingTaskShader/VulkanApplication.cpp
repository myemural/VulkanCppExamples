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

namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

namespace
{
    std::array<glm::vec4, 6> ExtractFrustumPlanes(const glm::mat4& m)
    {
        std::array<glm::vec4, 6> planes{};

        planes[0] = glm::vec4(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0]); // Left
        planes[1] = glm::vec4(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0], m[3][3] - m[3][0]); // Right
        planes[2] = glm::vec4(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1], m[3][3] + m[3][1]); // Bottom
        planes[3] = glm::vec4(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1], m[3][3] - m[3][1]); // Top
        planes[4] = glm::vec4(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2], m[3][3] + m[3][2]); // Near
        planes[5] = glm::vec4(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2], m[3][3] - m[3][2]); // Far

        for (auto& plane: planes) {
            const float len = glm::length(glm::vec3(plane));
            plane /= len;
        }

        return planes;
    }

    std::array<glm::vec4, 8> ComputeFrustumCornersWorld(const glm::mat4& viewProj)
    {
        const glm::mat4 inv = glm::inverse(viewProj);
        constexpr float kNdcNearZ = 0.0f;
        constexpr float kNdcFarZ = 1.0f;

        constexpr std::array<glm::vec4, 8> ndcCorners = {
            glm::vec4(-1.0f, -1.0f, kNdcNearZ, 1.0f), glm::vec4(1.0f, -1.0f, kNdcNearZ, 1.0f),
            glm::vec4(-1.0f, 1.0f, kNdcNearZ, 1.0f),  glm::vec4(1.0f, 1.0f, kNdcNearZ, 1.0f),
            glm::vec4(-1.0f, -1.0f, kNdcFarZ, 1.0f),  glm::vec4(1.0f, -1.0f, kNdcFarZ, 1.0f),
            glm::vec4(-1.0f, 1.0f, kNdcFarZ, 1.0f),   glm::vec4(1.0f, 1.0f, kNdcFarZ, 1.0f)};

        std::array<glm::vec4, 8> worldCorners{};
        for (std::size_t i = 0; i < 8; ++i) {
            glm::vec4 worldPos = inv * ndcCorners[i];
            worldPos /= worldPos.w;
            worldCorners[i] = glm::vec4(glm::vec3(worldPos), 1.0f);
        }

        return worldCorners;
    }
} // namespace

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
        InitTransformsAndCameras();

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

    UpdateResources();
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

    // Build the meshlet data
    suzanneMeshletData_ = suzanneModelAsset_->BuildMeshlets(0, 0);
    totalMeshletCount_ = static_cast<std::uint32_t>(suzanneMeshletData_.meshlets.size());
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
    const auto meshletBoundsBufferSize =
            static_cast<std::uint32_t>(suzanneMeshletData_.bounds.size() * sizeof(MeshletBounds));
    constexpr auto frustumPlanesBufferSize = static_cast<std::uint32_t>(6 * sizeof(glm::vec4));
    constexpr auto frustumCornersBufferSize = static_cast<std::uint32_t>(8 * sizeof(glm::vec4));

    resourceCreateInfo.buffers = {
        {kMeshletVertexBuffer, vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletVertexIndicesBuffer, meshletVertexIndicesSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletTriangleIndicesBuffer, meshletTriangleIndicesSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletDescriptorBuffer, meshletDescriptorBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMeshletBoundsBuffer, meshletBoundsBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kFrustumPlanesBuffer, frustumPlanesBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kFrustumCornersBuffer, frustumCornersBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};


    // Fill shader module create infos
    const auto mainTaskShaderAsset = assetManager_->Load<ShaderAsset>(kMainTaskShaderFile);
    const auto mainMeshShaderAsset = assetManager_->Load<ShaderAsset>(kMainMeshShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);
    const auto lineVertShaderAsset = assetManager_->Load<ShaderAsset>(kLineVertShaderFile);
    const auto lineFragShaderAsset = assetManager_->Load<ShaderAsset>(kLineFragShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainTaskShaderKey, .asset = assetManager_->Get(mainTaskShaderAsset)},
                    {.name = kMainMeshShaderKey, .asset = assetManager_->Get(mainMeshShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)},
                    {.name = kLineVertShaderKey, .asset = assetManager_->Get(lineVertShaderAsset)},
                    {.name = kLineFragShaderKey, .asset = assetManager_->Get(lineFragShaderAsset)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.descriptors = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 7}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_TASK_BIT_EXT, nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_TASK_BIT_EXT, nullptr}}},
                    {.name = kLineDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kLineDescSet, .layoutName = kLineDescSetLayout}}};

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
    resources_->SetBuffer(kMeshletBoundsBuffer, suzanneMeshletData_.bounds.data(),
                          static_cast<std::uint32_t>(suzanneMeshletData_.bounds.size() * sizeof(MeshletBounds)));

    UpdateDescriptorSets();
}

void VulkanApplication::InitTransformsAndCameras()
{
    // Scene camera
    float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 8.0f), aspectRatio);

    // Debug camera
    debugCamera_ = std::make_unique<PerspectiveCamera>(glm::vec3(7.0f, 5.0f, 7.0f), aspectRatio);
    debugCamera_->SetRotation(-135.0f, -25.0f);
    debugCamera_->SetFarPlane(200.0f);

    // Initialize debug viewport values
    debugViewportWidth_ = currentWindowWidth_ / 3;
    debugViewportHeight_ = currentWindowHeight_ / 3;

    constexpr std::uint32_t kDebugViewportMargin = 16;
    debugViewportX_ = currentWindowWidth_ - debugViewportWidth_ - kDebugViewportMargin;
    debugViewportY_ = currentWindowHeight_ - debugViewportHeight_ - kDebugViewportMargin;

    // Initialize model matrices
    constexpr std::array modelPositions = {glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -5.0f},
                                           glm::vec3{0.0f, 0.0f, -10.0f}};

    for (const auto& modelPosition: modelPositions) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, modelPosition);
        model = glm::scale(model, glm::vec3(2.0f));
        modelMatrices_.emplace_back(model);
    }
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
    CreateCullPipeline();
    CreateLinePipeline();
}

void VulkanApplication::CreateCullPipeline()
{
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(CullingPushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {pushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainTaskShaderKey)->GetHandle();
        });
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
            viewportStateCreateInfo.pViewports = nullptr;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = nullptr;
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
        builder.SetDynamicState([&](auto& dynamicStateCreateInfo) {
            dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
            dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
        });
    });

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::CreateLinePipeline()
{
    VkPushConstantRange linePushConstant;
    linePushConstant.offset = 0;
    linePushConstant.size = sizeof(LinePushConstants);
    linePushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutLines_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kLineDescSetLayout)}, {linePushConstant});

    if (!pipelineLayoutLines_) {
        throw std::runtime_error("Failed to create line pipeline layout!");
    }

    VkViewport viewport{static_cast<float>(debugViewportX_),
                        static_cast<float>(debugViewportY_),
                        static_cast<float>(debugViewportWidth_),
                        static_cast<float>(debugViewportHeight_),
                        0.0f,
                        1.0f};
    VkRect2D scissor{{static_cast<std::int32_t>(debugViewportX_), static_cast<std::int32_t>(debugViewportY_)},
                     {debugViewportWidth_, debugViewportHeight_}};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    pipelineLines_ = device_->CreateGraphicsPipeline(pipelineLayoutLines_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kLineVertShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kLineFragShaderKey)->GetHandle();
        });
        builder.SetInputAssemblyState([](auto& inputAssemblyStateCreateInfo) {
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
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
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!pipelineLines_) {
        throw std::runtime_error("Failed to create line graphics pipeline!");
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

    std::vector<VkDescriptorBufferInfo> storageMeshletBoundsBufferInfos;
    storageMeshletBoundsBufferInfos.emplace_back(resources_->GetBuffer(kMeshletBoundsBuffer)->GetHandle(), 0,
                                                 VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageFrustumPlanesBufferInfos;
    storageFrustumPlanesBufferInfos.emplace_back(resources_->GetBuffer(kFrustumPlanesBuffer)->GetHandle(), 0,
                                                 VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageFrustumCornersBufferInfos;
    storageFrustumCornersBufferInfos.emplace_back(resources_->GetBuffer(kFrustumCornersBuffer)->GetHandle(), 0,
                                                  VK_WHOLE_SIZE);

    BufferWriteRequest bufferMeshletVertexRequest;
    bufferMeshletVertexRequest.descriptorSetName = kMainDescSet;
    bufferMeshletVertexRequest.bindingIndex = 0;
    bufferMeshletVertexRequest.buffers = storageMeshletVertexBufferInfos;
    bufferMeshletVertexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletVertexIndicesRequest;
    bufferMeshletVertexIndicesRequest.descriptorSetName = kMainDescSet;
    bufferMeshletVertexIndicesRequest.bindingIndex = 1;
    bufferMeshletVertexIndicesRequest.buffers = storageMeshletVertexIndicesBufferInfos;
    bufferMeshletVertexIndicesRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletTriangleIndicesRequest;
    bufferMeshletTriangleIndicesRequest.descriptorSetName = kMainDescSet;
    bufferMeshletTriangleIndicesRequest.bindingIndex = 2;
    bufferMeshletTriangleIndicesRequest.buffers = storageMeshletTriangleIndicesBufferInfos;
    bufferMeshletTriangleIndicesRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletDescriptorRequest;
    bufferMeshletDescriptorRequest.descriptorSetName = kMainDescSet;
    bufferMeshletDescriptorRequest.bindingIndex = 3;
    bufferMeshletDescriptorRequest.buffers = storageMeshletDescriptorBufferInfos;
    bufferMeshletDescriptorRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferMeshletBoundsRequest;
    bufferMeshletBoundsRequest.descriptorSetName = kMainDescSet;
    bufferMeshletBoundsRequest.bindingIndex = 4;
    bufferMeshletBoundsRequest.buffers = storageMeshletBoundsBufferInfos;
    bufferMeshletBoundsRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferFrustumPlanesRequest;
    bufferFrustumPlanesRequest.descriptorSetName = kMainDescSet;
    bufferFrustumPlanesRequest.bindingIndex = 5;
    bufferFrustumPlanesRequest.buffers = storageFrustumPlanesBufferInfos;
    bufferFrustumPlanesRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest bufferFrustumCornersRequest;
    bufferFrustumCornersRequest.descriptorSetName = kLineDescSet;
    bufferFrustumCornersRequest.bindingIndex = 0;
    bufferFrustumCornersRequest.buffers = storageFrustumCornersBufferInfos;
    bufferFrustumCornersRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {bufferMeshletVertexRequest, bufferMeshletVertexIndicesRequest,
                                bufferMeshletTriangleIndicesRequest, bufferMeshletDescriptorRequest,
                                bufferMeshletBoundsRequest, bufferFrustumPlanesRequest, bufferFrustumCornersRequest}};

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

    const std::uint32_t taskGroupCount = (totalMeshletCount_ + kTaskGroupSize - 1U) / kTaskGroupSize;
    const std::vector mainDescSets{resources_->GetDescriptorSet(kMainDescSet)};
    const std::vector lineDescSets{resources_->GetDescriptorSet(kLineDescSet)};

    const glm::mat4 mainView = camera_->GetViewMatrix();
    const glm::mat4 mainProj = camera_->GetProjectionMatrix();
    const glm::mat4 mainViewProj = mainProj * mainView;

    const glm::mat4 debugView = debugCamera_->GetViewMatrix();
    const glm::mat4 debugProj = debugCamera_->GetProjectionMatrix();
    const auto debugViewProj = debugProj * debugView;

    // Set viewport for the main scene (whole screen)
    {
        VkViewport viewport{
            0, 0, static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_), 0.0f, 1.0f};
        VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

        currentCmdBuffer->SetViewports(0, {viewport});
        currentCmdBuffer->SetScissors(0, {scissor});
    }

    // PASS 1: Render main scene with main camera to whole screen
    {
        currentCmdBuffer->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, mainDescSets);

        for (const auto& modelMatrix: modelMatrices_) {
            CullingPushConstants mainPushConstants{};
            mainPushConstants.modelMatrix = modelMatrix;
            mainPushConstants.projViewMatrix = mainViewProj;
            mainPushConstants.meshletCount = totalMeshletCount_;
            currentCmdBuffer->PushConstants(pipelineLayout_,
                                            VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0,
                                            sizeof(CullingPushConstants), &mainPushConstants);
            currentCmdBuffer->DrawMeshTasksEXT(taskGroupCount, 1, 1);
        }
    }

    // Clear the color and depth attachments for the debug area
    {
        std::vector<VkClearAttachment> clearAttachmentValues(2);
        clearAttachmentValues[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachmentValues[0].colorAttachment = 0;
        clearAttachmentValues[0].clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}}; // Debug panel background color
        clearAttachmentValues[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        clearAttachmentValues[1].clearValue.depthStencil = {1.0f, 0};

        const VkRect2D backgroundRect{
            {static_cast<std::int32_t>(debugViewportX_), static_cast<std::int32_t>(debugViewportY_)},
            {debugViewportWidth_, debugViewportHeight_}};

        VkClearRect backgroundClearRect{};
        backgroundClearRect.rect = backgroundRect;
        backgroundClearRect.baseArrayLayer = 0;
        backgroundClearRect.layerCount = 1;

        currentCmdBuffer->ClearAttachments(clearAttachmentValues, {backgroundClearRect});
    }

    // Set viewport for the debug view
    {
        VkViewport viewport{static_cast<float>(debugViewportX_),
                            static_cast<float>(debugViewportY_),
                            static_cast<float>(debugViewportWidth_),
                            static_cast<float>(debugViewportHeight_),
                            0.0f,
                            1.0f};
        VkRect2D scissor{{static_cast<std::int32_t>(debugViewportX_), static_cast<std::int32_t>(debugViewportY_)},
                         {debugViewportWidth_, debugViewportHeight_}};

        currentCmdBuffer->SetViewports(0, {viewport});
        currentCmdBuffer->SetScissors(0, {scissor});
    }

    // PASS 2: Render debug scene with debug camera to specified area
    {
        currentCmdBuffer->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, mainDescSets);

        for (const auto& modelMatrix: modelMatrices_) {
            CullingPushConstants debugPushConstants{};
            debugPushConstants.modelMatrix = modelMatrix;
            debugPushConstants.projViewMatrix = debugViewProj;
            debugPushConstants.meshletCount = totalMeshletCount_;
            currentCmdBuffer->PushConstants(pipelineLayout_,
                                            VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0,
                                            sizeof(CullingPushConstants), &debugPushConstants);
            currentCmdBuffer->DrawMeshTasksEXT(taskGroupCount, 1, 1);
        }
    }

    // PASS 3: Render main camera's frustum wireframe to the debug scene
    {
        currentCmdBuffer->BindPipeline(pipelineLines_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutLines_, 0, lineDescSets);

        LinePushConstants linePushConstants{};
        linePushConstants.viewProjMat = debugViewProj;
        currentCmdBuffer->PushConstants(pipelineLayoutLines_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(LinePushConstants),
                                        &linePushConstants);
        // 24 vertices = 12 line segments, no vertex or index buffer required
        currentCmdBuffer->Draw(24, 1, 0, 0);
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateResources() const
{
    const glm::mat4 mainView = camera_->GetViewMatrix();
    const glm::mat4 mainProj = camera_->GetProjectionMatrix();
    const glm::mat4 mainViewProj = mainProj * mainView;

    // Main camera's frustum planes
    const auto planes = ExtractFrustumPlanes(mainViewProj);
    resources_->SetBuffer(kFrustumPlanesBuffer, planes.data(), planes.size() * sizeof(glm::vec4));

    // Main camera's frustum corners, for the wireframe drawn in the debug scene
    const auto corners = ComputeFrustumCornersWorld(mainViewProj);
    resources_->SetBuffer(kFrustumCornersBuffer, corners.data(), corners.size() * sizeof(glm::vec4));
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
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
