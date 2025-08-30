/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>
#include <queue>

#include "ShaderLoader.h"
#include "VulkanHelpers.h"
#include "ApplicationData.h"

namespace examples::fundamentals::basics::using_staging_buffer
{

using namespace common::utility;
using namespace common::vulkan_wrapper;

VulkanApplication::VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings)
: ApplicationBasics(config), settings_(settings)
{
}

bool VulkanApplication::Init()
{
    try {
        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();

        const uint32_t vertexDataSize = vertices.size() * sizeof(VertexPos2);
        CreateVertexBuffer(vertexDataSize);
        CreateStagingBuffer(vertexDataSize);
        FillStagingBuffer(vertices.data(), vertexDataSize);

        CreateDefaultRenderPass();
        CreateShaderModules();
        CreatePipeline();
        CreateDefaultFramebuffers();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(kMaxFramesInFlight);
        CreateCommandBuffers();

        const uint32_t vertexCount = vertices.size();
        RecordCommandBuffers(vertexCount); // Recording in Init for this example
        CopyStagingBuffer(vertexDataSize); // Record transfer related command buffer and submit it directly
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

    currentIndex_ = (currentIndex_ + 1) % kMaxFramesInFlight;
}

void VulkanApplication::CreateVertexBuffer(std::uint64_t dataSize)
{
    vertexBuffer_ = device_->CreateBuffer([&](auto &builder) {
        builder.SetSize(dataSize);
        // Need transfer dst bit
        builder.SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    });

    if (!vertexBuffer_) {
        throw std::runtime_error("Failed to create vertex buffer!");
    }

    const auto memoryReq = vertexBuffer_->GetBufferMemoryRequirements();

    // Get memory type index for device-only (GPU) storage
    const uint32_t memoryTypeIndex = physicalDevice_->FindMemoryType(memoryReq.memoryTypeBits,
                                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vertexBufferMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!vertexBufferMemory_) {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }

    vertexBuffer_->BindBufferMemory(vertexBufferMemory_, 0);
}

void VulkanApplication::CreateStagingBuffer(std::uint64_t dataSize)
{
    stagingBuffer_ = device_->CreateBuffer([&](auto &builder) {
        builder.SetSize(dataSize);
        // Need transfer src bit
        builder.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    });

    if (!stagingBuffer_) {
        throw std::runtime_error("Failed to create staging buffer!");
    }

    const auto memoryReq = stagingBuffer_->GetBufferMemoryRequirements();

    // Get memory type index for storage that accessible by CPU
    const uint32_t memoryTypeIndex = physicalDevice_->FindMemoryType(memoryReq.memoryTypeBits,
                                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    stagingBufferMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!stagingBufferMemory_) {
        throw std::runtime_error("Failed to allocate staging buffer memory!");
    }

    stagingBuffer_->BindBufferMemory(stagingBufferMemory_, 0);
}

void VulkanApplication::FillStagingBuffer(const void *data, const std::uint64_t dataSize) const
{
    void* stagingData = stagingBufferMemory_->MapMemory(VK_WHOLE_SIZE, 0);

    if (!stagingData) {
        throw std::runtime_error("Failed to map staging buffer data!");
    }

    std::memcpy(stagingData, data, dataSize);

    stagingBufferMemory_->FlushMappedMemoryRanges({{VK_WHOLE_SIZE, 0}});

    stagingBufferMemory_->UnmapMemory();
}

void VulkanApplication::CreateShaderModules()
{
    const ShaderLoader shaderLoader{SHADERS_DIR, kCurrentShaderType};
    // Vertex Shader
    const auto vertexShaderCode = shaderLoader.LoadSpirV(kVertexShaderFileName);
    const auto vertexShaderModule = device_->CreateShaderModule(vertexShaderCode);
    if (!vertexShaderModule) {
        throw std::runtime_error("Failed to create vertex shader module!");
    }
    shaderModules_[kVertexShaderHash] = vertexShaderModule;

    // Fragment Shader
    const auto fragmentShaderCode = shaderLoader.LoadSpirV(kFragmentShaderFileName);
    const auto fragmentShaderModule = device_->CreateShaderModule(fragmentShaderCode);
    if (!fragmentShaderModule) {
        throw std::runtime_error("Failed to create fragment shader module!");
    }
    shaderModules_[kFragmentShaderHash] = fragmentShaderModule;
}

void VulkanApplication::CreatePipeline()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    pipelineLayout_ = device_->CreatePipelineLayout();

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
    auto bindingDescription = GenerateBindingDescription<VertexPos2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2, Position, bindingIndex);
    const std::array attributeDescriptions{
        posAttribDescription
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
        builder.SetInputAssemblyState([&](auto& inputAssemblyStateCreateInfo) {
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
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
    });

    if (!pipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    // 3 command buffers for fill framebuffers
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers for presentation!");
    }

    // 1 command buffer for transfer data from staging buffer to vertex buffer
    cmdBufferTransfer_ = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferTransfer_) {
        throw std::runtime_error("Failed to create command buffer for transfer!");
    }
}

void VulkanApplication::RecordCommandBuffers(const std::uint32_t vertexCount)
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkClearValue clearColor;
        clearColor.color = settings_.ClearColor;
        if (!cmdBuffersPresent_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffersPresent_[i]->BeginRenderPass([&](auto& beginInfo) {
            beginInfo.renderPass = renderPass_->GetHandle();
            beginInfo.framebuffer = framebuffers_[i]->GetHandle();
            beginInfo.renderArea.offset = { 0, 0 };
            beginInfo.renderArea.extent = VkExtent2D(windowWidth, windowHeight);
            beginInfo.clearValueCount = 1;
            beginInfo.pClearValues = &clearColor;
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffersPresent_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffersPresent_[i]->BindVertexBuffers({vertexBuffer_}, 0, 1, {0});
        cmdBuffersPresent_[i]->Draw(vertexCount, 1, 0, 0);
        cmdBuffersPresent_[i]->EndRenderPass();
        if (!cmdBuffersPresent_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::CopyStagingBuffer(const std::uint64_t dataSize)
{
    if (!cmdBufferTransfer_->BeginCommandBuffer([](auto& beginInfo) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkBufferCopy copyRegion{};
    copyRegion.size = dataSize;
    cmdBufferTransfer_->CopyBuffer(stagingBuffer_, vertexBuffer_, {copyRegion});

    if (!cmdBufferTransfer_->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue_->Submit({cmdBufferTransfer_});
    queue_->WaitIdle();
}
} // namespace examples::fundamentals::basics::using_staging_buffer