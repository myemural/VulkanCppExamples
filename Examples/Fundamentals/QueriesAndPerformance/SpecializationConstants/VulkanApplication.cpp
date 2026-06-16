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
#include "ScopedTimer.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "TimeUtils.h"
#include "VulkanHelpers.h"
#include "VulkanQueryPool.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::queries_and_performance::specialization_constants
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationQueriesAndPerformance(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
        camera_ = std::make_unique<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, 6.0f), aspectRatio);

        InitInputSystem();

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects();

        InitAssetManager();
        CreateResources();
        InitResources();

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
    PROFILE_FUNCTION_EVERY(1000);

    inFlightFences_[currentIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    CalculateAndSetMvp();
    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationQueriesAndPerformance::PreUpdate();

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

void VulkanApplication::PrepareBufferInfos()
{
    // Create primitive data
    primitivesData_[0] = {cubeVertices, cubeIndices, PER_OBJECT_COUNT};
    primitivesData_[1] = {sphereVertices, sphereIndices, PER_OBJECT_COUNT};
    primitivesData_[2] = {coneVertices, coneIndices, PER_OBJECT_COUNT};
    primitivesData_[3] = {cylinderVertices, cylinderIndices, PER_OBJECT_COUNT};

    // Calculate buffer allocation infos for vertices
    VkDeviceSize vOffset = 0;
    for (size_t i = 0; i < primitivesData_.size(); i++) {
        bufferAllocInfos_[i].vertexOffset = vOffset;
        bufferAllocInfos_[i].vertexSize = primitivesData_[i].vertices.size() * sizeof(VertexPos3Uv2);

        vOffset += bufferAllocInfos_[i].vertexSize;
    }

    // Store total size of vertices
    const VkDeviceSize totalVertexBufferSize = vOffset;

    // Calculate buffer allocation infos for indices
    VkDeviceSize iOffset = 0;
    for (size_t i = 0; i < primitivesData_.size(); i++) {
        bufferAllocInfos_[i].indexOffset = totalVertexBufferSize + iOffset;
        bufferAllocInfos_[i].indexSize = primitivesData_[i].indices.size() * sizeof(std::uint16_t);
        bufferAllocInfos_[i].indexCount = primitivesData_[i].indices.size();

        iOffset += bufferAllocInfos_[i].indexSize;
    }

    // Store total size of buffer
    totalBufferSize_ = totalVertexBufferSize + iOffset;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateResources()
{
    depthImageFormat_ = physicalDevice_->FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Pre-load textures
    const auto crateTextureAssetHandler = assetManager_->Load<TextureAsset>(kCrateTexturePath);
    crateTextureAsset_ = assetManager_->Get(crateTextureAssetHandler);

    ResourceDescriptor resourceCreateInfo;

    PrepareBufferInfos();

    // Find GPU uniform buffer offset alignment
    const auto props = physicalDevice_->GetProperties();
    const auto minUboAlignment = props.limits.minUniformBufferOffsetAlignment;
    uboAlignedSize_ = (sizeof(ObjectUbo) + minUboAlignment - 1) & ~(minUboAlignment - 1);

    // Calculate UBO size
    const std::uint32_t uboSize = uboAlignedSize_ * MAX_NUM_OBJECTS;

    resourceCreateInfo.buffers = {{kMainVertexIndexBuffer, totalBufferSize_,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kDynamicUniformBuffer, uboSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};

    // Fill descriptor set create infos
    resourceCreateInfo.descriptors = {
        .maxSets = 1,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kCubeDescSet, .layoutName = kMainDescSetLayout}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{
            .name = kCrateImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .dimensions = {crateTextureAsset_.width, crateTextureAsset_.height, 1},
            .views = {ImageViewCreateInfo{.viewName = kCrateImageView, .format = VK_FORMAT_R8G8B8A8_SRGB}}},
        ImageResourceCreateInfo{
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

void VulkanApplication::InitResources() const
{
    // Set vertex data
    for (auto i = 0U; i < primitivesData_.size(); ++i) {
        resources_->SetBuffer(kMainVertexIndexBuffer, primitivesData_[i].vertices.data(),
                              bufferAllocInfos_[i].vertexSize, bufferAllocInfos_[i].vertexOffset, false);
    }

    // Set index data
    for (auto i = 0U; i < primitivesData_.size(); ++i) {
        resources_->SetBuffer(kMainVertexIndexBuffer, primitivesData_[i].indices.data(), bufferAllocInfos_[i].indexSize,
                              bufferAllocInfos_[i].indexOffset, false);
    }

    resources_->SetImageFromTexture(cmdPool_, queue_, kCrateImage, crateTextureAsset_);

    UpdateDescriptorSets();
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
    pipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)});

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

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos3Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

    for (const auto& coloringMode: shaderColoringMode_) {
        uint32_t mode = coloringMode;

        VkSpecializationMapEntry entry{};
        entry.constantID = 0;
        entry.offset = 0;
        entry.size = sizeof(uint32_t);

        VkSpecializationInfo specInfo{};
        specInfo.mapEntryCount = 1;
        specInfo.pMapEntries = &entry;
        specInfo.dataSize = sizeof(uint32_t);
        specInfo.pData = &mode;

        const auto pipeline = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
            builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
            });
            builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
                shaderStageCreateInfo.pSpecializationInfo = &specInfo;
            });
            builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
                vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
                vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
                vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
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

        if (!pipeline) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        pipelines_.push_back(pipeline);
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorBufferInfo> uboBufferInfos;
    uboBufferInfos.emplace_back(resources_->GetBuffer(kDynamicUniformBuffer)->GetHandle(), 0, uboAlignedSize_);

    std::vector<VkDescriptorImageInfo> cubeImageSamplerInfos;
    cubeImageSamplerInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                       resources_->GetImageView(kCrateImage, kCrateImageView)->GetHandle(),
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    BufferWriteRequest objectUboRequest;
    objectUboRequest.descriptorSetName = kCubeDescSet;
    objectUboRequest.bindingIndex = 0;
    objectUboRequest.buffers = uboBufferInfos;
    objectUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    ImageWriteRequest samplerUpdateRequestCube;
    samplerUpdateRequestCube.descriptorSetName = kCubeDescSet;
    samplerUpdateRequestCube.bindingIndex = 1;
    samplerUpdateRequestCube.images = cubeImageSamplerInfos;
    samplerUpdateRequestCube.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {.bufferWriteRequests = {objectUboRequest},
                                                          .imageWriteRequests = {samplerUpdateRequestCube}};

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

    const std::vector cubeDescSets{resources_->GetDescriptorSet(kCubeDescSet)};
    unsigned int currentDrawCount = 0U;
    for (auto i = 0U; i < bufferAllocInfos_.size(); ++i) {
        for (auto j = 0U; j < primitivesData_[i].drawCount; ++j) {
            const auto currentMvpIndex = currentDrawCount + j;
            uint32_t dynamicOffset = currentMvpIndex * uboAlignedSize_;

            currentCmdBuffer->BindPipeline(pipelines_[currentMvpIndex % pipelines_.size()],
                                           VK_PIPELINE_BIND_POINT_GRAPHICS);
            const std::vector cubeVertexBuffers{resources_->GetBuffer(kMainVertexIndexBuffer)};
            currentCmdBuffer->BindVertexBuffers(cubeVertexBuffers, 0, 1, {bufferAllocInfos_[i].vertexOffset});
            currentCmdBuffer->BindIndexBuffer(resources_->GetBuffer(kMainVertexIndexBuffer),
                                              bufferAllocInfos_[i].indexOffset);

            currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, cubeDescSets,
                                                 {dynamicOffset});
            currentCmdBuffer->DrawIndexed(bufferAllocInfos_[i].indexCount, 1, 0, 0, 0);
        }

        currentDrawCount += primitivesData_[i].drawCount;
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    const auto currentTime = static_cast<float>(GetCurrentTime());
    for (size_t i = 0; i < MAX_NUM_OBJECTS; i++) {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, modelPositions[i]);
        const auto angle = glm::radians(std::fmod(static_cast<float>(i) * 45.0f + 10.0f, 180.0f));
        model = glm::rotate(model, currentTime * angle, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, currentTime * angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, currentTime * angle, glm::vec3(1.0f, 0.0f, 0.0f));

        const glm::mat4 view = camera_->GetViewMatrix();
        const glm::mat4 proj = camera_->GetProjectionMatrix();

        objectUbo_[i].mvpMatrix = proj * view * model;
    }

    resources_->SetBufferAlignedWithoutUnmap(kDynamicUniformBuffer, objectUbo_.data(), sizeof(ObjectUbo),
                                             MAX_NUM_OBJECTS, uboAlignedSize_);
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
} // namespace examples::fundamentals::queries_and_performance::specialization_constants
