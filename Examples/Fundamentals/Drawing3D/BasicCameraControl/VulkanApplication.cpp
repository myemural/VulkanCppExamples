/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::drawing_3d::basic_camera_control
{

using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

/// TODO: Will be move into utils
float GetCurrentTime()
{
    using clock = std::chrono::steady_clock;
    const auto now = clock::now().time_since_epoch();
    return std::chrono::duration<float>(now).count();
}

VulkanApplication::VulkanApplication(const ApplicationCreateConfig &config, const ApplicationSettings& settings)
    : ApplicationDrawing3D(config), settings_(settings)
{
}

bool VulkanApplication::Init()
{
    try {
        InitInputSystem();

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();

        const auto depthFormat = physicalDevice_->FindSupportedFormat({
                                                                          VK_FORMAT_D32_SFLOAT,
                                                                          VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                                          VK_FORMAT_D24_UNORM_S8_UINT
                                                                      },
                                                                      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);


        InitResources(depthFormat);

        SetBuffer(kVertexBufferKey, vertices.data(), vertices.size() * sizeof(VertexPos3Uv2));
        SetBuffer(kIndexBufferKey, indices.data(), indices.size() * sizeof(indices[0]));
        SetBuffer(kTextureStagingBufferKey, crateTextureHandler_.Data, crateTextureHandler_.GetByteSize());
        SetBuffer(kMvpUniformBufferKey, &mvpUbObject, sizeof(MvpData));
        SetImageFromBuffer(kCrateImageKey, buffers_[kTextureStagingBufferKey]->GetBuffer(), {
                               crateTextureHandler_.Width, crateTextureHandler_.Height, 1
                           });

        CreateRenderPass(depthFormat);
        CreatePipeline();
        CreateDefaultFramebuffers(images_[kDepthImageKey]->GetImageView(kDepthImageViewKey));
        CreateDefaultSyncObjects(kMaxFramesInFlight);
        CreateCommandBuffers();

        const uint32_t indexCount = indices.size();
        RecordPresentCommandBuffers(indexCount);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void VulkanApplication::DrawFrame()
{
    inFlightFences_[currentIndex_]->WaitForFence(true, UINT64_MAX);
    inFlightFences_[currentIndex_]->ResetFence();

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    CalculateAndSetMvp();

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[currentIndex_]}, inFlightFences_[currentIndex_], {
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                   });

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[currentIndex_]});

    currentIndex_ = (currentIndex_ + 1) % kMaxFramesInFlight;
}

void VulkanApplication::Update()
{
    ProcessInput();
    ApplicationDrawing3D::Update();
    const float currentFrame = GetCurrentTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;
}

void VulkanApplication::Cleanup()
{
    ApplicationDrawing3D::Cleanup();
    crateTextureHandler_.Clear();
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = window_->GetWindowWidth() / 2.0f;
    lastY_ = window_->GetWindowHeight() / 2.0f;

    window_->DisableCursor();

    window_->SetKeyCallback([&](const int key, [[maybe_unused]] const int scancode, const int action,
                                [[maybe_unused]] const int mods) {
        if (action == GLFW_PRESS) {
            keys_[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys_[key] = false;
        }
    });


    window_->SetMouseCallback([&](const double x, const double y) {
        const float xPos = static_cast<float>(x);
        const float yPos = static_cast<float>(y);

        if (firstMouseTriggered_)
        {
            lastX_ = xPos;
            lastY_ = yPos;
            firstMouseTriggered_ = false;
        }

        float xOffset = xPos - lastX_;
        float yOffset = lastY_ - yPos;
        lastX_ = xPos;
        lastY_ = yPos;

        const float sensitivity = settings_.MouseSensitivity * deltaTime_;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yawAngle_ += xOffset;
        pitchAngle_ += yOffset;

        pitchAngle_ = std::clamp(pitchAngle_, -89.0f, 89.0f);

        const float yawRad   = glm::radians(yawAngle_);
        const float pitchRad = glm::radians(pitchAngle_);

        const glm::vec3 front{
            std::cos(yawRad) * std::cos(pitchRad),
            std::sin(pitchRad),
            std::sin(yawRad) * std::cos(pitchRad)
        };
        cameraFront_ = glm::normalize(front);
    });
}

void VulkanApplication::InitResources(const VkFormat &depthImageFormat)
{
    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    crateTextureHandler_ = textureLoader.Load(kTextureCratePath);

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);
    bufferCreateInfos_ = {
        {
            kVertexBufferKey, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            kIndexBufferKey, indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            kTextureStagingBufferKey, crateTextureHandler_.GetByteSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            kMvpUniformBufferKey, sizeof(MvpData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }
    };

    // Fill shader module create infos
    shaderModuleCreateInfo_ = {
        .BasePath = SHADERS_DIR,
        .ShaderType = kCurrentShaderType,
        .Modules = {
            {
                .Name = kVertexShaderHash,
                .FileName = kVertexShaderFileName
            },
            {
                .Name = kFragmentShaderHash,
                .FileName = kFragmentShaderFileName
            }
        }
    };

    // Fill descriptor set create infos
    descriptorSetCreateInfo_ = {
        .MaxSets = 1,
        .PoolSizes = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
        },
        .Layouts = {
            {
                .Name = kMainDescSetLayoutKey,
                .Bindings = {
                    {
                        0,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        1,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        nullptr
                    },
                    {
                        1,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        1,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        nullptr
                    }
                }
            }
        }
    };

    imageResourceCreateInfos_ = {
        ImageResourceCreateInfo{
            .Name = kCrateImageKey,
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = VK_FORMAT_R8G8B8A8_SRGB,
            .Dimensions = {crateTextureHandler_.Width, crateTextureHandler_.Height, 1},
            .Views = {
                ImageViewCreateInfo{
                    .ViewName = kCrateImageViewKey,
                    .Format = VK_FORMAT_R8G8B8A8_SRGB
                }
            }
        },
        ImageResourceCreateInfo{
            .Name = kDepthImageKey,
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = depthImageFormat,
            .Dimensions = {window_->GetWindowWidth(), window_->GetWindowHeight(), 1},
            .UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .Views = {
                ImageViewCreateInfo{
                    .ViewName = kDepthImageViewKey,
                    .Format = depthImageFormat,
                    .SubresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    }
                }
            }
        }
    };

    CreateBuffers(bufferCreateInfos_);
    CreateImages(imageResourceCreateInfos_);
    CreateSampler();
    CreateShaderModules(shaderModuleCreateInfo_);
    CreateDescriptorSets(descriptorSetCreateInfo_);
    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPass(const VkFormat& depthImageFormat)
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto &builder) {
        builder.AddAttachment([](auto &attachmentCreateInfo) {
                    attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                })
                .AddAttachment([&depthImageFormat](auto &attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                })
                .AddSubpass([&](auto &subpassCreateInfo) {
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
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    pipelineLayout_ = device_->CreatePipelineLayout({descriptorRegistry_->GetDescriptorLayout(kMainDescSetLayoutKey)});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 1.0f};
    VkRect2D scissor{0, 0, windowWidth, windowHeight};

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos3Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos3Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{
        posAttribDescription,
        uvAttribDescription
    };

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto &builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderModules_[kVertexShaderHash]->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderModules_[kFragmentShaderHash]->GetHandle();
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
        builder.SetColorBlendState([&](auto &blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
        builder.SetDepthStencilState([&](auto &depthStencilStateCreateInfo) {
            depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        });
    });

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets()
{
    std::vector<VkDescriptorImageInfo> imageSamplerInfos;
    imageSamplerInfos.emplace_back(sampler_->GetHandle(),
                                   images_[kCrateImageKey]->GetImageView(kCrateImageViewKey)->GetHandle(),
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.emplace_back(buffers_[kMvpUniformBufferKey]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.LayoutName = kMainDescSetLayoutKey;
    samplerUpdateRequest.BindingIndex = 0;
    samplerUpdateRequest.Images = imageSamplerInfos;
    samplerUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest ubUpdateRequest;
    ubUpdateRequest.LayoutName = kMainDescSetLayoutKey;
    ubUpdateRequest.BindingIndex = 1;
    ubUpdateRequest.Buffers = bufferInfos;
    ubUpdateRequest.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    descriptorSetUpdateInfo_ = {
        .BufferWriteRequests = {ubUpdateRequest},
        .ImageWriteRequests = {samplerUpdateRequest}
    };

    UpdateDescriptorSet(descriptorSetUpdateInfo_);
}

void VulkanApplication::CreateSampler()
{
    sampler_ = device_->CreateSampler([](auto& builder) {
       builder.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
    });

    if (!sampler_) {
        throw std::runtime_error("Failed to create sampler!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }

    cmdBufferTransfer_ = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferTransfer_) {
        throw std::runtime_error("Failed to create command buffer for transfer!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t indexCount)
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = settings_.ClearColor;
        clearValues[1].depthStencil = {1.0f, 0};
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass([&](auto &beginInfo) {
            beginInfo.renderPass = renderPass_->GetHandle();
            beginInfo.framebuffer = framebuffers_[i]->GetHandle();
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = VkExtent2D(windowWidth, windowHeight);
            beginInfo.clearValueCount = clearValues.size();
            beginInfo.pClearValues = clearValues.data();
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                                                  {descriptorRegistry_->GetDescriptorSet(kMainDescSetLayoutKey)});
        cmdBuffersPresent_[i]->BindVertexBuffers({buffers_[kVertexBufferKey]->GetBuffer()}, 0, 1, {0});
        cmdBuffersPresent_[i]->BindIndexBuffer(buffers_[kIndexBufferKey]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
        cmdBuffersPresent_[i]->DrawIndexed(indexCount, 1, 0, 0, 0);
        cmdBuffersPresent_[i]->EndRenderPass();
        if (!cmdBuffersPresent_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::CalculateAndSetMvp()
{
    const auto currentTime = static_cast<float>(glfwGetTime());
    auto model = glm::mat4(1.0f);
    model = glm::rotate(model, currentTime * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, currentTime * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    mvpUbObject.model = model;

    const glm::mat4 view = glm::lookAt(
        cameraPos_,
        cameraPos_ + cameraFront_,
        cameraUp_
    );
    mvpUbObject.view = view;

    const float aspectRatio = static_cast<float>(window_->GetWindowWidth()) / static_cast<float>(window_->
                                  GetWindowHeight());
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f), // FOV
        aspectRatio, // Aspect ratio
        0.1f, // Near clipping-plane
        10.0f // Far clipping plane
    );
    proj[1][1] *= -1; // Vulkan trick for projection
    mvpUbObject.projection = proj;

    SetBuffer(kMvpUniformBufferKey, &mvpUbObject, sizeof(MvpData));
}

void VulkanApplication::ProcessInput()
{
    const float cameraSpeed = settings_.CameraSpeed * deltaTime_;
    if (keys_[GLFW_KEY_W]) {
        cameraPos_ += cameraSpeed * cameraFront_;
    }
    if (keys_[GLFW_KEY_S]) {
        cameraPos_ -= cameraSpeed * cameraFront_;
    }
    if (keys_[GLFW_KEY_A]) {
        cameraPos_ -= glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
    if (keys_[GLFW_KEY_D]) {
        cameraPos_ += glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
}
} // namespace examples::fundamentals::drawing_3d::basic_camera_control