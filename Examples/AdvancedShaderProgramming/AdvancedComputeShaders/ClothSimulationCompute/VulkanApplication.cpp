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

namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
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
    std::vector<ClothParticle> CreateInitialClothParticles()
    {
        std::vector<ClothParticle> particles(kClothParticleCount);

        constexpr auto half = kClothSize * 0.5f;

        for (auto i = 0U; i < kClothVertexPerSide; ++i) {
            const float z = -half + kClothSize * static_cast<float>(i) / static_cast<float>(kClothGridSize);

            for (auto j = 0U; j < kClothVertexPerSide; ++j) {
                const float x = -half + kClothSize * static_cast<float>(j) / static_cast<float>(kClothGridSize);
                const glm::vec3 position = kClothSpawnPosition + glm::vec3(x, 0.0f, z);

                auto& particle = particles[i * kClothVertexPerSide + j];
                particle.position = glm::vec4(position, 1.0f); // Inverse mass = 1.0, no pinned particle
                particle.prevPosition = glm::vec4(position, 0.0f);
                particle.velocity = glm::vec4(0.0f);
                particle.normal = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            }
        }

        return particles;
    }

    std::vector<ClothVertex> CreateInitialClothVertices(const std::vector<ClothParticle>& particles)
    {
        std::vector<ClothVertex> vertices(particles.size());

        for (size_t i = 0; i < particles.size(); ++i) {
            vertices[i].position = glm::vec4(glm::vec3(particles[i].position), 1.0f);
            vertices[i].normal = particles[i].normal;
        }

        return vertices;
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
        UploadInitialClothState();
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
    constexpr auto particleBufferSize = static_cast<std::uint32_t>(sizeof(ClothParticle) * kClothParticleCount);
    constexpr auto vertexBufferSize = static_cast<std::uint32_t>(sizeof(ClothVertex) * kClothParticleCount);

    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kClothUniformBuffer, sizeof(ClothUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kClothParticleBufferA, particleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kClothParticleBufferB, particleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kClothVertexBuffer, vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto sceneVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSceneVertexShaderFile);
    const auto sceneFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSceneFragmentShaderFile);
    const auto clothVertexShaderAsset = assetManager_->Load<ShaderAsset>(kClothVertexShaderFile);
    const auto clothFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kClothFragmentShaderFile);
    const auto clothIntegrationComputeShaderAsset =
            assetManager_->Load<ShaderAsset>(kClothIntegrationComputeShaderFile);
    const auto clothConstraintComputeShaderAsset = assetManager_->Load<ShaderAsset>(kClothConstraintComputeShaderFile);
    const auto clothUpdateComputeShaderAsset = assetManager_->Load<ShaderAsset>(kClothUpdateComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kSceneVertexShaderKey, .asset = assetManager_->Get(sceneVertexShaderAsset)},
            {.name = kSceneFragmentShaderKey, .asset = assetManager_->Get(sceneFragmentShaderAsset)},
            {.name = kClothVertexShaderKey, .asset = assetManager_->Get(clothVertexShaderAsset)},
            {.name = kClothFragmentShaderKey, .asset = assetManager_->Get(clothFragmentShaderAsset)},
            {.name = kClothIntegrationComputeShaderKey,
             .asset = assetManager_->Get(clothIntegrationComputeShaderAsset)},
            {.name = kClothConstraintComputeShaderKey, .asset = assetManager_->Get(clothConstraintComputeShaderAsset)},
            {.name = kClothUpdateComputeShaderKey, .asset = assetManager_->Get(clothUpdateComputeShaderAsset)}}};

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

    resourceCreateInfo.samplers = {
        {.name = kMainSampler, .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::UploadInitialClothState() const
{
    const auto particles = CreateInitialClothParticles();
    const auto vertices = CreateInitialClothVertices(particles);

    const auto particleDataSize = static_cast<std::uint64_t>(particles.size() * sizeof(ClothParticle));
    const auto vertexDataSize = static_cast<std::uint64_t>(vertices.size() * sizeof(ClothVertex));

    resources_->SetBuffer(kClothParticleBufferA, particles.data(), particleDataSize);
    resources_->SetBuffer(kClothParticleBufferB, particles.data(), particleDataSize);
    resources_->SetBuffer(kClothVertexBuffer, vertices.data(), vertexDataSize);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;
    sceneConfig.primitiveStackCount = kClothGridSize;
    sceneConfig.primitiveSectorCount = kClothGridSize;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 1.5f, 8.0f), aspectRatio);

    // Materials
    const auto wallStoneTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneTexturePath);
    const auto wallStoneTextureId =
            sceneImageStorage.StoreTexture(kWallStoneTexture, kMainSampler, assetManager_->Get(wallStoneTextureAsset));
    const auto wallStoneNormalTextureAsset = assetManager_->Load<TextureAsset>(kWallStoneNormalTexturePath);
    const auto wallStoneNormalTextureId =
            sceneImageStorage.StoreTexture(kWallStoneNormalTexture, kMainSampler,
                                           assetManager_->Get(wallStoneNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    const auto fabricDenimPatchTextureAsset = assetManager_->Load<TextureAsset>(kFabricDenimPatchTexturePath);
    const auto fabricDenimPatchTextureId = sceneImageStorage.StoreTexture(
            kFabricDenimPatchTexture, kMainSampler, assetManager_->Get(fabricDenimPatchTextureAsset));

    Material defaultMaterial;
    defaultMaterial.ambientStrength = 0.06f;
    defaultMaterial.specularStrength = 0.35f;
    defaultMaterial.shininess = 64.0f;
    defaultMaterial.diffuseMap = wallStoneTextureId;
    defaultMaterial.normalMap = wallStoneNormalTextureId;

    Material fabricMaterial;
    fabricMaterial.ambientStrength = 0.12f;
    fabricMaterial.specularStrength = 0.10f;
    fabricMaterial.shininess = 16.0f;
    fabricMaterial.uvScale = 2.0f;
    fabricMaterial.diffuseMap = fabricDenimPatchTextureId;

    const auto rootObject = SceneObjectBuilder(*scene_, kRootObject)
                                    .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                    .AddChild(SceneObjectBuilder(*scene_, kSphereObject)
                                                      .WithBuiltinMesh(BuiltinMeshType::SPHERE)
                                                      .WithMaterial(defaultMaterial)
                                                      .WithPosition(kSpherePosition)
                                                      .WithScale(glm::vec3{kSphereScale}))
                                    .AddChild(SceneObjectBuilder(*scene_, kGroundObject)
                                                      .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                      .WithMaterial(defaultMaterial)
                                                      .WithPosition(glm::vec3{0.0f, kGroundLevel, 0.0f})
                                                      .WithScale(glm::vec3{kGroundScale}))
                                    .AddChild(SceneObjectBuilder(*scene_, kClothObject)
                                                      .WithTag(kClothTag)
                                                      .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                      .WithMaterial(fabricMaterial)
                                                      .WithPosition(kClothSpawnPosition)
                                                      .WithScale(glm::vec3{kClothSize}))
                                    .Build();

    scene_->AddRootObject(rootObject);
}

void VulkanApplication::ResetClothSimulation() const
{
    device_->WaitIdle();
    UploadInitialClothState();
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 3,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}},
                    {.name = kClothDescriptorSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kClothDescriptorSetPing, .layoutName = kClothDescriptorSetLayout},
                           {.name = kClothDescriptorSetPong, .layoutName = kClothDescriptorSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> clothUboInfos;
    clothUboInfos.emplace_back(resources_->GetBuffer(kClothUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> particleBufferAInfos;
    particleBufferAInfos.emplace_back(resources_->GetBuffer(kClothParticleBufferA)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> particleBufferBInfos;
    particleBufferBInfos.emplace_back(resources_->GetBuffer(kClothParticleBufferB)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> clothVertexBufferInfos;
    clothVertexBufferInfos.emplace_back(resources_->GetBuffer(kClothVertexBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();

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

    ImageWriteRequest textureUpdateRequest;
    textureUpdateRequest.descriptorSetName = kMainDescSet;
    textureUpdateRequest.bindingIndex = 3;
    textureUpdateRequest.images = descriptorImageInfos;
    textureUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest clothVertexReadRequest;
    clothVertexReadRequest.descriptorSetName = kMainDescSet;
    clothVertexReadRequest.bindingIndex = 4;
    clothVertexReadRequest.buffers = clothVertexBufferInfos;
    clothVertexReadRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest pingUboRequest;
    pingUboRequest.descriptorSetName = kClothDescriptorSetPing;
    pingUboRequest.bindingIndex = 0;
    pingUboRequest.buffers = clothUboInfos;
    pingUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    BufferWriteRequest pingInRequest;
    pingInRequest.descriptorSetName = kClothDescriptorSetPing;
    pingInRequest.bindingIndex = 1;
    pingInRequest.buffers = particleBufferAInfos;
    pingInRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest pingOutRequest;
    pingOutRequest.descriptorSetName = kClothDescriptorSetPing;
    pingOutRequest.bindingIndex = 2;
    pingOutRequest.buffers = particleBufferBInfos;
    pingOutRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest pingVertexRequest;
    pingVertexRequest.descriptorSetName = kClothDescriptorSetPing;
    pingVertexRequest.bindingIndex = 3;
    pingVertexRequest.buffers = clothVertexBufferInfos;
    pingVertexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;


    BufferWriteRequest pongUboRequest;
    pongUboRequest.descriptorSetName = kClothDescriptorSetPong;
    pongUboRequest.bindingIndex = 0;
    pongUboRequest.buffers = clothUboInfos;
    pongUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    BufferWriteRequest pongInRequest;
    pongInRequest.descriptorSetName = kClothDescriptorSetPong;
    pongInRequest.bindingIndex = 1;
    pongInRequest.buffers = particleBufferBInfos;
    pongInRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest pongOutRequest;
    pongOutRequest.descriptorSetName = kClothDescriptorSetPong;
    pongOutRequest.bindingIndex = 2;
    pongOutRequest.buffers = particleBufferAInfos;
    pongOutRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest pongVertexRequest;
    pongVertexRequest.descriptorSetName = kClothDescriptorSetPong;
    pongVertexRequest.bindingIndex = 3;
    pongVertexRequest.buffers = clothVertexBufferInfos;
    pongVertexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest, clothVertexReadRequest, pingUboRequest, pingInRequest, pingOutRequest,
                                pingVertexRequest, pongUboRequest, pongInRequest, pongOutRequest, pongVertexRequest},
        .imageWriteRequests = {textureUpdateRequest}};

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

    window_->OnKey([&](const KeyEvent& event) {
        if (event.action != GLFW_PRESS) {
            return;
        }

        // Restart the simulation
        if (event.key == GLFW_KEY_R) {
            ResetClothSimulation();
            std::cout << "Cloth simulation restarted!" << std::endl;
        }

        // Pause/resume the simulation
        if (event.key == GLFW_KEY_SPACE) {
            isSimulationPaused_ = !isSimulationPaused_;
            std::cout << "Cloth simulation: " << (isSimulationPaused_ ? "PAUSED" : "RUNNING") << std::endl;
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
    clothComputePipelineLayout_ =
            device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kClothDescriptorSetLayout)});

    if (!clothComputePipelineLayout_) {
        throw std::runtime_error("Failed to create cloth compute pipeline layout!");
    }

    clothIntegratePipeline_ = device_->CreateComputePipeline(clothComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClothIntegrationComputeShaderKey)->GetHandle();
        });
    });

    if (!clothIntegratePipeline_) {
        throw std::runtime_error("Failed to create cloth integrate pipeline!");
    }

    clothConstraintPipeline_ = device_->CreateComputePipeline(clothComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClothConstraintComputeShaderKey)->GetHandle();
        });
    });

    if (!clothConstraintPipeline_) {
        throw std::runtime_error("Failed to create cloth constraint pipeline!");
    }

    clothUpdatePipeline_ = device_->CreateComputePipeline(clothComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClothUpdateComputeShaderKey)->GetHandle();
        });
    });

    if (!clothUpdatePipeline_) {
        throw std::runtime_error("Failed to create cloth update pipeline!");
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

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
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

    clothPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClothVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kClothFragmentShaderKey)->GetHandle();
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

    if (!clothPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for cloth objects)!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordClothSimulation(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    const auto particleBufferA = resources_->GetBuffer(kClothParticleBufferA);
    const auto particleBufferB = resources_->GetBuffer(kClothParticleBufferB);
    const auto clothVertexBuffer = resources_->GetBuffer(kClothVertexBuffer);

    const std::vector pingSet{resources_->GetDescriptorSet(kClothDescriptorSetPing)}; // A to B
    const std::vector pongSet{resources_->GetDescriptorSet(kClothDescriptorSetPong)}; // B to A

    // Every pass swaps the input/output buffers, a sub-step contains (1 + iterations + 1) passes
    // So, an even iteration count guarantees that frame always starts and ends with buffer A
    bool usePingSet = true;
    const auto bindNextSet = [&] {
        cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, clothComputePipelineLayout_, 0,
                                      usePingSet ? pingSet : pongSet);
        usePingSet = !usePingSet;
    };

    const auto insertComputeBarrier = [&] {
        const std::vector barriers{
            particleBufferA->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
                                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
            particleBufferB->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT,
                                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)};
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
                                   barriers);
    };

    insertComputeBarrier();

    for (auto subStep = 0U; subStep < kSubStepCount; ++subStep) {
        // Integrate Phase: Gravity, damping and position prediction
        cmdBuffer->BindPipeline(clothIntegratePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        bindNextSet();
        cmdBuffer->Dispatch(kClothGroupCount, kClothGroupCount, 1);
        insertComputeBarrier();

        // Constraint Phase: Solve distance constraints and collisions (Jacobi iterations)
        cmdBuffer->BindPipeline(clothConstraintPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        for (auto iteration = 0U; iteration < kConstraintIterationCount; ++iteration) {
            bindNextSet();
            cmdBuffer->Dispatch(kClothGroupCount, kClothGroupCount, 1);
            insertComputeBarrier();
        }

        // Update Phase: Derive velocities, rebuild normals and fill the render vertex buffer
        cmdBuffer->BindPipeline(clothUpdatePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
        bindNextSet();
        cmdBuffer->Dispatch(kClothGroupCount, kClothGroupCount, 1);
        insertComputeBarrier();
    }

    // From compute shader writes to vertex shader reads
    const std::vector vertexReadBarriers{
        clothVertexBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)};
    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, {},
                               vertexReadBarriers);
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

    // Compute Phase: Cloth simulation
    if (!isSimulationPaused_) {
        RecordClothSimulation(currentCmdBuffer);
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

    const auto drawSceneObject = [&](const SceneObject& sceneObject) {
        const auto [vertexOffsets, indexOffset, indexCount] = sceneObject.GetMeshGpu().value();
        currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, vertexBuffers.size(), vertexOffsets);
        currentCmdBuffer->BindIndexBuffer(scene_->GetGeometryBuffer(), indexOffset);

        MeshPushConstants meshPushConstants{};
        meshPushConstants.objectId = sceneObject.GetObjectId();
        meshPushConstants.view = camera_->GetViewMatrix();
        meshPushConstants.projection = camera_->GetProjectionMatrix();
        meshPushConstants.cameraPosition = glm::vec4(camera_->GetPosition(), 1.0f);
        currentCmdBuffer->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(meshPushConstants), &meshPushConstants);
        currentCmdBuffer->DrawIndexed(indexCount, 1, 0, 0, 0);
    };

    // Render Pass: Draw static scene objects
    currentCmdBuffer->BindPipeline(scenePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() != kClothTag) {
            drawSceneObject(sceneObject);
        }
    });

    // Render Pass: Draw cloth objects
    currentCmdBuffer->BindPipeline(clothPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() == kClothTag) {
            drawSceneObject(sceneObject);
        }
    });

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    LightUbo lightUbo{};
    lightUbo.lightDirection = glm::vec4(kLightDirection, 1.0f);
    lightUbo.lightColor = glm::vec4(kLightColor, 1.0f);
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));

    ClothUbo clothUbo{};
    clothUbo.gravityDeltaTime = glm::vec4(kGravity, kFixedTimeStep / static_cast<float>(kSubStepCount));
    clothUbo.sphereCenterRadius = glm::vec4(kSpherePosition, kSphereRadius);
    clothUbo.params = glm::vec4(kClothRestDistance, kVelocityDamping, kCollisionFriction, kGroundLevel);
    clothUbo.stiffness = glm::vec4(kClothThickness, kStructuralStiffness, kShearStiffness, kBendStiffness);
    clothUbo.gridInfo = glm::uvec4(kClothVertexPerSide, kClothParticleCount, 0U, 0U);
    resources_->SetBuffer(kClothUniformBuffer, &clothUbo, sizeof(clothUbo));
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
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::cloth_simulation_compute
