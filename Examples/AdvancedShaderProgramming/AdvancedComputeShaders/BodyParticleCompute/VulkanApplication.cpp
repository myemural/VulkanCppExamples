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
#include "MathUtils.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "TimeUtils.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
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
    std::vector<Particle> CreateGalaxyCollisionParticles()
    {
        std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution uniformDist{0.0f, 1.0f};
        std::normal_distribution normalDist{0.0f, 1.0f};

        constexpr std::array centers{glm::vec3{-kGalaxySeparation * 0.5f, 0.0f, 0.0f},
                                     glm::vec3{kGalaxySeparation * 0.5f, 0.0f, 6.0f}};

        constexpr std::array bulkVelocities{glm::vec3{kApproachSpeed, 0.0f, 0.0f},
                                            glm::vec3{-kApproachSpeed, 0.0f, 0.0f}};

        const std::array normals{glm::normalize(glm::vec3{0.15f, 1.0f, 0.10f}),
                                 glm::normalize(glm::vec3{-0.35f, 1.0f, -0.25f})};

        std::vector<Particle> particles;
        particles.reserve(kParticleCount);

        for (auto galaxy = 0U; galaxy < kGalaxyCount; ++galaxy) {
            const auto normal = normals.at(galaxy);

            // Orthonormal basis of the disc plane
            const auto helper = std::abs(normal.y) < 0.9f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            const auto tangentU = glm::normalize(glm::cross(helper, normal));
            const auto tangentV = glm::cross(normal, tangentU);

            // Galactic core: A single heavy body that holds the disc together
            particles.push_back(Particle{glm::vec4{centers[galaxy], kCoreMass},
                                         glm::vec4{bulkVelocities[galaxy], static_cast<float>(galaxy)}});

            for (auto i = 1U; i < kParticlesPerGalaxy; ++i) {
                const auto radius = kGalaxyRadius * std::sqrt(uniformDist(rng)) * 0.5f;
                const auto angle = uniformDist(rng) * glm::two_pi<float>();

                const auto radial = tangentU * std::cos(angle) + tangentV * std::sin(angle);
                const auto position = centers[galaxy] + radial * radius + normal * (normalDist(rng) * kGalaxyThickness);

                const auto enclosedMass = kCoreMass + static_cast<float>(kParticlesPerGalaxy) * kParticleMass * 4.0f *
                                          (radius * radius) / (kGalaxyRadius * kGalaxyRadius);
                const auto orbitalSpeed = std::sqrt(kGravity * enclosedMass / radius);
                const auto tangential = glm::normalize(glm::cross(normal, radial));

                particles.push_back(Particle{
                    glm::vec4{position, kParticleMass},
                    glm::vec4{bulkVelocities[galaxy] + tangential * orbitalSpeed, static_cast<float>(galaxy)}});
            }
        }

        return particles;
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
        CreateAndUpdateDescriptorSets();

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

    UpdateCamera();
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

void VulkanApplication::CreateInitialResources()
{
    // Add camera
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, kCameraHeight, kCameraDistance), aspectRatio, 45.0f,
                                                  0.1f, 500.0f);

    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    constexpr auto particleBufferSize = static_cast<std::uint32_t>(kParticleCount * sizeof(Particle));
    resourceCreateInfo.buffers = {{kParticleBufferA, particleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
                                  {kParticleBufferB, particleBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);
    const auto nBodyComputeShaderAsset = assetManager_->Load<ShaderAsset>(kNBodyComputeShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)},
                    {.name = kNBodyComputeShaderKey, .asset = assetManager_->Get(nBodyComputeShaderAsset)}}};

    CreateVulkanResources(resourceCreateInfo);

    // Seed both buffers with the same initial galaxy state
    const auto initialParticles = CreateGalaxyCollisionParticles();
    resources_->SetBuffer(kParticleBufferA, initialParticles.data(), particleBufferSize);
    resources_->SetBuffer(kParticleBufferB, initialParticles.data(), particleBufferSize);
}

void VulkanApplication::CreateAndUpdateDescriptorSets() const
{
    // Create descriptor sets
    const DescriptorResourceCreateInfo descriptorResourceCreateInfo = {
        .maxSets = 4,
        .poolSizes = {{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6}},
        .layouts = {{.name = kMainDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}}},
                    {.name = kComputeDescSetLayout,
                     .bindings = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}}}},
        .descriptorSets = {{.name = kMainDescSetA, .layoutName = kMainDescSetLayout},
                           {.name = kMainDescSetB, .layoutName = kMainDescSetLayout},
                           {.name = kComputeDescSetAToB, .layoutName = kComputeDescSetLayout},
                           {.name = kComputeDescSetBToA, .layoutName = kComputeDescSetLayout}}};

    resources_->CreateDescriptorSets(descriptorResourceCreateInfo);

    std::vector<VkDescriptorBufferInfo> particleABufferInfos;
    particleABufferInfos.emplace_back(resources_->GetBuffer(kParticleBufferA)->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> particleBBufferInfos;
    particleBBufferInfos.emplace_back(resources_->GetBuffer(kParticleBufferB)->GetHandle(), 0, VK_WHOLE_SIZE);

    BufferWriteRequest renderReadA;
    renderReadA.descriptorSetName = kMainDescSetA;
    renderReadA.bindingIndex = 0;
    renderReadA.buffers = particleABufferInfos;
    renderReadA.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest renderReadB;
    renderReadB.descriptorSetName = kMainDescSetB;
    renderReadB.bindingIndex = 0;
    renderReadB.buffers = particleBBufferInfos;
    renderReadB.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBPrev;
    computeAToBPrev.descriptorSetName = kComputeDescSetAToB;
    computeAToBPrev.bindingIndex = 0;
    computeAToBPrev.buffers = particleABufferInfos;
    computeAToBPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeAToBNext;
    computeAToBNext.descriptorSetName = kComputeDescSetAToB;
    computeAToBNext.bindingIndex = 1;
    computeAToBNext.buffers = particleBBufferInfos;
    computeAToBNext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToAPrev;
    computeBToAPrev.descriptorSetName = kComputeDescSetBToA;
    computeBToAPrev.bindingIndex = 0;
    computeBToAPrev.buffers = particleBBufferInfos;
    computeBToAPrev.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    BufferWriteRequest computeBToANext;
    computeBToANext.descriptorSetName = kComputeDescSetBToA;
    computeBToANext.bindingIndex = 1;
    computeBToANext.buffers = particleABufferInfos;
    computeBToANext.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {.bufferWriteRequests = {renderReadA, renderReadB,
                                                                                  computeAToBPrev, computeAToBNext,
                                                                                  computeBToAPrev, computeBToANext}};

    resources_->UpdateDescriptorSet(descriptorSetUpdateInfo);
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
    VkPushConstantRange nBodyPushConstants;
    nBodyPushConstants.offset = 0;
    nBodyPushConstants.size = sizeof(NBodyPushConstants);
    nBodyPushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computePipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kComputeDescSetLayout)},
                                                           {nBodyPushConstants});

    if (!computePipelineLayout_) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    computePipeline_ = device_->CreateComputePipeline(computePipelineLayout_, [&](auto& builder) {
        builder.SetShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kNBodyComputeShaderKey)->GetHandle();
        });
    });

    if (!computePipeline_) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    VkPushConstantRange particlePushConstants;
    particlePushConstants.offset = 0;
    particlePushConstants.size = sizeof(ParticleRenderPushConstants);
    particlePushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    particlePipelineLayout_ = device_->CreatePipelineLayout({resources_->GetDescriptorLayout(kMainDescSetLayout)},
                                                            {particlePushConstants});

    if (!particlePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    particlePipeline_ = device_->CreateGraphicsPipeline(particlePipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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
        builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
        });
    });

    if (!particlePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for particle sprites)!");
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

    // Ping-pong: Even frames read from A and write to B, odd frames read from B write to A
    const bool isAtoB = frameCounter_ % 2 == 0;
    const auto particleBufferA = resources_->GetBuffer(kParticleBufferA);
    const auto particleBufferB = resources_->GetBuffer(kParticleBufferB);
    const auto writtenBuffer = isAtoB ? particleBufferB : particleBufferA;

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Change buffers from READ to READ/WRITE
    {
        const std::vector preComputeBarriers{
            particleBufferA->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_READ_BIT,
                                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT),
            particleBufferB->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_READ_BIT,
                                                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)};
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, {},
                                          preComputeBarriers);
    }

    // Compute Phase: Integrate the N-body system for one step
    {
        currentCmdBuffer->BindPipeline(computePipeline_, VK_PIPELINE_BIND_POINT_COMPUTE);


        const std::vector descSets{resources_->GetDescriptorSet(isAtoB ? kComputeDescSetAToB : kComputeDescSetBToA)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout_, 0, descSets);

        NBodyPushConstants pushConstants{};
        pushConstants.deltaTime = std::min(static_cast<float>(deltaTime_), kMaxDeltaTime) * kTimeScale;
        pushConstants.gravity = kGravity;
        pushConstants.softening = kSoftening;
        pushConstants.damping = kDamping;
        pushConstants.particleCount = kParticleCount;

        currentCmdBuffer->PushConstants(computePipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                        sizeof(NBodyPushConstants), &pushConstants);

        const auto groupCountX = CeilDiv(kParticleCount, kLocalSizeX);
        currentCmdBuffer->Dispatch(groupCountX, 1, 1);

        ++frameCounter_;
    }

    // Render Phase: Draw every particle as an additive billboard sprite
    {
        // Change currently written buffer from WRITE to READ
        const std::vector postComputeBarriers{
            writtenBuffer->CreateBufferMemoryBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)};
        currentCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, {},
                                          postComputeBarriers);

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
        currentCmdBuffer->BindPipeline(particlePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        const std::vector descSets{resources_->GetDescriptorSet(isAtoB ? kMainDescSetB : kMainDescSetA)};
        currentCmdBuffer->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, particlePipelineLayout_, 0, descSets);

        const auto view = camera_->GetViewMatrix();
        const auto proj = camera_->GetProjectionMatrix();

        ParticleRenderPushConstants pushConstants{};
        pushConstants.viewProjMatrix = proj * view;
        pushConstants.projScale = glm::vec2(proj[0][0], proj[1][1]);
        pushConstants.particleRadius = kParticleRadius;
        pushConstants.brightness = kBrightness;

        currentCmdBuffer->PushConstants(particlePipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                        sizeof(ParticleRenderPushConstants), &pushConstants);

        // Draw fullscreen quad
        currentCmdBuffer->Draw(6, kParticleCount, 0, 0);

        currentCmdBuffer->EndRenderPass();
    }

    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateCamera() const
{
    // Slow orbit around the origin, always looking at the center of mass
    const auto angle = glm::radians(static_cast<float>(GetCurrentTime()) * kCameraOrbitSpeed);
    const glm::vec3 position{sin(angle) * kCameraDistance, kCameraHeight, cos(angle) * kCameraDistance};
    camera_->SetPosition(position);

    const auto forward = glm::normalize(-position);
    camera_->SetRotation(glm::degrees(std::atan2(forward.z, forward.x)), glm::degrees(std::asin(forward.y)));
}
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
