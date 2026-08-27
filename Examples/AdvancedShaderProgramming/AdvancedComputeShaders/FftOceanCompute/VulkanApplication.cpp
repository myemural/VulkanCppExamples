/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <random>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
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
    // Creates the time-independent wave spectrum (h0) on the CPU
    std::vector<glm::vec4> CreateInitialSpectrum()
    {
        constexpr auto twoPi = glm::two_pi<float>();
        constexpr auto size = static_cast<int>(kFftSize);
        constexpr auto windLength = kWindSpeed * kWindSpeed / kGravity; // Largest possible wave for the wind speed

        // Phillips Spectrum: Energy of a single wave vector
        const auto phillips = [&](const glm::vec2& waveVector) {
            const auto kSquareWave = glm::dot(waveVector, waveVector);
            if (kSquareWave < 1.0e-12f) {
                return 0.0f;
            }

            const auto kLength = std::sqrt(kSquareWave);
            const auto directional = glm::dot(waveVector / kLength, kWindDirection);
            const auto kl = kLength * windLength;

            auto energy = kPhillipsAmplitude * std::exp(-1.0f / (kl * kl)) / (kSquareWave * kSquareWave) * directional *
                          directional;

            // Suppress very short waves and waves traveling against the wind
            energy *= std::exp(-kSquareWave * kSmallWaveSuppression * kSmallWaveSuppression);
            if (directional < 0.0f) {
                energy *= kOppositeWaveDamping;
            }

            return energy;
        };

        std::mt19937 rng{std::random_device{}()};
        std::normal_distribution gaussian{0.0f, 1.0f};

        // Gaussian complex wave amplitude per wave vector
        std::vector<glm::vec2> h0(kOceanVertexCount);
        for (auto z = 0; z < size; ++z) {
            for (auto x = 0; x < size; ++x) {
                const glm::vec2 waveVector{
                    twoPi * (static_cast<float>(x) - static_cast<float>(size) / 2.0f) / kOceanPatchLength,
                    twoPi * (static_cast<float>(z) - static_cast<float>(size) / 2.0f) / kOceanPatchLength};

                const auto amplitude = std::sqrt(phillips(waveVector) * 0.5f);
                h0[z * size + x] = glm::vec2{gaussian(rng), gaussian(rng)} * amplitude;
            }
        }

        // Pack h0[k] together with conj(h0[-k])
        std::vector<glm::vec4> initialSpectrum(kOceanVertexCount);
        for (auto z = 0; z < size; ++z) {
            const auto mirroredZ = (size - z) % size;
            for (auto x = 0; x < size; ++x) {
                const auto mirroredX = (size - x) % size;
                const auto mirrored = h0[mirroredZ * size + mirroredX];
                initialSpectrum[z * size + x] = glm::vec4{h0[z * size + x], mirrored.x, -mirrored.y};
            }
        }

        return initialSpectrum;
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
        UploadInitialSpectrum();
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

    // Advance the wave animation time
    if (!isSimulationPaused_) {
        simulationTime_ += static_cast<float>(deltaTime_) * kTimeScale;
    }

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
    constexpr auto spectrumBufferSize = static_cast<std::uint32_t>(sizeof(glm::vec4) * kOceanVertexCount);
    constexpr auto oceanVertexBufferSize = static_cast<std::uint32_t>(sizeof(OceanVertex) * kOceanVertexCount);

    resourceCreateInfo.buffers = {{kLightUniformBuffer, sizeof(LightUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kOceanUniformBuffer, sizeof(OceanUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kInitialSpectrumBuffer, spectrumBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kSpectrumBuffer, spectrumBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kOceanVertexBuffer, oceanVertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto oceanVertexShaderAsset = assetManager_->Load<ShaderAsset>(kOceanVertexShaderFile);
    const auto oceanFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kOceanFragmentShaderFile);
    const auto skyboxVertexShaderAsset = assetManager_->Load<ShaderAsset>(kSkyboxVertexShaderFile);
    const auto skyboxFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kSkyboxFragmentShaderFile);
    const auto spectrumComputeShaderAsset = assetManager_->Load<ShaderAsset>(kSpectrumComputeShaderFile);
    const auto fftComputeShaderAsset = assetManager_->Load<ShaderAsset>(kFftComputeShaderFile);
    const auto displacementComputeShaderAsset = assetManager_->Load<ShaderAsset>(kDisplacementComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kOceanVertexShaderKey, .asset = assetManager_->Get(oceanVertexShaderAsset)},
            {.name = kOceanFragmentShaderKey, .asset = assetManager_->Get(oceanFragmentShaderAsset)},
            {.name = kSkyboxVertexShaderKey, .asset = assetManager_->Get(skyboxVertexShaderAsset)},
            {.name = kSkyboxFragmentShaderKey, .asset = assetManager_->Get(skyboxFragmentShaderAsset)},
            {.name = kSpectrumComputeShaderKey, .asset = assetManager_->Get(spectrumComputeShaderAsset)},
            {.name = kFftComputeShaderKey, .asset = assetManager_->Get(fftComputeShaderAsset)},
            {.name = kDisplacementComputeShaderKey, .asset = assetManager_->Get(displacementComputeShaderAsset)}}};
    ;

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
        SamplerResourceCreateInfo{.name = kMainSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR}},
        SamplerResourceCreateInfo{.name = kSkyboxSampler,
                                  .filtering = {.magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR},
                                  .addressModes = {VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                   VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::UploadInitialSpectrum() const
{
    const auto initialSpectrum = CreateInitialSpectrum();
    const auto dataSize = initialSpectrum.size() * sizeof(glm::vec4);

    resources_->SetBuffer(kInitialSpectrumBuffer, initialSpectrum.data(), dataSize);
}

void VulkanApplication::BuildScene()
{
    SceneConfig sceneConfig;
    sceneConfig.attributeLayout = attributeLayouts;
    sceneConfig.enabledMaterialComponents = enabledMaterialComponents;
    sceneConfig.imageTransferCmdPool = cmdPool_;
    sceneConfig.imageTransferQueue = queue_;
    sceneConfig.primitiveStackCount = kOceanGridSize;
    sceneConfig.primitiveSectorCount = kOceanGridSize;

    scene_ = std::make_unique<Scene>(*resources_, sceneConfig);
    auto& sceneImageStorage = scene_->GetGpuImageStorage();

    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(kCameraStartPosition, aspectRatio, kCameraFov, kCameraNearPlane,
                                                  kCameraFarPlane);

    // Materials
    const auto waterNormalTextureAsset = assetManager_->Load<TextureAsset>(kWaterNormalTexturePath);
    const auto waterNormalTextureId = sceneImageStorage.StoreTexture(
            kWaterNormalTexture, kMainSampler, assetManager_->Get(waterNormalTextureAsset), VK_FORMAT_R8G8B8A8_UNORM);

    const auto cubemapRightTextureAsset = assetManager_->Load<TextureAsset>(kCubemapRightTexturePath);
    const auto cubemapLeftTextureAsset = assetManager_->Load<TextureAsset>(kCubemapLeftTexturePath);
    const auto cubemapTopTextureAsset = assetManager_->Load<TextureAsset>(kCubemapTopTexturePath);
    const auto cubemapBottomTextureAsset = assetManager_->Load<TextureAsset>(kCubemapBottomTexturePath);
    const auto cubemapBackTextureAsset = assetManager_->Load<TextureAsset>(kCubemapBackTexturePath);
    const auto cubemapFrontTextureAsset = assetManager_->Load<TextureAsset>(kCubemapFrontTexturePath);
    [[maybe_unused]] const auto cubemapTextureId = sceneImageStorage.StoreCubemapTexture(
            kCubemapTexture, kSkyboxSampler, assetManager_->Get(cubemapRightTextureAsset),
            assetManager_->Get(cubemapLeftTextureAsset), assetManager_->Get(cubemapTopTextureAsset),
            assetManager_->Get(cubemapBottomTextureAsset), assetManager_->Get(cubemapBackTextureAsset),
            assetManager_->Get(cubemapFrontTextureAsset));

    Material oceanMaterial;
    oceanMaterial.ambientStrength = 0.04f;
    oceanMaterial.specularStrength = 1.0f;
    oceanMaterial.shininess = 900.0f;
    oceanMaterial.uvScale = kOceanUvScale;
    oceanMaterial.normalMap = waterNormalTextureId;

    const auto rootObject = SceneObjectBuilder(*scene_, kRootObject)
                                    .WithPosition(glm::vec3{0.0f, 0.0f, 0.0f})
                                    .AddChild(SceneObjectBuilder(*scene_, kOceanObject)
                                                      .WithTag(kOceanObjectGroup)
                                                      .WithBuiltinMesh(BuiltinMeshType::PLANE)
                                                      .WithMaterial(oceanMaterial)
                                                      .WithPosition(glm::vec3(0.0f, kOceanLevel, 0.0f))
                                                      .WithScale(glm::vec3{kOceanPlaneSize}))
                                    .AddChild(SceneObjectBuilder(*scene_, kSkyboxCubeObject)
                                                      .WithTag(kSkyboxObjectGroup)
                                                      .WithBuiltinMesh(BuiltinMeshType::CUBE)
                                                      .WithMaterial(Material{})
                                                      .WithPosition(glm::vec3(0.0f, 0.0f, 0.0f)))
                                    .Build();

    scene_->AddRootObject(rootObject);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const auto combinedImageSamplerCount = scene_->GetGpuImageStorage().GetTextureCount();
    const auto cubemapCount = scene_->GetGpuImageStorage().GetCubemapTextureCount();

    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 2,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6},
                      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
                      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount + cubemapCount}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, combinedImageSamplerCount,
                                   VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                   nullptr},
                                  {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}},
                    {.name = kOceanComputeDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSet, .layoutName = kMainDescSetLayout},
                           {.name = kOceanComputeDescSet, .layoutName = kOceanComputeDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> storageTransformBufferInfos;
    storageTransformBufferInfos.emplace_back(scene_->GetTransformStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> storageMaterialBufferInfos;
    storageMaterialBufferInfos.emplace_back(scene_->GetMaterialStorageBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> lightUboInfos;
    lightUboInfos.emplace_back(resources_->GetBuffer(kLightUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> oceanUboInfos;
    oceanUboInfos.emplace_back(resources_->GetBuffer(kOceanUniformBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> initialSpectrumBufferInfos;
    initialSpectrumBufferInfos.emplace_back(resources_->GetBuffer(kInitialSpectrumBuffer)->GetHandle(), 0,
                                            VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> spectrumBufferInfos;
    spectrumBufferInfos.emplace_back(resources_->GetBuffer(kSpectrumBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> oceanVertexBufferInfos;
    oceanVertexBufferInfos.emplace_back(resources_->GetBuffer(kOceanVertexBuffer)->GetHandle(), 0, VK_WHOLE_SIZE);

    auto descriptorImageInfos = scene_->GetGpuImageStorage().GetDescriptorImageInfos();
    auto cubemapImageInfos = scene_->GetGpuImageStorage().GetCubemapDescriptorImageInfo(kCubemapTexture);

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

    ImageWriteRequest cubemapUpdateRequest;
    cubemapUpdateRequest.descriptorSetName = kMainDescSet;
    cubemapUpdateRequest.bindingIndex = 4;
    cubemapUpdateRequest.images = cubemapImageInfos;
    cubemapUpdateRequest.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest oceanVertexReadRequest;
    oceanVertexReadRequest.descriptorSetName = kMainDescSet;
    oceanVertexReadRequest.bindingIndex = 5;
    oceanVertexReadRequest.buffers = oceanVertexBufferInfos;
    oceanVertexReadRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeUboRequest;
    computeUboRequest.descriptorSetName = kOceanComputeDescSet;
    computeUboRequest.bindingIndex = 0;
    computeUboRequest.buffers = oceanUboInfos;
    computeUboRequest.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    BufferWriteRequest computeInitialSpectrumRequest;
    computeInitialSpectrumRequest.descriptorSetName = kOceanComputeDescSet;
    computeInitialSpectrumRequest.bindingIndex = 1;
    computeInitialSpectrumRequest.buffers = initialSpectrumBufferInfos;
    computeInitialSpectrumRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeSpectrumRequest;
    computeSpectrumRequest.descriptorSetName = kOceanComputeDescSet;
    computeSpectrumRequest.bindingIndex = 2;
    computeSpectrumRequest.buffers = spectrumBufferInfos;
    computeSpectrumRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeOceanVertexRequest;
    computeOceanVertexRequest.descriptorSetName = kOceanComputeDescSet;
    computeOceanVertexRequest.bindingIndex = 3;
    computeOceanVertexRequest.buffers = oceanVertexBufferInfos;
    computeOceanVertexRequest.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .bufferWriteRequests = {objectStorageTransformBufferRequest, objectStorageMaterialBufferRequest,
                                lightUboRequest, oceanVertexReadRequest, computeUboRequest,
                                computeInitialSpectrumRequest, computeSpectrumRequest, computeOceanVertexRequest},
        .imageWriteRequests = {textureUpdateRequest, cubemapUpdateRequest}};

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
            simulationTime_ = 0.0f;
            std::cout << "Ocean simulation restarted!" << std::endl;
        }

        // Pause/resume the simulation
        if (event.key == GLFW_KEY_SPACE) {
            isSimulationPaused_ = !isSimulationPaused_;
            std::cout << "Ocean simulation: " << (isSimulationPaused_ ? "PAUSED" : "RUNNING") << std::endl;
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
    VkPushConstantRange fftPushConstant;
    fftPushConstant.offset = 0;
    fftPushConstant.size = sizeof(FftPushConstants);
    fftPushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    oceanComputePipelineLayout_ = device_->CreatePipelineLayout(
            {resources_->GetDescriptorLayout(kOceanComputeDescSetLayout)}, {fftPushConstant});

    if (!oceanComputePipelineLayout_) {
        throw std::runtime_error("Failed to create ocean compute pipeline layout!");
    }

    spectrumPipeline_ = device_->CreateComputePipeline(oceanComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSpectrumComputeShaderKey)->GetHandle();
        });
    });

    if (!spectrumPipeline_) {
        throw std::runtime_error("Failed to create spectrum compute pipeline!");
    }

    fftPipeline_ = device_->CreateComputePipeline(oceanComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kFftComputeShaderKey)->GetHandle();
        });
    });

    if (!fftPipeline_) {
        throw std::runtime_error("Failed to create fft compute pipeline!");
    }

    displacementPipeline_ = device_->CreateComputePipeline(oceanComputePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kDisplacementComputeShaderKey)->GetHandle();
        });
    });

    if (!displacementPipeline_) {
        throw std::runtime_error("Failed to create displacement compute pipeline!");
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

    skyboxPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kSkyboxFragmentShaderKey)->GetHandle();
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
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        });
    });

    if (!skyboxPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for skybox)!");
    }

    oceanPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kOceanVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kOceanFragmentShaderKey)->GetHandle();
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

    if (!oceanPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for ocean objects)!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordOceanSimulation(const std::shared_ptr<VulkanCommandBuffer>& cmdBuffer) const
{
    const auto spectrumBuffer = resources_->GetBuffer(kSpectrumBuffer);
    const auto oceanVertexBuffer = resources_->GetBuffer(kOceanVertexBuffer);

    const std::vector descSets{resources_->GetDescriptorSet(kOceanComputeDescSet)};
    cmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, oceanComputePipelineLayout_, 0, descSets);

    const auto insertSpectrumBarrier = [&] {
        const std::vector barriers{spectrumBuffer->CreateBufferMemoryBarrier(
                VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)};
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
                                   barriers);
    };

    // The previous frame may still be reading these buffers, so writing them again has to be synchronized
    {
        const std::vector prevFrameBarriers{
            spectrumBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                                      VK_ACCESS_SHADER_WRITE_BIT),
            oceanVertexBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT)};
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {}, prevFrameBarriers);
    }

    // Spectrum Phase: h(k, t) from the pre-computed h0(k) plus the packed horizontal displacement spectrum
    cmdBuffer->BindPipeline(spectrumPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
    cmdBuffer->Dispatch(kOceanGroupCount, kOceanGroupCount, 1);
    insertSpectrumBarrier();

    // FFT Phase: 2D inverse FFT as two 1D passes (rows and columns) in-place
    cmdBuffer->BindPipeline(fftPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
    for (auto direction = 0U; direction < 2U; ++direction) {
        const FftPushConstants pushConstants{direction};
        cmdBuffer->PushConstants(oceanComputePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants),
                                 &pushConstants);
        cmdBuffer->Dispatch(kFftSize, 1, 1); // One work group per row/column
        insertSpectrumBarrier();
    }

    // Displacement Phase: FFT shift, normals from differences and foam from the folding Jacobian
    cmdBuffer->BindPipeline(displacementPipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);
    cmdBuffer->Dispatch(kOceanGroupCount, kOceanGroupCount, 1);
    insertSpectrumBarrier();

    // From compute shader writes to vertex shader reads
    const std::vector vertexReadBarriers{
        oceanVertexBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)};
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

    // Compute Phase: FFT ocean simulation
    if (!isSimulationPaused_) {
        RecordOceanSimulation(currentCmdBuffer);
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

    // Render Pass: Draw skybox
    currentCmdBuffer->BindPipeline(skyboxPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() == kSkyboxObjectGroup) {
            drawSceneObject(sceneObject);
        }
    });

    // Render Pass: Draw displaced ocean surface
    currentCmdBuffer->BindPipeline(oceanPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    scene_->Traverse([&](const SceneObject& sceneObject) {
        if (sceneObject.HasRenderable() && sceneObject.GetTag() == kOceanObjectGroup) {
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
    lightUbo.sceneParams = glm::vec4(simulationTime_, kFoamIntensity, kFogStartDistance, kFogEndDistance);
    resources_->SetBuffer(kLightUniformBuffer, &lightUbo, sizeof(lightUbo));

    OceanUbo oceanUbo{};
    oceanUbo.patchParams = glm::vec4(kOceanPatchLength, kGravity, simulationTime_, kChoppiness);
    oceanUbo.waveParams = glm::vec4(kAmplitudeScale, kFoamThreshold, kFoamSharpness, 0.0f);
    oceanUbo.gridInfo = glm::uvec4(kFftSize, 0U, 0U, 0U);
    resources_->SetBuffer(kOceanUniformBuffer, &oceanUbo, sizeof(oceanUbo));
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
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fft_ocean_compute
