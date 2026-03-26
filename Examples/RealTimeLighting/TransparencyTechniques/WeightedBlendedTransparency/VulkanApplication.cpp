/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
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

namespace examples::real_time_lighting::transparency_techniques::weighted_blended_transparency
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::scene;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationTransparencyTechniques(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationTransparencyTechniques::Init()) {
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
    ApplicationTransparencyTechniques::PreUpdate();

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
    const auto transparencyVertexShaderAsset = assetManager_->Load<ShaderAsset>(kTransparencyVertexShaderFile);
    const auto transparencyFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kTransparencyFragmentShaderFile);
    const auto compositionVertexShaderAsset = assetManager_->Load<ShaderAsset>(kCompositionVertexShaderFile);
    const auto compositionFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kCompositionFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kTransparencyVertexShaderKey, .asset = assetManager_->Get(transparencyVertexShaderAsset)},
            {.name = kTransparencyFragmentShaderKey, .asset = assetManager_->Get(transparencyFragmentShaderAsset)},
            {.name = kCompositionVertexShaderKey, .asset = assetManager_->Get(compositionVertexShaderAsset)},
            {.name = kCompositionFragmentShaderKey, .asset = assetManager_->Get(compositionFragmentShaderAsset)}}};

    resourceCreateInfo.images = {
        ImageResourceCreateInfo{.name = kAccumulationImage,
                                .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                                .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
                                .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                .views = {ImageViewCreateInfo{.viewName = kAccumulationImageView,
                                                              .format = VK_FORMAT_R16G16B16A16_SFLOAT}}},
        ImageResourceCreateInfo{
            .name = kRevealageImage,
            .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .format = VK_FORMAT_R8_UNORM,
            .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .views = {ImageViewCreateInfo{.viewName = kRevealageImageView, .format = VK_FORMAT_R8_UNORM}}},
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

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout.emplace_back(AttributeType::POSITION, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TEXCOORD, AccessorType::VEC2);
    sceneConfig.attributeLayout.emplace_back(AttributeType::NORMAL, AccessorType::VEC3);
    sceneConfig.attributeLayout.emplace_back(AttributeType::TANGENT, AccessorType::VEC4);
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;

    materialManager_ = std::make_unique<MaterialManager>(*resources_, cmdPool_, queue_);
    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 2.0f, 7.0f), aspectRatio);

    // Add scene objects
    std::uint32_t index = 0;
    auto rootObjectBuilder = SceneObjectBuilder(*scene_, kRootObject);
    for (const auto& modelPos: modelPositions) {
        Material transparentMaterial{};
        transparentMaterial.diffuseColor = glm::vec4(GenerateRandomColor(0.1f, 1.0f), 1.0f);
        transparentMaterial.ambientStrength = GetParamFloat(AppSettings::AmbientStrength);
        transparentMaterial.shininess = GetParamFloat(AppSettings::Shininess);
        transparentMaterial.specularStrength = GetParamFloat(AppSettings::SpecularStrength);
        transparentMaterial.opacity = GenerateRandomValue(0.1f, 0.8f);

        if (const auto value = GenerateRandomValue(0U, 1U); value == 0) {
            rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kCubeObject + std::to_string(index))
                                               .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                               .WithMaterial(transparentMaterial)
                                               .WithPosition(modelPos)
                                               .WithScale(glm::vec3{2.0f}));

        } else {
            rootObjectBuilder.AddChild(SceneObjectBuilder(*scene_, kPlaneObject + std::to_string(index))
                                               .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                               .WithMaterial(transparentMaterial)
                                               .WithPosition(modelPos)
                                               .WithEulerAngles(glm::vec3(90.0f, 0.0f, 0.0f))
                                               .WithScale(glm::vec3{2.0f}));
        }

        index++;
    }

    const auto& rootObject = rootObjectBuilder.Build();
    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 5,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}},
                    {.name = kCompositionDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kCompositionDescSet, .layoutName = kCompositionDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorImageInfo> transparencyAccImageInfos;
    transparencyAccImageInfos.emplace_back(
            resources_->GetSampler(kMainSampler)->GetHandle(),
            resources_->GetImageView(kAccumulationImage, kAccumulationImageView)->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> revealageImageInfos;
    revealageImageInfos.emplace_back(resources_->GetSampler(kMainSampler)->GetHandle(),
                                     resources_->GetImageView(kRevealageImage, kRevealageImageView)->GetHandle(),
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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

    ImageWriteRequest compositeAccumulationImageUpdateRequest;
    compositeAccumulationImageUpdateRequest.descriptorSetName = kCompositionDescSet;
    compositeAccumulationImageUpdateRequest.bindingIndex = 0;
    compositeAccumulationImageUpdateRequest.images = transparencyAccImageInfos;
    compositeAccumulationImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    ImageWriteRequest compositeRevealageImageUpdateRequest;
    compositeRevealageImageUpdateRequest.descriptorSetName = kCompositionDescSet;
    compositeRevealageImageUpdateRequest.bindingIndex = 1;
    compositeRevealageImageUpdateRequest.images = revealageImageInfos;
    compositeRevealageImageUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest},
        .imageWriteRequests = {compositeAccumulationImageUpdateRequest, compositeRevealageImageUpdateRequest}};

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
    VkAttachmentReference colorAttachmentRefs[2]{
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}, // accumulation
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}  // revealage
    };

    VkAttachmentReference depthAttachmentRef{2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    transparencyAccRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               })
                .AddAttachment([](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = VK_FORMAT_R8_UNORM;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
                    subpassCreateInfo.colorAttachmentCount = 2;
                    subpassCreateInfo.pColorAttachments = colorAttachmentRefs;
                    subpassCreateInfo.pDepthStencilAttachment = &depthAttachmentRef;
                });
    });

    if (!transparencyAccRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for transparency accumulation)!");
    }

    VkAttachmentReference compositeColorAttachment{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    compositeRenderPass_ = device_->CreateRenderPass([&](auto& builder) {
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
                    subpassCreateInfo.pColorAttachments = &compositeColorAttachment;
                });
    });

    if (!compositeRenderPass_) {
        throw std::runtime_error("Failed to create render pass (for composition)!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange meshPushConstant;
    meshPushConstant.offset = 0;
    meshPushConstant.size = sizeof(MeshPushConstants);
    meshPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    transparencyAccPipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)}, {meshPushConstant});

    if (!transparencyAccPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for transparency accumulation)!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    const auto bindings = scene_->GetBindingDescriptions();
    const auto attributes = scene_->GetAttributeDescriptions();

    VkPipelineColorBlendAttachmentState accumBlend{};
    accumBlend.blendEnable = VK_TRUE;
    accumBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    accumBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    accumBlend.colorBlendOp = VK_BLEND_OP_ADD;
    accumBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    accumBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    accumBlend.alphaBlendOp = VK_BLEND_OP_ADD;
    accumBlend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendAttachmentState revealBlend{};
    revealBlend.blendEnable = VK_TRUE;
    revealBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    revealBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    revealBlend.colorBlendOp = VK_BLEND_OP_ADD;
    revealBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    revealBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    revealBlend.alphaBlendOp = VK_BLEND_OP_ADD;
    revealBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;

    std::array transparencyPipelineColorBlendAttachments = {accumBlend, revealBlend};

    transparencyAccPipeline_ = device_->CreateGraphicsPipeline(
            transparencyAccPipelineLayout_, transparencyAccRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kTransparencyVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kTransparencyFragmentShaderKey)->GetHandle();
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
                    blendStateCreateInfo.attachmentCount = transparencyPipelineColorBlendAttachments.size();
                    blendStateCreateInfo.pAttachments = transparencyPipelineColorBlendAttachments.data();
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                });
            });

    if (!transparencyAccPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for transparency accumulation)!");
    }

    compositePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kCompositionDescSetLayout)});

    if (!compositePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for composition)!");
    }

    VkPipelineColorBlendAttachmentState compositePipelineColorBlend{};
    compositePipelineColorBlend.blendEnable = VK_FALSE;
    compositePipelineColorBlend.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    compositePipeline_ =
            device_->CreateGraphicsPipeline(compositePipelineLayout_, compositeRenderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kCompositionVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kCompositionFragmentShaderKey)->GetHandle();
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
                    blendStateCreateInfo.pAttachments = &compositePipelineColorBlend;
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
                });
            });

    if (!compositePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for composition)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    // Transparency accumulation framebuffer
    const auto& accumulationImageView = resources_->GetImageView(kAccumulationImage, kAccumulationImageView);
    const auto& revealageImageView = resources_->GetImageView(kRevealageImage, kRevealageImageView);
    const auto& depthImageView = resources_->GetImageView(kDepthImage, kDepthImageView);
    transparencyFramebuffer_ = device_->CreateFramebuffer(
            transparencyAccRenderPass_, {accumulationImageView, revealageImageView, depthImageView},
            [&](auto& builder) { builder.SetDimensions(currentWindowWidth_, currentWindowHeight_); });

    if (!transparencyFramebuffer_) {
        throw std::runtime_error("Failed to create framebuffer (for geometry)!");
    }

    // Present framebuffers
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(compositeRenderPass_, {swapImage}, [&](auto& builder) {
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
    std::array<VkClearValue, 3> transparencyClearValues{};
    transparencyClearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f}; // accumulation
    transparencyClearValues[1].color = {1.0f, 0.0f, 0.0f, 0.0f}; // revealage
    transparencyClearValues[2].depthStencil = {1.0f, 0};

    VkClearValue compositeClear{};
    compositeClear.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Transparency Accumulation Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = transparencyAccRenderPass_->GetHandle();
                    beginInfo.framebuffer = transparencyFramebuffer_->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = transparencyClearValues.size();
                    beginInfo.pClearValues = transparencyClearValues.data();
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector descSets{resources_->GetDescriptorSet(kMainDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, transparencyAccPipelineLayout_, 0,
                                             descSets);
        const std::vector vertexBuffers(scene_->GetAttributeCount(), scene_->GetGeometryBuffer());

        // Draw transparent objects
        currentCmdBuffer->BindPipeline(transparencyAccPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
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
                currentCmdBuffer->PushConstants(transparencyAccPipelineLayout_,
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                sizeof(meshPushConstants), &meshPushConstants);
                currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
            }
        });

        currentCmdBuffer->EndRenderPass();
    }

    // Composite Pass
    {
        currentCmdBuffer->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = compositeRenderPass_->GetHandle();
                    beginInfo.framebuffer = presentFramebuffers_[currentImageIndex]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &compositeClear;
                },
                VK_SUBPASS_CONTENTS_INLINE);

        const std::vector compositeDescSets{resources_->GetDescriptorSet(kCompositionDescSet)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, compositePipelineLayout_, 0,
                                             compositeDescSets);
        currentCmdBuffer->BindPipeline(compositePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, 1, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightDirection), 1.0f);
    lightUbo.lightColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::LightColor), 1.0f);
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
} // namespace examples::real_time_lighting::transparency_techniques::weighted_blended_transparency
