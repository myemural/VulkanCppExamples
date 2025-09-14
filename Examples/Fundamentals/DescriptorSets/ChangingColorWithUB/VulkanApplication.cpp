/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "AppConfig.h"
#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer &&params)
    : ApplicationDescriptorSets(std::move(params))
{
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2);
    constexpr std::uint32_t uniformBufferSize = sizeof(params_.Get<Color3>(AppSettings::TriangleColor));

    bufferCreateInfos_ = {
        {
            params_.Get<std::string>(AppConstants::MainVertexBuffer), vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            params_.Get<std::string>(AppConstants::MainUniformBuffer), uniformBufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        }
    };

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
}

bool VulkanApplication::Init()
{
    try {
        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();

        CreateBuffers(bufferCreateInfos_);
        SetBuffer(params_.Get<std::string>(AppConstants::MainVertexBuffer), vertices.data(),
                  vertices.size() * sizeof(VertexPos2));
        const auto triangleColor = params_.Get<Color3>(AppSettings::TriangleColor);
        SetBuffer(params_.Get<std::string>(AppConstants::MainUniformBuffer), &triangleColor, sizeof(Color3));

        CreateDefaultRenderPass();
        CreateShaderModules(shaderModuleCreateInfo_);
        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSet();
        CreatePipeline();
        CreateDefaultFramebuffers();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(params_.Get<std::uint32_t>(AppConstants::MaxFramesInFlight));
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

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[currentIndex_]}, inFlightFences_[currentIndex_], {
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                   });

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[currentIndex_]});

    currentIndex_ = (currentIndex_ + 1) % params_.Get<std::uint32_t>(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::CreateDescriptorSetLayout()
{
    const std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            nullptr
        }
    };

    descriptorSetLayout_ = device_->CreateDescriptorSetLayout(bindings);

    if (!descriptorSetLayout_) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanApplication::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;
    descriptorPool_ = device_->CreateDescriptorPool(1, {poolSize}, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    if (!descriptorPool_) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void VulkanApplication::CreateDescriptorSet()
{
    descriptorSet_ = descriptorPool_->CreateDescriptorSets({descriptorSetLayout_}).front();

    if (!descriptorSet_) {
        throw std::runtime_error("Failed to create descriptor set!");
    }

    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.emplace_back(
        buffers_[params_.Get<std::string>(AppConstants::MainUniformBuffer)]->GetBuffer()->GetHandle(), 0,
        VK_WHOLE_SIZE);

    const auto descriptorWrite = descriptorSet_->CreateWriteDescriptorSet(
        0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfos);

    device_->UpdateDescriptorSets({descriptorWrite});
}

void VulkanApplication::CreatePipeline()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    pipelineLayout_ = device_->CreatePipelineLayout({descriptorSetLayout_});

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

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffers_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffers_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordCommandBuffers(const std::uint32_t vertexCount)
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

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
            beginInfo.renderArea.extent = VkExtent2D(windowWidth, windowHeight);
            beginInfo.clearValueCount = 1;
            beginInfo.pClearValues = &clearColor;
        }, VK_SUBPASS_CONTENTS_INLINE);
        cmdBuffers_[i]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        cmdBuffers_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, {descriptorSet_});
        cmdBuffers_[i]->BindVertexBuffers({
                                              buffers_[params_.Get<std::string>(AppConstants::MainVertexBuffer)]->
                                              GetBuffer()
                                          }, 0, 1, {0});
        cmdBuffers_[i]->Draw(vertexCount, 1, 0, 0);
        cmdBuffers_[i]->EndRenderPass();
        if (!cmdBuffers_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}
} // namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
