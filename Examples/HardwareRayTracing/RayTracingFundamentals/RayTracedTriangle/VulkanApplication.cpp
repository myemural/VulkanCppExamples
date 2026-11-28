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
#include "PerspectiveCamera.h"
#include "ShaderLoader.h"
#include "VulkanAccelerationStructure.h"
#include "VulkanShaderModule.h"

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
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
    // Usage flags of the buffers which are read while building the acceleration structures
    constexpr VkBufferUsageFlags kAccelerationStructureInputUsage =
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
} // namespace

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationRayTracingFundamentals(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationRayTracingFundamentals::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        CreateAccelerationStructures();
        CreateAndUpdateDescriptorSets();
        UpdateSceneUniforms();

        CreateRenderPass();
        CreatePipelines();
        CreateShaderBindingTable();
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

    RecordCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos. Vertex, index and instance buffers are only read by the acceleration structure builds,
    // so they are accessed with their device addresses instead of being bound as vertex or index buffers.
    const auto vertexBufferSize = vertices.size() * sizeof(VertexPos3);
    const auto indexBufferSize = indices.size() * sizeof(std::uint32_t);

    resourceCreateInfo.buffers = {{kVertexBuffer, vertexBufferSize, kAccelerationStructureInputUsage,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kIndexBuffer, indexBufferSize, kAccelerationStructureInputUsage,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kInstanceBuffer, sizeof(VkAccelerationStructureInstanceKHR),
                                   kAccelerationStructureInputUsage,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kSceneUniformBuffer, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto rayGenShaderAsset = assetManager_->Load<ShaderAsset>(kRayGenShaderFile);
    const auto missShaderAsset = assetManager_->Load<ShaderAsset>(kMissShaderFile);
    const auto closestHitShaderAsset = assetManager_->Load<ShaderAsset>(kClosestHitShaderFile);
    const auto presentVertexShaderAsset = assetManager_->Load<ShaderAsset>(kPresentVertexShaderFile);
    const auto presentFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kPresentFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kRayGenShaderKey, .asset = assetManager_->Get(rayGenShaderAsset)},
                    {.name = kMissShaderKey, .asset = assetManager_->Get(missShaderAsset)},
                    {.name = kClosestHitShaderKey, .asset = assetManager_->Get(closestHitShaderAsset)},
                    {.name = kPresentVertexShaderKey, .asset = assetManager_->Get(presentVertexShaderAsset)},
                    {.name = kPresentFragmentShaderKey, .asset = assetManager_->Get(presentFragmentShaderAsset)}}};

    // Ray generation shader writes into this storage image, then the present pass samples it
    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = kRadianceImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = kRadianceImageFormat,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .views = {ImageViewCreateInfo{.viewName = kRadianceImageView, .format = kRadianceImageFormat}}}};

    resourceCreateInfo.samplers = {SamplerResourceCreateInfo{
        .name = kPresentSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);

    resources_->SetBuffer(kVertexBuffer, vertices.data(), vertexBufferSize);
    resources_->SetBuffer(kIndexBuffer, indices.data(), indexBufferSize);
}

void VulkanApplication::CreateAccelerationStructures() const
{
    // Bottom-level acceleration structure (BLAS) keeps the triangle geometry in its own (object) space
    AccelerationStructureTriangleGeometry triangleGeometry;
    triangleGeometry.vertexBufferAddress = resources_->GetBufferDeviceAddress(kVertexBuffer);
    triangleGeometry.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangleGeometry.vertexStride = sizeof(VertexPos3);
    triangleGeometry.maxVertex = static_cast<std::uint32_t>(vertices.size() - 1);
    triangleGeometry.indexBufferAddress = resources_->GetBufferDeviceAddress(kIndexBuffer);
    triangleGeometry.indexType = VK_INDEX_TYPE_UINT32;
    triangleGeometry.primitiveCount = static_cast<std::uint32_t>(indices.size() / 3);
    triangleGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // Any-hit shaders will not be invoked

    resources_->CreateAccelerationStructures({AccelerationStructureResourceCreateInfo{
        .name = kTriangleBlas,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .triangleGeometries = {triangleGeometry}}});

    // Top-level acceleration structure (TLAS) places the BLAS into the world with an instance. The instance references
    // the BLAS with its device address, so it can be written as soon as the BLAS object is created.
    VkAccelerationStructureInstanceKHR instance{};
    // Identity transform (row-major 3x4 matrix), the triangle is already in world space
    instance.transform = {{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}}};
    instance.instanceCustomIndex = 0;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0; // Uses the first hit group record
    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instance.accelerationStructureReference = resources_->GetAccelerationStructure(kTriangleBlas)->GetDeviceAddress();

    resources_->SetBuffer(kInstanceBuffer, &instance, sizeof(instance));

    AccelerationStructureInstanceGeometry instanceGeometry;
    instanceGeometry.instanceBufferAddress = resources_->GetBufferDeviceAddress(kInstanceBuffer);
    instanceGeometry.instanceCount = 1;

    resources_->CreateAccelerationStructures({AccelerationStructureResourceCreateInfo{
        .name = kSceneTlas,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .buildFlags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .instanceGeometry = instanceGeometry}});

    // Build both acceleration structures once, the scene is static
    ExecuteOneTimeCommands([&](const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) {
        VkMemoryBarrier asBuildBarrier{};
        asBuildBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        asBuildBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        asBuildBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

        resources_->BuildAccelerationStructure(cmdBuffer, kTriangleBlas);

        // TLAS build reads the BLAS, so the BLAS build must be finished before
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                   VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, {}, {}, {asBuildBarrier});

        resources_->BuildAccelerationStructure(cmdBuffer, kSceneTlas);

        // Make the TLAS visible to the ray tracing shaders of the following submissions
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                                   VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, {}, {}, {asBuildBarrier});
    });
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
        .layouts = {{.name = kRayTraceDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                                   VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, nullptr}}},
                    {.name = kPresentDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kRayTraceDescSet, .layoutName = kRayTraceDescSetLayout},
                           {.name = kPresentDescSet, .layoutName = kPresentDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorImageInfo> radianceImageStorageImageInfos;
    radianceImageStorageImageInfos.emplace_back(
            VK_NULL_HANDLE, resources_->GetImageView(kRadianceImage, kRadianceImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorBufferInfo> sceneUboInfos;
    sceneUboInfos.emplace_back(resources_->GetBuffer(kSceneUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorImageInfo> radianceImageSamplerInfos;
    radianceImageSamplerInfos.emplace_back(resources_->GetSampler(kPresentSampler)->GetHandle(),
                                           resources_->GetImageView(kRadianceImage, kRadianceImageView)->GetHandle(),
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    AccelerationStructureWriteRequest tlasRequest;
    tlasRequest.descriptorSetName = kRayTraceDescSet;
    tlasRequest.bindingIndex = 0;
    tlasRequest.accelerationStructures = {resources_->GetAccelerationStructure(kSceneTlas)->GetHandle()};

    ImageWriteRequest radianceImageRayTraceRequest;
    radianceImageRayTraceRequest.descriptorSetName = kRayTraceDescSet;
    radianceImageRayTraceRequest.bindingIndex = 1;
    radianceImageRayTraceRequest.images = radianceImageStorageImageInfos;
    radianceImageRayTraceRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    BufferWriteRequest sceneUboRequest;
    sceneUboRequest.descriptorSetName = kRayTraceDescSet;
    sceneUboRequest.bindingIndex = 2;
    sceneUboRequest.buffers = sceneUboInfos;
    sceneUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest radianceImagePresentRequest;
    radianceImagePresentRequest.descriptorSetName = kPresentDescSet;
    radianceImagePresentRequest.bindingIndex = 0;
    radianceImagePresentRequest.images = radianceImageSamplerInfos;
    radianceImagePresentRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {sceneUboRequest},
        .imageWriteRequests = {radianceImageRayTraceRequest, radianceImagePresentRequest},
        .accelerationStructureWriteRequests = {tlasRequest}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::UpdateSceneUniforms() const
{
    // Camera is static in this example, so the uniform buffer is written only once
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    const PerspectiveCamera camera{kCameraStartPosition, aspectRatio};

    const auto clearColor = params_.Get<VkClearColorValue>(AppSettings::ClearColor);

    SceneUbo sceneUbo{};
    sceneUbo.viewInverse = glm::inverse(camera.GetViewMatrix());
    sceneUbo.projInverse = glm::inverse(camera.GetProjectionMatrix());
    sceneUbo.missColor = glm::vec4(clearColor.float32[0], clearColor.float32[1], clearColor.float32[2], 1.0f);
    resources_->SetBuffer(kSceneUniformBuffer, &sceneUbo, sizeof(sceneUbo));
}

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

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
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                })
                // Wait for the swap chain image to be acquired before writing to it
                .AddDependency([](auto& dependency) {
                    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                    dependency.dstSubpass = 0;
                    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    dependency.srcAccessMask = 0;
                    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    dependency.dependencyFlags = 0;
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanApplication::CreatePipelines()
{
    rayTracingPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kRayTraceDescSetLayout)});

    if (!rayTracingPipelineLayout_) {
        throw std::runtime_error("Failed to create ray tracing pipeline layout!");
    }

    rayTracingPipeline_ = device_->CreateRayTracingPipeline(rayTracingPipelineLayout_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kRayGenShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMissShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClosestHitShaderKey)->GetHandle();
        });

        builder.AddGeneralShaderGroup(kRayGenShaderIndex);
        builder.AddGeneralShaderGroup(kMissShaderIndex);
        builder.AddTrianglesHitShaderGroup(kClosestHitShaderIndex);

        // Closest hit shader doesn't trace new rays, so only the rays of the ray generation shader exist
        builder.SetMaxRecursionDepth(1);
    });

    if (!rayTracingPipeline_) {
        throw std::runtime_error("Failed to create ray tracing pipeline!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    presentPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kPresentDescSetLayout)});

    if (!presentPipelineLayout_) {
        throw std::runtime_error("Failed to create present graphics pipeline layout!");
    }

    presentPipeline_ = device_->CreateGraphicsPipeline(presentPipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kPresentVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kPresentFragmentShaderKey)->GetHandle();
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
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
    });

    if (!presentPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for present)!");
    }
}

void VulkanApplication::CreateShaderBindingTable()
{
    // One record for each region, the instance uses SBT record offset 0, so its rays invoke the first hit group
    shaderBindingTable_ = std::make_unique<ShaderBindingTable>(physicalDevice_, device_);
    shaderBindingTable_->Create(rayTracingPipeline_, kShaderGroupCount,
                                ShaderBindingTableCreateInfo{.rayGenGroupIndex = kRayGenGroupIndex,
                                                             .missGroupIndices = {kMissGroupIndex},
                                                             .hitGroupIndices = {kHitGroupIndex}});
}

void VulkanApplication::CreateFramebuffers()
{
    // Present framebuffers
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage}, [&](auto& builder) {
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
    cmdBuffers_ = cmdPool_->CreateCommandBuffers(presentFramebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffers_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordCommandBuffers(const std::uint32_t currentImageIndex) const
{
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);

    const auto& currentCmdBuffer = cmdBuffers_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const auto radianceImage = resources_->GetImage(kRadianceImage);

    // Ray Tracing Phase: Trace one primary ray for each pixel
    {
        // Previous frame may still be sampling the image in its present pass, so wait for its fragment shader stage
        const auto barrierUndefinedToGeneral = radianceImage->CreateImageMemoryBarrier(
                0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, {barrierUndefinedToGeneral});

        currentCmdBuffer->BindPipeline(rayTracingPipeline_, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineLayout_, 0,
                                             {resources_->GetDescriptorSet(kRayTraceDescSet)});

        currentCmdBuffer->TraceRaysKHR(shaderBindingTable_->GetRayGenRegion(), shaderBindingTable_->GetMissRegion(),
                                       shaderBindingTable_->GetHitRegion(), shaderBindingTable_->GetCallableRegion(),
                                       currentWindowWidth_, currentWindowHeight_, 1);
    }

    // Present Phase: Present the result
    {
        const auto barrierGeneralToShaderRead = radianceImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {barrierGeneralToShaderRead});

        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = clearValues.size();
                    beginInfo.pClearValues = clearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kPresentDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, presentPipelineLayout_, 0, descSets);
        currentCmdBuffer->BindPipeline(presentPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        // Fullscreen triangle is generated in the vertex shader
        currentCmdBuffer->Draw(3, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}
} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
