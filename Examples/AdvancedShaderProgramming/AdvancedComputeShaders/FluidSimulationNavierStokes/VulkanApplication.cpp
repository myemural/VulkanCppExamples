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
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

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

    UpdateSceneUniforms();
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
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    resourceCreateInfo.buffers = {{kSceneUniformBuffer, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto sceneVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSceneVertexShaderFile);
    const auto sceneFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneFragmentShaderFile);
    const auto smokeFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSmokeFragmentShaderFile);
    const auto advectComputeAsset = assetManager_->Load<ShaderAsset>(kAdvectComputeShaderFile);
    const auto jacobiComputeAsset = assetManager_->Load<ShaderAsset>(kJacobiComputeShaderFile);
    const auto projectComputeAsset = assetManager_->Load<ShaderAsset>(kProjectComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
                    {.name = kSceneFragmentShaderKey, .asset = assetManager_->Get(sceneFragmentShaderAsset)},
                    {.name = kSmokeFragmentShaderKey, .asset = assetManager_->Get(smokeFragmentShaderAsset)},
                    {.name = kAdvectComputeShaderKey, .asset = assetManager_->Get(advectComputeAsset)},
                    {.name = kJacobiComputeShaderKey, .asset = assetManager_->Get(jacobiComputeAsset)},
                    {.name = kProjectComputeShaderKey, .asset = assetManager_->Get(projectComputeAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{.name = kFieldImageA,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .imageType = VK_IMAGE_TYPE_3D,
                                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                .dimensions = {kGridSizeX, kGridSizeY, kGridSizeZ},
                                .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kFieldImageViewA,
                                                              .viewType = VK_IMAGE_VIEW_TYPE_3D,
                                                              .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{.name = kFieldImageB,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .imageType = VK_IMAGE_TYPE_3D,
                                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                .dimensions = {kGridSizeX, kGridSizeY, kGridSizeZ},
                                .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kFieldImageViewB,
                                                              .viewType = VK_IMAGE_VIEW_TYPE_3D,
                                                              .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{.name = kPressureImageA,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .imageType = VK_IMAGE_TYPE_3D,
                                .format = VK_FORMAT_R32_SFLOAT,
                                .dimensions = {kGridSizeX, kGridSizeY, kGridSizeZ},
                                .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kPressureImageViewA,
                                                              .viewType = VK_IMAGE_VIEW_TYPE_3D,
                                                              .format = VK_FORMAT_R32_SFLOAT}}},
        ImageResourceCreateInfo{.name = kPressureImageB,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .imageType = VK_IMAGE_TYPE_3D,
                                .format = VK_FORMAT_R32_SFLOAT,
                                .dimensions = {kGridSizeX, kGridSizeY, kGridSizeZ},
                                .usageFlags = VK_IMAGE_USAGE_STORAGE_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kPressureImageViewB,
                                                              .viewType = VK_IMAGE_VIEW_TYPE_3D,
                                                              .format = VK_FORMAT_R32_SFLOAT}}},
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
        {.name = kVolumeSampler,
         .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR},
         .addressModes = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
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

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, orbitDistance_), aspectRatio);
    UpdateOrbitCamera();

    Material groundMaterial;
    groundMaterial.diffuseColor = glm::vec4(0.32f, 0.33f, 0.36f, 1.0f);
    groundMaterial.ambientStrength = 0.25f;
    groundMaterial.specularStrength = 0.1f;
    groundMaterial.shininess = 24.0f;

    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject)
                                     .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                     .AddChild(SceneObjectBuilder(*scene_, kGroundPlane)
                                                       .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                       .WithMaterial(groundMaterial)
                                                       .WithPosition(glm::vec3{0.0f, kGroundLevel, 0.0f})
                                                       .WithScale(glm::vec3{kGroundSize}))
                                     .AddChild(SceneObjectBuilder(*scene_, kSmokeVolume)
                                                       .WithTag(kSmokeGroup)
                                                       .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                                       .WithMaterial(Material{})
                                                       .WithPosition((kVolumeMin + kVolumeMax) * 0.5f)
                                                       .WithScale(kVolumeMax - kVolumeMin));

    scene_->AddRootObject(rootObjectBuilder.Build());
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 5,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2},
                      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 11}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}},
                    {.name = kAdvectDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}},
                    {.name = kJacobiDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}},
                    {.name = kProjectDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kAdvectDescSet, .layoutName = kAdvectDescSetLayout},
                           {.name = kJacobiDescSetAToB, .layoutName = kJacobiDescSetLayout},
                           {.name = kJacobiDescSetBToA, .layoutName = kJacobiDescSetLayout},
                           {.name = kProjectDescSet, .layoutName = kProjectDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    const auto volumeSampler = resources_->GetSampler(kVolumeSampler)->GetHandle();

    const auto bufferWrite = [](const std::string& setName, const std::uint32_t binding, const VkDescriptorType type,
                                const std::shared_ptr<VulkanBuffer>& buffer) {
        BufferWriteRequest request;
        request.descriptorSetName = setName;
        request.bindingIndex = binding;
        request.buffers = {{buffer->GetHandle(), 0, VK_WHOLE_SIZE}};
        request.type = type;
        return request;
    };

    const auto imageWrite = [&](const std::string& setName, const std::uint32_t binding, const VkDescriptorType type,
                                const std::string& imageName, const std::string& viewName) {
        ImageWriteRequest request;
        request.descriptorSetName = setName;
        request.bindingIndex = binding;
        request.images = {{type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? volumeSampler : VK_NULL_HANDLE,
                           resources_->GetImageView(imageName, viewName)->GetHandle(), VK_IMAGE_LAYOUT_GENERAL}};
        request.type = type;
        return request;
    };

    constexpr auto kStorage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    constexpr auto kSampled = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {bufferWrite(kMainDescSet, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                            scene_->GetTransformStorageBuffer()),
                                bufferWrite(kMainDescSet, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                            scene_->GetMaterialStorageBuffer()),
                                bufferWrite(kMainDescSet, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            resources_->GetBuffer(kSceneUniformBuffer))},
        .imageWriteRequests = {
            imageWrite(kMainDescSet, 3, kSampled, kFieldImageA, kFieldImageViewA),

            imageWrite(kAdvectDescSet, 0, kSampled, kFieldImageA, kFieldImageViewA),
            imageWrite(kAdvectDescSet, 1, kStorage, kFieldImageB, kFieldImageViewB),
            imageWrite(kAdvectDescSet, 2, kStorage, kPressureImageA, kPressureImageViewA),

            imageWrite(kJacobiDescSetAToB, 0, kStorage, kFieldImageB, kFieldImageViewB),
            imageWrite(kJacobiDescSetAToB, 1, kStorage, kPressureImageA, kPressureImageViewA),
            imageWrite(kJacobiDescSetAToB, 2, kStorage, kPressureImageB, kPressureImageViewB),
            imageWrite(kJacobiDescSetBToA, 0, kStorage, kFieldImageB, kFieldImageViewB),
            imageWrite(kJacobiDescSetBToA, 1, kStorage, kPressureImageB, kPressureImageViewB),
            imageWrite(kJacobiDescSetBToA, 2, kStorage, kPressureImageA, kPressureImageViewA),

            imageWrite(kProjectDescSet, 0, kStorage, kFieldImageB, kFieldImageViewB),
            imageWrite(kProjectDescSet, 1, kStorage, kPressureImageA, kPressureImageViewA),
            imageWrite(kProjectDescSet, 2, kStorage, kFieldImageA, kFieldImageViewA),
        }};

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

        UpdateOrbitCamera();
    });

    // For zooming to target
    window_->OnMouseScroll([this](const MouseScrollEvent& event) {
        constexpr auto minZoom = 0.2f;
        constexpr auto maxZoom = 90.0f;
        orbitDistance_ = glm::clamp(orbitDistance_ - static_cast<float>(event.deltaY) *
                                                             GetParamFloat(AppSettings::CameraZoomSpeed),
                                    minZoom, maxZoom);

        UpdateOrbitCamera();
    });

    // For turning on/off wind volume debug box and debug lines
    window_->OnKey([&](const KeyEvent& event) {
        if (event.action != GLFW_PRESS) {
            return;
        }

        if (event.key == GLFW_KEY_SPACE) {
            simulationPaused_ = !simulationPaused_;
            std::cout << "Simulation: " << (simulationPaused_ ? "PAUSED" : "RUNNING") << std::endl;
        } else if (event.key == GLFW_KEY_R) {
            seedRequested_ = true;
            std::cout << "Simulation reset!" << std::endl;
        }
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
    VkPushConstantRange fluidPushConstant;
    fluidPushConstant.offset = 0;
    fluidPushConstant.size = sizeof(FluidPushConstants);
    fluidPushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    advectPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kAdvectDescSetLayout)}, {fluidPushConstant});

    if (!advectPipelineLayout_) {
        throw std::runtime_error("Failed to create advect compute pipeline layout!");
    }

    advectPipeline_ = device_->CreateComputePipeline(advectPipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kAdvectComputeShaderKey)->GetHandle();
        });
    });

    if (!advectPipeline_) {
        throw std::runtime_error("Failed to create advect compute pipeline!");
    }

    jacobiPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kJacobiDescSetLayout)}, {fluidPushConstant});

    if (!jacobiPipelineLayout_) {
        throw std::runtime_error("Failed to create jacobi compute pipeline layout!");
    }

    jacobiPipeline_ = device_->CreateComputePipeline(jacobiPipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kJacobiComputeShaderKey)->GetHandle();
        });
    });

    if (!jacobiPipeline_) {
        throw std::runtime_error("Failed to create jacobi compute pipeline!");
    }

    projectPipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kProjectDescSetLayout)},
                                                           {fluidPushConstant});

    if (!projectPipelineLayout_) {
        throw std::runtime_error("Failed to create project compute pipeline layout!");
    }

    projectPipeline_ = device_->CreateComputePipeline(projectPipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kProjectComputeShaderKey)->GetHandle();
        });
    });

    if (!projectPipeline_) {
        throw std::runtime_error("Failed to create project compute pipeline!");
    }

    VkPushConstantRange mvpPushConstant;
    mvpPushConstant.offset = 0;
    mvpPushConstant.size = sizeof(MeshPushConstants);
    mvpPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {mvpPushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState opaqueBlendAttachment{};
    opaqueBlendAttachment.blendEnable = VK_FALSE;
    opaqueBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    scenePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneFragmentShaderKey)->GetHandle();
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
            blendStateCreateInfo.pAttachments = &opaqueBlendAttachment;
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

    VkPipelineColorBlendAttachmentState smokeBlendAttachment{};
    smokeBlendAttachment.blendEnable = VK_TRUE;
    smokeBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    smokeBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    smokeBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    smokeBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    smokeBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    smokeBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    smokeBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


    smokePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSceneVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSmokeFragmentShaderKey)->GetHandle();
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
        builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
            rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
            rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &smokeBlendAttachment;
        });
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!smokePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for smoke)!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordSimulationPasses(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    FluidPushConstants pushConstants{};
    pushConstants.params = glm::vec4{kTimeStep, kBuoyancy, kDissipation, kCellSize};
    pushConstants.gridInfo = glm::uvec4{kGridSizeX, kGridSizeY, kGridSizeZ, seedRequested_ ? 1U : 0U};

    const auto groupCountX = CeilDiv(kGridSizeX, kLocalSizeX);
    const auto groupCountY = CeilDiv(kGridSizeY, kLocalSizeY);
    const auto groupCountZ = CeilDiv(kGridSizeZ, kLocalSizeZ);

    const auto computePass = [&](const std::shared_ptr<VulkanPipeline>& pipeline,
                                 const std::shared_ptr<VulkanPipelineLayout>& pipelineLayout,
                                 const std::string& descSetName) {
        cmdBuffer->BindPipeline(pipeline, VK_PIPELINE_BIND_POINT_COMPUTE);
        cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0,
                                      {resources_->GetDescriptorSet(descSetName)});
        cmdBuffer->PushConstants(pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FluidPushConstants),
                                 &pushConstants);
        cmdBuffer->Dispatch(groupCountX, groupCountY, groupCountZ);

        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, {},
                                   {kComputeBarrier});
    };

    // Step 1: Semi-Lagrangian advection and buoyancy
    computePass(advectPipeline_, advectPipelineLayout_, kAdvectDescSet);

    // Step 2: Jacobi sweeps of the pressure Poisson equation
    for (auto iteration = 0U; iteration < kJacobianIterations; ++iteration) {
        computePass(jacobiPipeline_, jacobiPipelineLayout_,
                    (iteration % 2 == 0U) ? kJacobiDescSetAToB : kJacobiDescSetBToA);
    }

    // Step 3: Subtract the pressure gradient so the velocity field becomes divergence free
    computePass(projectPipeline_, projectPipelineLayout_, kProjectDescSet);

    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {},
                               {kRenderBarrier});
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

    // One-time from UNDEFINED to GENERAL transition
    if (!volumesInitialized_) {
        constexpr std::array kVolumeImageNames = {kFieldImageA, kFieldImageB, kPressureImageA, kPressureImageB};

        std::vector<VkImageMemoryBarrier> initialBarriers;
        initialBarriers.reserve(kVolumeImageNames.size());

        for (const auto& volumeName: kVolumeImageNames) {
            initialBarriers.push_back(
                    resources_->GetImage(volumeName)
                            ->CreateImageMemoryBarrier(0, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));
        }

        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                          initialBarriers);

        volumesInitialized_ = true;
    }

    // A reset still has to run once even while paused, otherwise nothing would change on screen
    if (!simulationPaused_ || seedRequested_) {
        RecordSimulationPasses(currentCmdBuffer);
        seedRequested_ = false;
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

    // Draw opaque scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->TraverseOrdered(camera_->GetPosition(), [&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() != kSmokeGroup) {
            const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            MeshPushConstants meshPushConstants{};
            meshPushConstants.objectId = sceneObject.GetObjectId();
            meshPushConstants.view = camera_->GetViewMatrix();
            meshPushConstants.projection = camera_->GetProjectionMatrix();
            meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            0, sizeof(meshPushConstants), &meshPushConstants);
            currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
        }
    });

    // Draw transparent smoke box, which the ray marcher fills in
    currentCmdBuffer->BindPipeline(smokePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->TraverseOrdered(camera_->GetPosition(), [&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() == kSmokeGroup) {
            const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
            currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
            currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

            MeshPushConstants meshPushConstants{};
            meshPushConstants.objectId = sceneObject.GetObjectId();
            meshPushConstants.view = camera_->GetViewMatrix();
            meshPushConstants.projection = camera_->GetProjectionMatrix();
            meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
            currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                            0, sizeof(meshPushConstants), &meshPushConstants);
            currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
        }
    });

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneUniforms() const
{
    SceneUbo sceneUbo{};
    sceneUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
    sceneUbo.lightColor = glm::vec4(kLightColor, 1.0f);
    sceneUbo.volumeMin = glm::vec4(kVolumeMin, kSmokeAbsorption);
    sceneUbo.volumeMax = glm::vec4(kVolumeMax, 0.0f);
    resources_->SetBuffer(kSceneUniformBuffer, &sceneUbo, sizeof(sceneUbo));
}

void VulkanApplication::UpdateOrbitCamera() const
{
    const float yawRad = glm::radians(orbitYaw_);
    const float pitchRad = glm::radians(orbitPitch_);

    glm::vec3 position;
    position.x = kOrbitTarget.x + orbitDistance_ * cosf(pitchRad) * cosf(yawRad);
    position.y = kOrbitTarget.y + orbitDistance_ * sinf(pitchRad);
    position.z = kOrbitTarget.z + orbitDistance_ * cosf(pitchRad) * sinf(yawRad);

    camera_->SetPosition(position);

    // Look to the target
    const glm::vec3 front = glm::normalize(kOrbitTarget - position);
    const float viewYaw = glm::degrees(atan2(front.z, front.x));
    const float viewPitch = glm::degrees(asin(front.y));

    camera_->SetRotation(viewYaw, viewPitch);
}

} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
