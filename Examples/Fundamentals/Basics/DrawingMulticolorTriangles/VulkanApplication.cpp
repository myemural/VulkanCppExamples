/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "AppConfig.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "VulkanHelpers.h"

namespace examples::fundamentals::basics::drawing_multicolor_triangles
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationBasics(std::move(params)) {}

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

        const uint32_t vertexCount = vertices.size();
        CreateCommandBuffers();
        RecordCommandBuffers(vertexCount); // Recording in Init for this example
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

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::CreateResources()
{
    const uint32_t vertexDataSize = vertices.size() * sizeof(vertices[0]);
    CreateVertexBuffer(vertexDataSize);

    CreateShaderModules();
}

void VulkanApplication::InitResources() const
{
    const uint32_t vertexDataSize = vertices.size() * sizeof(vertices[0]);
    FillVertexBuffer(vertices.data(), vertexDataSize);
}

void VulkanApplication::CreateVertexBuffer(std::uint64_t dataSize)
{
    vertexBuffer_ = device_->CreateBuffer([&](auto& builder) {
        builder.SetSize(dataSize);
        builder.SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    });

    if (!vertexBuffer_) {
        throw std::runtime_error("Failed to create vertex buffer!");
    }

    const auto memoryReq = vertexBuffer_->GetBufferMemoryRequirements();

    const uint32_t memoryTypeIndex = physicalDevice_->FindMemoryType(
            memoryReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vertexBufferMemory_ = device_->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!vertexBufferMemory_) {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }

    vertexBuffer_->BindBufferMemory(vertexBufferMemory_, 0);
}

void VulkanApplication::FillVertexBuffer(const void* data, const std::uint64_t dataSize) const
{
    void* vertexData = vertexBufferMemory_->MapMemory(VK_WHOLE_SIZE, 0);

    if (!vertexData) {
        throw std::runtime_error("Failed to map vertex buffer data!");
    }

    std::memcpy(vertexData, data, dataSize);

    vertexBufferMemory_->FlushMappedMemoryRanges({{VK_WHOLE_SIZE, 0}});

    vertexBufferMemory_->UnmapMemory();
}

void VulkanApplication::CreateShaderModules()
{
    const ShaderLoader shaderLoader{SHADERS_DIR, params_.Get<ShaderBaseType>(AppConstants::BaseShaderType)};
    // Vertex Shader
    const auto vertexShaderCode = shaderLoader.LoadSpirV(GetParamStr(AppConstants::MainVertexShaderFile));
    const auto vertexShaderModule = device_->CreateShaderModule(vertexShaderCode);
    if (!vertexShaderModule) {
        throw std::runtime_error("Failed to create vertex shader module!");
    }
    shaderModules_[GetParamStr(AppConstants::MainVertexShaderKey)] = vertexShaderModule;

    // Fragment Shader
    const auto fragmentShaderCode = shaderLoader.LoadSpirV(GetParamStr(AppConstants::MainFragmentShaderFile));
    const auto fragmentShaderModule = device_->CreateShaderModule(fragmentShaderCode);
    if (!fragmentShaderModule) {
        throw std::runtime_error("Failed to create fragment shader module!");
    }
    shaderModules_[GetParamStr(AppConstants::MainFragmentShaderKey)] = fragmentShaderModule;
}

void VulkanApplication::CreatePipeline()
{
    pipelineLayout_ = device_->CreatePipelineLayout();

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
    auto bindingDescription = GenerateBindingDescription<VertexPos2Color3>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2Color3, Position, bindingIndex);
    const auto colorAttribDescription = GenerateAttributeDescription(VertexPos2Color3, Color, bindingIndex);
    const std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{posAttribDescription,
                                                                                 colorAttribDescription};

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderModules_[GetParamStr(AppConstants::MainVertexShaderKey)]->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module =
                    shaderModules_[GetParamStr(AppConstants::MainFragmentShaderKey)]->GetHandle();
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

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffers_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffers_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordCommandBuffers(const std::uint32_t vertexCount)
{
    for (size_t i = 0; i < framebuffers_.size(); ++i) {
        VkClearValue clearColor;
        clearColor.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
        if (!cmdBuffers_[i]->BeginCommandBuffer(nullptr)) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        cmdBuffers_[i]->BeginRenderPass(
                [&](auto& beginInfo) {
                    beginInfo.renderPass = renderPass_->GetHandle();
                    beginInfo.framebuffer = framebuffers_[i]->GetHandle();
                    beginInfo.renderArea.offset = {0, 0};
                    beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                    beginInfo.clearValueCount = 1;
                    beginInfo.pClearValues = &clearColor;
                },
                VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffers_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffers_[i]->BindVertexBuffers({vertexBuffer_}, 0, 1, {0});
        cmdBuffers_[i]->Draw(vertexCount, 1, 0, 0);
        cmdBuffers_[i]->EndRenderPass();
        if (!cmdBuffers_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}
} // namespace examples::fundamentals::basics::drawing_multicolor_triangles
