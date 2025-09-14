/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "AppConfig.h"

namespace examples::fundamentals::images_and_samplers::combined_image_sampler
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer &&params)
    : ApplicationImagesAndSamplers(std::move(params))
{
    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    bricksTextureHandler_ = textureLoader.Load(params_.Get<std::string>(AppConstants::BricksTexturePath));

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);
    bufferCreateInfos_ = {
        {
            params_.Get<std::string>(AppConstants::MainVertexBuffer), vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            params_.Get<std::string>(AppConstants::MainIndexBuffer), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            params_.Get<std::string>(AppConstants::ImageStagingBuffer), bricksTextureHandler_.GetByteSize(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }
    };

    // Fill shader module create infos
    shaderModuleCreateInfo_ = {
        .BasePath = SHADERS_DIR,
        .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
        .Modules = {
            {
                .Name = params_.Get<std::string>(AppConstants::MainVertexShaderKey),
                .FileName = params_.Get<std::string>(AppConstants::MainVertexShaderFile)
            },
            {
                .Name = params_.Get<std::string>(AppConstants::MainFragmentShaderKey),
                .FileName = params_.Get<std::string>(AppConstants::MainFragmentShaderFile)
            }
        }
    };

    // Fill descriptor set create infos
    descriptorSetCreateInfo_ = {
        .MaxSets = 1,
        .PoolSizes = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
        },
        .Layouts = {
            {
                .Name = params_.Get<std::string>(AppConstants::MainDescSetLayout),
                .Bindings = {
                    {
                        0,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        1,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        nullptr
                    }
                }
            }
        }
    };
}

bool VulkanApplication::Init()
{
    try {
        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();

        CreateQuadTextureImage();
        CreateQuadTextureImageView();
        CreateSampler();

        CreateBuffers(bufferCreateInfos_);
        SetBuffer(params_.Get<std::string>(AppConstants::MainVertexBuffer), vertices.data(),
                  vertices.size() * sizeof(VertexPos2Uv2));
        SetBuffer(params_.Get<std::string>(AppConstants::MainIndexBuffer), indices.data(),
                  indices.size() * sizeof(indices[0]));
        SetBuffer(params_.Get<std::string>(AppConstants::ImageStagingBuffer), bricksTextureHandler_.Data,
                  bricksTextureHandler_.GetByteSize());

        CreateDefaultRenderPass();
        CreateShaderModules(shaderModuleCreateInfo_);
        CreateDescriptorSets(descriptorSetCreateInfo_);
        UpdateDescriptorSets();
        CreatePipeline();
        CreateDefaultFramebuffers();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(params_.Get<std::uint32_t>(AppConstants::MaxFramesInFlight));
        CreateCommandBuffers();

        const uint32_t indexCount = indices.size();
        RecordPresentCommandBuffers(indexCount);
        ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyStagingBuffer();
        ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[currentIndex_]}, inFlightFences_[currentIndex_], {
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                   });

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[currentIndex_]});

    currentIndex_ = (currentIndex_ + 1) % params_.Get<std::uint32_t>(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::Cleanup()
{
    ApplicationImagesAndSamplers::Cleanup();
    bricksTextureHandler_.Clear();
}

void VulkanApplication::CreatePipeline()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    pipelineLayout_ = device_->CreatePipelineLayout({
        descriptorRegistry_->GetDescriptorLayout(params_.Get<std::string>(AppConstants::MainDescSetLayout))
    });

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
    auto bindingDescription = GenerateBindingDescription<VertexPos2Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{
        posAttribDescription,
        uvAttribDescription
    };

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto &builder) {
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderModules_[params_.Get<std::string>(AppConstants::MainVertexShaderKey)]->
                    GetHandle();
        });
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderModules_[params_.Get<std::string>(AppConstants::MainFragmentShaderKey)]
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
    });

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets()
{
    std::vector<VkDescriptorImageInfo> imageSamplerInfos;
    imageSamplerInfos.emplace_back(sampler_->GetHandle(), quadTextureImageView_->GetHandle(),
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ImageWriteRequest samplerUpdateRequest;
    samplerUpdateRequest.LayoutName = params_.Get<std::string>(AppConstants::MainDescSetLayout);
    samplerUpdateRequest.BindingIndex = 0;
    samplerUpdateRequest.Images = imageSamplerInfos;
    samplerUpdateRequest.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    descriptorSetUpdateInfo_ = {
        .ImageWriteRequests = {samplerUpdateRequest}
    };

    UpdateDescriptorSet(descriptorSetUpdateInfo_);
}

void VulkanApplication::CreateQuadTextureImage()
{
    quadTextureImage_ = device_->CreateImage([&](auto &builder) {
        builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
        builder.SetDimensions(bricksTextureHandler_.Width, bricksTextureHandler_.Height);
    });

    if (!quadTextureImage_) {
        throw std::runtime_error("Failed to create texture image!");
    }

    const auto memoryReq = quadTextureImage_->GetImageMemoryRequirements();

    const uint32_t memoryTypeIndex = physicalDevice_->FindMemoryType(memoryReq.memoryTypeBits,
                                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureDeviceMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!textureDeviceMemory_) {
        throw std::runtime_error("Failed to allocate texture device memory!");
    }

    quadTextureImage_->BindImageMemory(textureDeviceMemory_, 0);
}

void VulkanApplication::CreateQuadTextureImageView()
{
    quadTextureImageView_ = device_->CreateImageView(quadTextureImage_, [](auto &builder) {
        builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
    });

    if (!quadTextureImageView_) {
        throw std::runtime_error("Failed to create texture image view!");
    }
}

void VulkanApplication::CreateSampler()
{
    sampler_ = device_->CreateSampler([](auto &builder) {
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
        VkClearValue clearColor;
        clearColor.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass([&](auto &beginInfo) {
            beginInfo.renderPass = renderPass_->GetHandle();
            beginInfo.framebuffer = framebuffers_[i]->GetHandle();
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = VkExtent2D(windowWidth, windowHeight);
            beginInfo.clearValueCount = 1;
            beginInfo.pClearValues = &clearColor;
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                                                  {
                                                      descriptorRegistry_->GetDescriptorSet(
                                                          params_.Get<std::string>(AppConstants::MainDescSetLayout))
                                                  });
        cmdBuffersPresent_[i]->BindVertexBuffers({
                                                     buffers_[params_.Get<std::string>(AppConstants::MainVertexBuffer)]
                                                     ->GetBuffer()
                                                 }, 0, 1, {0});
        cmdBuffersPresent_[i]->BindIndexBuffer(
            buffers_[params_.Get<std::string>(AppConstants::MainIndexBuffer)]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
        cmdBuffersPresent_[i]->DrawIndexed(indexCount, 1, 0, 0, 0);
        cmdBuffersPresent_[i]->EndRenderPass();
        if (!cmdBuffersPresent_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::CopyStagingBuffer()
{
    if (!cmdBufferTransfer_->BeginCommandBuffer([](auto &beginInfo) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {bricksTextureHandler_.Width, bricksTextureHandler_.Height, 1},
    };
    cmdBufferTransfer_->CopyBufferToImage(
        buffers_[params_.Get<std::string>(AppConstants::ImageStagingBuffer)]->GetBuffer(),
        quadTextureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        {copyRegion});

    if (!cmdBufferTransfer_->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue_->Submit({cmdBufferTransfer_});
    queue_->WaitIdle();
}
} // namespace examples::fundamentals::images_and_samplers::combined_image_sampler
