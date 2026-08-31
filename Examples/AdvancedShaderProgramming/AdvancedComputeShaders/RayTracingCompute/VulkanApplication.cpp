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
#include "BvhBuilder.h"
#include "MathUtils.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

namespace
{
    constexpr auto kRadianceImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

    // Recreates the very same built-in primitive on the host, so its triangles can be fed into BVH
    std::unique_ptr<BuiltinPrimitive> CreateHostPrimitive(const BuiltinMeshType meshType)
    {
        switch (meshType) {
            case BuiltinMeshType::CUBE:
                return std::make_unique<CubePrimitive>(1.0f);
            case BuiltinMeshType::SPHERE:
                return std::make_unique<SpherePrimitive>(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            case BuiltinMeshType::CONE:
                return std::make_unique<ConePrimitive>(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            case BuiltinMeshType::CYLINDER:
                return std::make_unique<CylinderPrimitive>(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            case BuiltinMeshType::PLANE:
                return std::make_unique<PlanePrimitive>(1.0f, kPrimitiveStackCount, kPrimitiveSectorCount);
            default:
                throw std::invalid_argument("Unsupported built-in mesh type for BVH build!");
        }
    }
} // namespace

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationAdvancedComputeShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationAdvancedComputeShaders::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        BuildScene();
        BuildAccelerationStructure();
        CreateAndUpdateDescriptorSets();

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
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

    UpdateSceneUniforms();
    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
    ++frameCount_;
}

void VulkanApplication::PreUpdate()
{
    // Poll events
    ApplicationAdvancedComputeShaders::PreUpdate();

    // Process continuous inputs
    ProcessInput();
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
    assetManager_->RegisterLoader<TextureAsset>(std::make_unique<TextureLoader>(ASSETS_DIR));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto presentVertexShaderAsset = assetManager_->Load<ShaderAsset>(kPresentVertexShaderFile);
    const auto presentFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kPresentFragmentShaderFile);
    const auto rayTraceComputeShaderAsset = assetManager_->Load<ShaderAsset>(kRayTraceComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kPresentVertexShaderKey, .asset = assetManager_->Get(presentVertexShaderAsset)},
                    {.name = kPresentFragmentShaderKey, .asset = assetManager_->Get(presentFragmentShaderAsset)},
                    {.name = kRayTraceComputeShaderKey, .asset = assetManager_->Get(rayTraceComputeShaderAsset)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = kRadianceImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = kRadianceImageFormat,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .views = {ImageViewCreateInfo{.viewName = kRadianceImageView, .format = kRadianceImageFormat}}}};

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
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;
    sceneConfig.primitiveStackCount = kPrimitiveStackCount;
    sceneConfig.primitiveSectorCount = kPrimitiveSectorCount;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(kCameraStartPosition, aspectRatio);

    const auto cubemapRightTextureAsset = assetManager_->Load<TextureAsset>(kCubemapRightTexturePath);
    const auto cubemapLeftTextureAsset = assetManager_->Load<TextureAsset>(kCubemapLeftTexturePath);
    const auto cubemapTopTextureAsset = assetManager_->Load<TextureAsset>(kCubemapTopTexturePath);
    const auto cubemapBottomTextureAsset = assetManager_->Load<TextureAsset>(kCubemapBottomTexturePath);
    const auto cubemapBackTextureAsset = assetManager_->Load<TextureAsset>(kCubemapBackTexturePath);
    const auto cubemapFrontTextureAsset = assetManager_->Load<TextureAsset>(kCubemapFrontTexturePath);
    [[maybe_unused]] const auto cubemapTextureId = scene_->GetGpuImageStorage().StoreCubemapTexture(
            kCubemapTexture, kSkyboxSampler, assetManager_->Get(cubemapRightTextureAsset),
            assetManager_->Get(cubemapLeftTextureAsset), assetManager_->Get(cubemapTopTextureAsset),
            assetManager_->Get(cubemapBottomTextureAsset), assetManager_->Get(cubemapBackTextureAsset),
            assetManager_->Get(cubemapFrontTextureAsset));

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject).WithPosition(glm::vec3{0.0f});

    for (const auto& desc: kSceneObjects) {
        Material material;
        material.albedoColor = desc.albedoColor;
        material.roughness = desc.roughness;
        material.metallic = desc.metallic;
        material.reflectivity = desc.reflectivity;

        rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, desc.name)
                                           .WithBuiltinMesh(desc.meshType)
                                           .WithMaterial(material)
                                           .WithPosition(desc.position)
                                           .WithEulerAngles(desc.eulerAngles)
                                           .WithScale(desc.scale));
    }

    scene_->AddRootObject(rootObjectBuilder.Build());
}

void VulkanApplication::BuildAccelerationStructure()
{
    BvhBuilder builder;

    // The scene is static, so the BVH is built once over the world-space triangles of every scene object
    for (const auto& desc: kSceneObjects) {
        const auto sceneObject = scene_->FindObjectByName(desc.name);
        if (sceneObject == nullptr) {
            throw std::runtime_error("Scene object not found while building the BVH!");
        }

        const auto primitive = CreateHostPrimitive(desc.meshType);
        builder.AddMesh(primitive->GetMeshPrimitive(), sceneObject->GetWorldMatrix(), sceneObject->GetObjectId());
    }

    builder.Build(kBvhMaxLeafSize);

    const auto& nodes = builder.GetNodes();
    const auto& triangles = builder.GetTriangles();
    bvhNodeCount_ = static_cast<uint32_t>(nodes.size());
    triangleCount_ = static_cast<uint32_t>(triangles.size());

    const auto nodeBufferSize = static_cast<uint32_t>(nodes.size() * sizeof(BvhNodeGpu));
    const auto triangleBufferSize = static_cast<uint32_t>(triangles.size() * sizeof(TriangleGpu));

    resources_->CreateBuffers(
            {BufferResourceCreateInfo{kBvhNodeBuffer, nodeBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
             BufferResourceCreateInfo{kTriangleBuffer, triangleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}});

    resources_->SetBuffer(kBvhNodeBuffer, nodes.data(), nodeBufferSize);
    resources_->SetBuffer(kTriangleBuffer, triangles.data(), triangleBufferSize);

    std::cout << "BVH built with " << bvhNodeCount_ << " nodes and " << triangleCount_ << " triangles." << std::endl;
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
                      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}},
        .layouts = {{.name = kRayTraceDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                   nullptr}}},
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

    std::vector<VkDescriptorBufferInfo> bvhNodeBufferInfos;
    bvhNodeBufferInfos.emplace_back(resources_->GetBuffer(kBvhNodeBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> triangleBufferInfos;
    triangleBufferInfos.emplace_back(resources_->GetBuffer(kTriangleBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorImageInfo> radianceImageSamplerInfos;
    radianceImageSamplerInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                           resources_->GetImageView(kRadianceImage, kRadianceImageView)->GetHandle(),
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    const auto skyboxCubemapImageInfos = scene_->GetGpuImageStorage().GetCubemapDescriptorImageInfo(kCubemapTexture);

    ImageWriteRequest radianceImageRayTraceRequest;
    radianceImageRayTraceRequest.descriptorSetName = kRayTraceDescSet;
    radianceImageRayTraceRequest.bindingIndex = 0;
    radianceImageRayTraceRequest.images = radianceImageStorageImageInfos;
    radianceImageRayTraceRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    BufferWriteRequest bvhNodeBufferRequest;
    bvhNodeBufferRequest.descriptorSetName = kRayTraceDescSet;
    bvhNodeBufferRequest.bindingIndex = 1;
    bvhNodeBufferRequest.buffers = bvhNodeBufferInfos;
    bvhNodeBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest triangleBufferRequest;
    triangleBufferRequest.descriptorSetName = kRayTraceDescSet;
    triangleBufferRequest.bindingIndex = 2;
    triangleBufferRequest.buffers = triangleBufferInfos;
    triangleBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest materialBufferRequest;
    materialBufferRequest.descriptorSetName = kRayTraceDescSet;
    materialBufferRequest.bindingIndex = 3;
    materialBufferRequest.buffers = storageMaterialBufferInfos;
    materialBufferRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest lightUboRequest;
    lightUboRequest.descriptorSetName = kRayTraceDescSet;
    lightUboRequest.bindingIndex = 4;
    lightUboRequest.buffers = lightUboInfos;
    lightUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    ImageWriteRequest skyboxCubemapImageRequest;
    skyboxCubemapImageRequest.descriptorSetName = kRayTraceDescSet;
    skyboxCubemapImageRequest.bindingIndex = 5;
    skyboxCubemapImageRequest.images = skyboxCubemapImageInfos;
    skyboxCubemapImageRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest radianceImagePresentRequest;
    radianceImagePresentRequest.descriptorSetName = kPresentDescSet;
    radianceImagePresentRequest.bindingIndex = 0;
    radianceImagePresentRequest.images = radianceImageSamplerInfos;
    radianceImagePresentRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {bvhNodeBufferRequest, triangleBufferRequest, materialBufferRequest, lightUboRequest},
        .imageWriteRequests = {radianceImageRayTraceRequest, skyboxCubemapImageRequest, radianceImagePresentRequest}};

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
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange rayTracePushConstant;
    rayTracePushConstant.offset = 0;
    rayTracePushConstant.size = sizeof(RayTracePushConstants);
    rayTracePushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    rayTracePipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kRayTraceDescSetLayout)},
                                                            {rayTracePushConstant});

    if (!rayTracePipelineLayout_) {
        throw std::runtime_error("Failed to create ray tracing compute pipeline layout!");
    }

    rayTracePipeline_ = device_->CreateComputePipeline(rayTracePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kRayTraceComputeShaderKey)->GetHandle();
        });
    });

    if (!rayTracePipeline_) {
        throw std::runtime_error("Failed to create ray tracing compute pipeline!");
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
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(presentFramebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const auto radianceImage = resources_->GetImage(kRadianceImage);

    // Compute Phase: Ray tracing
    {
        const auto barrierUndefinedToGeneral = radianceImage->CreateImageMemoryBarrier(
                0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          {barrierUndefinedToGeneral});

        currentCmdBuffer->BindPipeline(rayTracePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, rayTracePipelineLayout_, 0,
                                             {resources_->GetDescriptorSet(kRayTraceDescSet)});

        RayTracePushConstants pushConstants{};
        pushConstants.invViewProj = glm::inverse(camera_->GetProjectionMatrix() * camera_->GetViewMatrix());
        pushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
        pushConstants.frameInfo = glm::uvec4(currentWindowWidth_, currentWindowHeight_, frameCount_, 0U);
        currentCmdBuffer->PushConstants(rayTracePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(RayTracePushConstants), &pushConstants);

        currentCmdBuffer->Dispatch(CeilDiv(currentWindowWidth_, kLocalSizeX),
                                   CeilDiv(currentWindowHeight_, kLocalSizeY), 1);
    }

    // Render Phase: Present the result
    {
        const auto barrierGeneralToShaderRead = radianceImage->CreateImageMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          {barrierGeneralToShaderRead});

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

        currentCmdBuffer->Draw(3, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneUniforms() const
{
    LightUbo lightUbo{};
    lightUbo.lightPosition = glm::vec4(kLightCenter, kLightHalfSize);
    lightUbo.lightColor = glm::vec4(kLightColor, kLightIntensity);
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
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::ray_tracing_compute
