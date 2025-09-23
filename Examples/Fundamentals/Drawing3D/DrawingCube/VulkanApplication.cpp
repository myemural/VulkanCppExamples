/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"
#include "AppConfig.h"

namespace examples::fundamentals::drawing_3d::drawing_cube
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer &&params)
    : ApplicationDrawing3D(std::move(params))
{
    currentWindowWidth_ = GetParamU32(WindowParams::Width);
    currentWindowHeight_ = GetParamU32(WindowParams::Height);
}

bool VulkanApplication::Init()
{
    try {
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

        SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), vertices.data(),
                  vertices.size() * sizeof(VertexPos3Uv2));
        SetBuffer(GetParamStr(AppConstants::MainIndexBuffer), indices.data(),
                  indices.size() * sizeof(indices[0]));
        SetBuffer(GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.Data,
                  crateTextureHandler_.GetByteSize());
        SetBuffer(GetParamStr(AppConstants::MvpUniformBuffer), &mvpUbObject, sizeof(MvpData));
        SetImageFromBuffer(GetParamStr(AppConstants::CrateImage),
                           buffers_[GetParamStr(AppConstants::ImageStagingBuffer)]->GetBuffer(), {
                               crateTextureHandler_.Width, crateTextureHandler_.Height, 1
                           });

        CreateRenderPass(depthFormat);
        CreatePipeline();
        CreateDefaultFramebuffers(
            images_[GetParamStr(AppConstants::DepthImage)]->GetImageView(
                GetParamStr(AppConstants::DepthImageView)));
        CreateDefaultSyncObjects(GetParamU32(AppConstants::MaxFramesInFlight));
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

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::Cleanup() noexcept
{
    ApplicationDrawing3D::Cleanup();
    crateTextureHandler_.Clear();
}

void VulkanApplication::InitResources(const VkFormat &depthImageFormat)
{
    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    crateTextureHandler_ = textureLoader.Load(GetParamStr(AppConstants::CrateTexturePath));

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos3Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);
    bufferCreateInfos_ = {
        {
            GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::MainIndexBuffer), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::ImageStagingBuffer), crateTextureHandler_.GetByteSize(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::MvpUniformBuffer), sizeof(MvpData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }
    };

    // Fill shader module create infos
    shaderModuleCreateInfo_ = {
        .BasePath = SHADERS_DIR,
        .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
        .Modules = {
            {
                .Name = GetParamStr(AppConstants::MainVertexShaderKey),
                .FileName = GetParamStr(AppConstants::MainVertexShaderFile)
            },
            {
                .Name = GetParamStr(AppConstants::MainFragmentShaderKey),
                .FileName = GetParamStr(AppConstants::MainFragmentShaderFile)
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
                .Name = GetParamStr(AppConstants::MainDescSetLayout),
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
            .Name = GetParamStr(AppConstants::CrateImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = VK_FORMAT_R8G8B8A8_SRGB,
            .Dimensions = {crateTextureHandler_.Width, crateTextureHandler_.Height, 1},
            .Views = {
                ImageViewCreateInfo{
                    .ViewName = GetParamStr(AppConstants::CrateImageView),
                    .Format = VK_FORMAT_R8G8B8A8_SRGB
                }
            }
        },
        ImageResourceCreateInfo{
            .Name = GetParamStr(AppConstants::DepthImage),
            .MemProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .Format = depthImageFormat,
            .Dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
            .UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .Views = {
                ImageViewCreateInfo{
                    .ViewName = GetParamStr(AppConstants::DepthImageView),
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

    samplerResourceCreateInfos_ = {
        {
            .Name = GetParamStr(AppConstants::MainSampler),
            .FilteringBehavior = {
                .MagFilter = VK_FILTER_LINEAR,
                .MinFilter = VK_FILTER_LINEAR
            }
        }
    };

    CreateBuffers(bufferCreateInfos_);
    CreateImages(imageResourceCreateInfos_);
    CreateSamplers(samplerResourceCreateInfos_);
    CreateShaderModules(shaderModuleCreateInfo_);
    CreateDescriptorSets(descriptorSetCreateInfo_);
    UpdateDescriptorSets();
}

void VulkanApplication::CreateRenderPass(const VkFormat &depthImageFormat)
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
    pipelineLayout_ = device_->CreatePipelineLayout({
        descriptorRegistry_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))
    });

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0, 0, static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_), 0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

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
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))->
                    GetHandle();
        });
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(GetParamStr(AppConstants::MainFragmentShaderKey))
                    ->GetHandle();
        });
        builder.SetVertexInputState([&](auto &vertexInputStateCreateInfo) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        });
        builder.SetViewportState([&](auto &viewportStateCreateInfo) {
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
    imageSamplerInfos.emplace_back(
        samplers_[GetParamStr(AppConstants::MainSampler)]->GetSampler()->GetHandle(),
        images_[GetParamStr(AppConstants::CrateImage)]->GetImageView(
            GetParamStr(AppConstants::CrateImageView))->GetHandle(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.emplace_back(
        buffers_[GetParamStr(AppConstants::MvpUniformBuffer)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.LayoutName = GetParamStr(AppConstants::MainDescSetLayout);
    samplerUpdateRequest.BindingIndex = 0;
    samplerUpdateRequest.Images = imageSamplerInfos;
    samplerUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    BufferWriteRequest ubUpdateRequest;
    ubUpdateRequest.LayoutName = GetParamStr(AppConstants::MainDescSetLayout);
    ubUpdateRequest.BindingIndex = 1;
    ubUpdateRequest.Buffers = bufferInfos;
    ubUpdateRequest.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    descriptorSetUpdateInfo_ = {
        .BufferWriteRequests = {ubUpdateRequest},
        .ImageWriteRequests = {samplerUpdateRequest}
    };

    UpdateDescriptorSet(descriptorSetUpdateInfo_);
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
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
        clearValues[1].depthStencil = {1.0f, 0};
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass([&](auto &beginInfo) {
            beginInfo.renderPass = renderPass_->GetHandle();
            beginInfo.framebuffer = framebuffers_[i]->GetHandle();
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
            beginInfo.clearValueCount = clearValues.size();
            beginInfo.pClearValues = clearValues.data();
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                                                  {
                                                      descriptorRegistry_->GetDescriptorSet(
                                                          GetParamStr(AppConstants::MainDescSetLayout))
                                                  });
        cmdBuffersPresent_[i]->BindVertexBuffers({
                                                     buffers_[GetParamStr(AppConstants::MainVertexBuffer)]
                                                     ->GetBuffer()
                                                 }, 0, 1, {0});
        cmdBuffersPresent_[i]->BindIndexBuffer(
            buffers_[GetParamStr(AppConstants::MainIndexBuffer)]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
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
        glm::vec3(2.0f, 2.0f, 2.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Look at position
        glm::vec3(0.0f, 0.0f, 1.0f) // Up vector
    );
    mvpUbObject.view = view;

    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f), // FOV
        aspectRatio, // Aspect ratio
        0.1f, // Near clipping-plane
        10.0f // Far clipping plane
    );
    proj[1][1] *= -1; // Vulkan trick for projection
    mvpUbObject.projection = proj;

    SetBuffer(GetParamStr(AppConstants::MvpUniformBuffer), &mvpUbObject, sizeof(MvpData));
}
} // namespace examples::fundamentals::drawing_3d::drawing_cube
