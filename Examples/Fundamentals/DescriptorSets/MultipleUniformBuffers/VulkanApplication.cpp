/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "AppConfig.h"
#include "ShaderLoader.h"
#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::descriptor_sets::multiple_uniform_buffers
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer &&params)
    : ApplicationDescriptorSets(std::move(params))
{
    currentWindowWidth_ = GetParamU32(WindowParams::Width);
    currentWindowHeight_ = GetParamU32(WindowParams::Height);

    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2);
    constexpr std::uint32_t uniformBufferSize = sizeof(params_.Get<Color3>(AppSettings::InitialTriangleColor));
    bufferCreateInfos_ = {
        {
            GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::ScreenSizeUB), 2 * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::TopLeftUB), uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::TopRightUB), uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::BottomLeftUB), uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::BottomRightUB), uniformBufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }
    };

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
}

bool VulkanApplication::Init()
{
    try {
        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();

        const auto windowWidth = window_->GetWindowWidth();
        const auto windowHeight = window_->GetWindowHeight();
        const std::array screenSizes = {static_cast<float>(windowWidth), static_cast<float>(windowHeight)};
        CreateBuffers(bufferCreateInfos_);
        SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), vertices.data(),
                  vertices.size() * sizeof(VertexPos2));
        SetBuffer(GetParamStr(AppConstants::ScreenSizeUB), &screenSizes, sizeof(screenSizes));
        const auto initialTriangleColor = params_.Get<Color3>(AppSettings::InitialTriangleColor);
        SetBuffer(GetParamStr(AppConstants::TopLeftUB), &initialTriangleColor, sizeof(Color3));
        SetBuffer(GetParamStr(AppConstants::TopRightUB), &initialTriangleColor, sizeof(Color3));
        SetBuffer(GetParamStr(AppConstants::BottomLeftUB), &initialTriangleColor, sizeof(Color3));
        SetBuffer(GetParamStr(AppConstants::BottomRightUB), &initialTriangleColor, sizeof(Color3));

        CreateDefaultRenderPass();
        CreateShaderModules(shaderModuleCreateInfo_);
        CreateDescriptorPool();
        CreateDescriptorSetLayout();
        CreateDescriptorSet();
        CreatePipeline();
        CreateDefaultFramebuffers();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(GetParamU32(AppConstants::MaxFramesInFlight));
        CreateCommandBuffers();

        const uint32_t vertexCount = vertices.size();
        RecordCommandBuffers(vertexCount); // Recording in Init for this example
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

    const auto currentTime = static_cast<float>(glfwGetTime());
    const auto currentValue = 0.5f + 0.5f * std::sin(currentTime * 1.1f);
    const Color3 topLeftColor{currentValue, 0.0f, 0.0f};
    buffers_[GetParamStr(AppConstants::TopLeftUB)]->MapMemory();
    buffers_[GetParamStr(AppConstants::TopLeftUB)]->FlushData(&topLeftColor, sizeof(Color3));
    buffers_[GetParamStr(AppConstants::TopLeftUB)]->UnmapMemory();

    const Color3 topRightColor{0.0f, currentValue, 0.0f};
    buffers_[GetParamStr(AppConstants::TopRightUB)]->MapMemory();
    buffers_[GetParamStr(AppConstants::TopRightUB)]->FlushData(&topRightColor, sizeof(Color3));
    buffers_[GetParamStr(AppConstants::TopRightUB)]->UnmapMemory();

    const Color3 bottomLeftColor{0.0f, 0.0f, currentValue};
    buffers_[GetParamStr(AppConstants::BottomLeftUB)]->MapMemory();
    buffers_[GetParamStr(AppConstants::BottomLeftUB)]->FlushData(&bottomLeftColor, sizeof(Color3));
    buffers_[GetParamStr(AppConstants::BottomLeftUB)]->UnmapMemory();

    const Color3 bottomRightColor{currentValue, 0.0f, currentValue};
    buffers_[GetParamStr(AppConstants::BottomRightUB)]->MapMemory();
    buffers_[GetParamStr(AppConstants::BottomRightUB)]->FlushData(&bottomRightColor, sizeof(Color3));
    buffers_[GetParamStr(AppConstants::BottomRightUB)]->UnmapMemory();

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[currentIndex_]}, inFlightFences_[currentIndex_], {
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                   });

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[currentIndex_]});

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 5;
    descriptorPool_ = device_->CreateDescriptorPool(1, {poolSize}, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    if (!descriptorPool_) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}


void VulkanApplication::CreateDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    for (std::uint32_t i = 0; i < 5; ++i) {
        bindings.emplace_back(i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    }

    descriptorSetLayout_ = device_->CreateDescriptorSetLayout(bindings);

    if (!descriptorSetLayout_) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanApplication::CreateDescriptorSet()
{
    descriptorSet_ = descriptorPool_->CreateDescriptorSets({descriptorSetLayout_}).front();

    if (!descriptorSet_) {
        throw std::runtime_error("Failed to create descriptor set!");
    }

    std::vector<VkDescriptorBufferInfo> bufferInfoScreenSizes;
    bufferInfoScreenSizes.emplace_back(
        buffers_[GetParamStr(AppConstants::ScreenSizeUB)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> bufferInfoTopLeft;
    bufferInfoTopLeft.emplace_back(
        buffers_[GetParamStr(AppConstants::TopLeftUB)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> bufferInfoTopRight;
    bufferInfoTopRight.emplace_back(
        buffers_[GetParamStr(AppConstants::TopRightUB)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> bufferInfoBottomLeft;
    bufferInfoBottomLeft.emplace_back(
        buffers_[GetParamStr(AppConstants::BottomLeftUB)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    std::vector<VkDescriptorBufferInfo> bufferInfoBottomRight;
    bufferInfoBottomRight.emplace_back(
        buffers_[GetParamStr(AppConstants::BottomRightUB)]->GetBuffer()->GetHandle(), 0, VK_WHOLE_SIZE);

    const auto descriptorWriteScreenSize = descriptorSet_->CreateWriteDescriptorSet(
        0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfoScreenSizes);
    const auto descriptorWriteTopLeft = descriptorSet_->CreateWriteDescriptorSet(
        1, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfoTopLeft);
    const auto descriptorWriteTopRight = descriptorSet_->CreateWriteDescriptorSet(
        2, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfoTopRight);
    const auto descriptorWriteBottomLeft = descriptorSet_->CreateWriteDescriptorSet(
        3, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfoBottomLeft);
    const auto descriptorWriteBottomRight = descriptorSet_->CreateWriteDescriptorSet(
        4, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfoBottomRight);

    device_->UpdateDescriptorSets({
        descriptorWriteScreenSize, descriptorWriteTopLeft, descriptorWriteTopRight, descriptorWriteBottomLeft,
        descriptorWriteBottomRight
    });
}

void VulkanApplication::CreatePipeline()
{
    pipelineLayout_ = device_->CreatePipelineLayout({descriptorSetLayout_});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{
        0, 0, static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_), 0.0f, 1.0f
    };
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
    auto bindingDescription = GenerateBindingDescription<VertexPos2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2, Position, bindingIndex);
    const std::array attributeDescriptions{
        posAttribDescription
    };

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto &builder) {
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderModules_[GetParamStr(AppConstants::MainVertexShaderKey)]->
                    GetHandle();
        });
        builder.AddShaderStage([&](auto &shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderModules_[GetParamStr(AppConstants::MainFragmentShaderKey)]
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
        cmdBuffers_[i]->BeginRenderPass([&](auto &beginInfo) {
            beginInfo.renderPass = renderPass_->GetHandle();
            beginInfo.framebuffer = framebuffers_[i]->GetHandle();
            beginInfo.renderArea.offset = {0, 0};
            beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
            beginInfo.clearValueCount = 1;
            beginInfo.pClearValues = &clearColor;
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffers_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffers_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, {descriptorSet_});
        cmdBuffers_[i]->BindVertexBuffers({buffers_[GetParamStr(AppConstants::MainVertexBuffer)]->GetBuffer()}, 0, 1,
                                          {0});
        cmdBuffers_[i]->Draw(vertexCount, 1, 0, 0);
        cmdBuffers_[i]->EndRenderPass();
        if (!cmdBuffers_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}
} // namespace examples::fundamentals::descriptor_sets::multiple_uniform_buffers
