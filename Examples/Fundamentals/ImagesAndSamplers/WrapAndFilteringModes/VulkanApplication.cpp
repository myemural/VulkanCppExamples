/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "AppConfig.h"
#include "ApplicationData.h"
#include "VulkanHelpers.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::images_and_samplers::wrap_and_filtering_modes
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationImagesAndSamplers(std::move(params)) {}

bool VulkanApplication::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(GetParamU32(AppConstants::MaxFramesInFlight));

        CreateResources();
        InitResources();

        CreateDefaultRenderPass();
        CreatePipeline();
        CreateDefaultFramebuffers();

        const uint32_t indexCount = indices.size();
        CreateCommandBuffers();
        RecordPresentCommandBuffers(indexCount);
    } catch (const std::exception& e) {
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
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::Cleanup() noexcept
{
    ApplicationImagesAndSamplers::Cleanup();
    bricksTextureHandler_.Clear();
}

void VulkanApplication::CreateResources()
{
    // Pre-load textures
    const TextureLoader textureLoader{ASSETS_DIR};
    bricksTextureHandler_ = textureLoader.Load(GetParamStr(AppConstants::BricksTexturePath));

    // Fill buffer create infos
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2Uv2);
    const uint32_t indexDataSize = indices.size() * sizeof(indices[0]);
    const std::vector<BufferResourceCreateInfo> bufferCreateInfos = {
        {GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::MainIndexBuffer), indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {GetParamStr(AppConstants::ImageStagingBuffer), bricksTextureHandler_.GetByteSize(),
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
    CreateBuffers(bufferCreateInfos);

    // Fill shader module create infos
    const ShaderModulesCreateInfo shaderModuleCreateInfo = {
        .BasePath = SHADERS_DIR,
        .ShaderType = params_.Get<ShaderBaseType>(AppConstants::BaseShaderType),
        .Modules = {{.Name = GetParamStr(AppConstants::MainVertexShaderKey),
                     .FileName = GetParamStr(AppConstants::MainVertexShaderFile)},
                    {.Name = GetParamStr(AppConstants::MainFragmentShaderKey),
                     .FileName = GetParamStr(AppConstants::MainFragmentShaderFile)}}};
    CreateShaderModules(shaderModuleCreateInfo);

    // Fill descriptor set create infos
    const DescriptorResourceCreateInfo descriptorSetCreateInfo = {
        .MaxSets = 1,
        .PoolSizes = {{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_SAMPLER, 4}},
        .Layouts = {{.Name = GetParamStr(AppConstants::MainDescSetLayout),
                     .Bindings = {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                  {1, VK_DESCRIPTOR_TYPE_SAMPLER, 4, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}}}};
    CreateDescriptorSets(descriptorSetCreateInfo);

    CreateQuadTextureImage();
    CreateQuadTextureImageView();
    CreateSamplers();
}

void VulkanApplication::InitResources()
{
    // Fill push constant data for every quad
    pushConstantData_[TOP_LEFT_QUAD_INDEX] = {.Offset = {-0.5, -0.5}, .SamplerIndex = TOP_LEFT_QUAD_INDEX};
    pushConstantData_[TOP_RIGHT_QUAD_INDEX] = {.Offset = {0.5, -0.5}, .SamplerIndex = TOP_RIGHT_QUAD_INDEX};
    pushConstantData_[BOTTOM_LEFT_QUAD_INDEX] = {.Offset = {-0.5, 0.5}, .SamplerIndex = BOTTOM_LEFT_QUAD_INDEX};
    pushConstantData_[BOTTOM_RIGHT_QUAD_INDEX] = {.Offset = {0.5, 0.5}, .SamplerIndex = BOTTOM_RIGHT_QUAD_INDEX};

    SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), vertices.data(), vertices.size() * sizeof(VertexPos2Uv2));
    SetBuffer(GetParamStr(AppConstants::MainIndexBuffer), indices.data(), indices.size() * sizeof(indices[0]));
    SetBuffer(GetParamStr(AppConstants::ImageStagingBuffer), bricksTextureHandler_.Data,
              bricksTextureHandler_.GetByteSize());

    UpdateDescriptorSets();

    ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyStagingBuffer();
    ChangeImageLayout(quadTextureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanApplication::CreatePipeline()
{
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(PushConstantData);
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineLayout_ = device_->CreatePipelineLayout(
            {descriptorRegistry_->GetDescriptorLayout(GetParamStr(AppConstants::MainDescSetLayout))}, {pushConstant});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos2Uv2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Position, bindingIndex);
    const auto uvAttribDescription = GenerateAttributeDescription(VertexPos2Uv2, Uv, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription, uvAttribDescription};

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module =
                    shaderResources_->GetShaderModule(GetParamStr(AppConstants::MainVertexShaderKey))->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    shaderResources_->GetShaderModule(GetParamStr(AppConstants::MainFragmentShaderKey))->GetHandle();
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
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
    });

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::UpdateDescriptorSets() const
{
    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.emplace_back(VK_NULL_HANDLE, quadTextureImageView_->GetHandle(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> samplerInfos;
    samplerInfos.emplace_back(samplerTopLeft_->GetHandle());
    samplerInfos.emplace_back(samplerTopRight_->GetHandle());
    samplerInfos.emplace_back(samplerBottomLeft_->GetHandle());
    samplerInfos.emplace_back(samplerBottomRight_->GetHandle());

    ImageWriteRequest imageUpdateRequest;
    imageUpdateRequest.LayoutName = GetParamStr(AppConstants::MainDescSetLayout);
    imageUpdateRequest.BindingIndex = 0;
    imageUpdateRequest.Images = imageInfos;
    imageUpdateRequest.Type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    ImageWriteRequest samplersUpdateRequest;
    samplersUpdateRequest.LayoutName = GetParamStr(AppConstants::MainDescSetLayout);
    samplersUpdateRequest.BindingIndex = 1;
    samplersUpdateRequest.Images = samplerInfos;
    samplersUpdateRequest.Type = VK_DESCRIPTOR_TYPE_SAMPLER;

    const DescriptorUpdateInfo descriptorSetUpdateInfo = {
        .ImageWriteRequests = {imageUpdateRequest, samplersUpdateRequest}};

    UpdateDescriptorSet(descriptorSetUpdateInfo);
}

void VulkanApplication::CreateQuadTextureImage()
{
    quadTextureImage_ = device_->CreateImage([&](auto& builder) {
        builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB);
        builder.SetDimensions(bricksTextureHandler_.Width, bricksTextureHandler_.Height);
    });

    if (!quadTextureImage_) {
        throw std::runtime_error("Failed to create texture image!");
    }

    const auto memoryReq = quadTextureImage_->GetImageMemoryRequirements();

    const uint32_t memoryTypeIndex =
            physicalDevice_->FindMemoryType(memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    textureDeviceMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!textureDeviceMemory_) {
        throw std::runtime_error("Failed to allocate texture device memory!");
    }

    quadTextureImage_->BindImageMemory(textureDeviceMemory_, 0);
}

void VulkanApplication::CreateQuadTextureImageView()
{
    quadTextureImageView_ = device_->CreateImageView(quadTextureImage_,
                                                     [](auto& builder) { builder.SetFormat(VK_FORMAT_R8G8B8A8_SRGB); });

    if (!quadTextureImageView_) {
        throw std::runtime_error("Failed to create texture image view!");
    }
}

void VulkanApplication::CreateSamplers()
{
    samplerTopLeft_ = device_->CreateSampler([](auto& builder) {
        builder.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
        builder.SetAddressModes(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    });

    samplerTopRight_ = device_->CreateSampler([](auto& builder) {
        builder.SetFilters(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
        builder.SetAddressModes(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                VK_SAMPLER_ADDRESS_MODE_REPEAT);
    });

    samplerBottomLeft_ = device_->CreateSampler([](auto& builder) {
        builder.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
        builder.SetAddressModes(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                                VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
    });

    samplerBottomRight_ = device_->CreateSampler([](auto& builder) {
        builder.SetFilters(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
        builder.SetFilters(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
        builder.SetAddressModes(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
        builder.SetBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    });

    if (!samplerTopLeft_ || !samplerTopRight_ || !samplerBottomLeft_ || !samplerBottomRight_) {
        throw std::runtime_error("Failed to create samplers!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t indexCount)
{
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkClearValue clearColor;
        clearColor.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[i]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &clearColor;
                },
                VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindDescriptorSets(
                VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0,
                {descriptorRegistry_->GetDescriptorSet(GetParamStr(AppConstants::MainDescSetLayout))});
        cmdBuffersPresent_[i]->BindVertexBuffers({buffers_[GetParamStr(AppConstants::MainVertexBuffer)]->GetBuffer()},
                                                 0, 1, {0});
        cmdBuffersPresent_[i]->BindIndexBuffer(buffers_[GetParamStr(AppConstants::MainIndexBuffer)]->GetBuffer(), 0,
                                               VK_INDEX_TYPE_UINT16);

        for (auto& data: pushConstantData_) {
            cmdBuffersPresent_[i]->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                                                 sizeof(PushConstantData), &data);
            cmdBuffersPresent_[i]->DrawIndexed(indexCount, 1, 0, 0, 0);
        }
        cmdBuffersPresent_[i]->EndRenderPass();
        if (!cmdBuffersPresent_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::CopyStagingBuffer()
{
    const auto cmdBufferTransfer = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferTransfer->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        .imageOffset = {0, 0, 0},
        .imageExtent = {bricksTextureHandler_.Width, bricksTextureHandler_.Height, 1},
    };
    cmdBufferTransfer->CopyBufferToImage(buffers_[GetParamStr(AppConstants::ImageStagingBuffer)]->GetBuffer(),
                                         quadTextureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copyRegion});

    if (!cmdBufferTransfer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue_->Submit({cmdBufferTransfer});
    queue_->WaitIdle();
}
} // namespace examples::fundamentals::images_and_samplers::wrap_and_filtering_modes
