/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include "ShaderLoader.h"
#include "VulkanHelpers.h"
#include "ApplicationData.h"
#include "VulkanDescriptorPool.h"
#include "VulkanShaderModule.h"
#include "AppConfig.h"

namespace examples::fundamentals::descriptor_sets::basic_push_constants
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
                currentColor = firstColor;
                break;
            case GLFW_KEY_2:
                currentColor = secondColor;
                break;
            case GLFW_KEY_3:
                currentColor = thirdColor;
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    }
}

VulkanApplication::VulkanApplication(ParameterServer &&params)
    : ApplicationDescriptorSets(std::move(params))
{
    currentWindowWidth_ = GetParamU32(WindowParams::Width);
    currentWindowHeight_ = GetParamU32(WindowParams::Height);

    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2);
    const std::uint32_t indexBufferSize = indices.size() * sizeof(uint16_t);
    bufferCreateInfos_ = {
        {
            GetParamStr(AppConstants::MainVertexBuffer), vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        },
        {
            GetParamStr(AppConstants::MainIndexBuffer), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
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
    // Bind input callback
    glfwSetKeyCallback(window_->GetGLFWWindow(), KeyCallback);

    try {
        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();

        CreateBuffers(bufferCreateInfos_);
        SetBuffer(GetParamStr(AppConstants::MainVertexBuffer), vertices.data(),
                  vertices.size() * sizeof(VertexPos2));
        SetBuffer(GetParamStr(AppConstants::MainIndexBuffer), indices.data(),
                  indices.size() * sizeof(uint16_t));

        CreateDefaultRenderPass();
        CreateShaderModules(shaderModuleCreateInfo_);
        CreatePipeline();
        CreateDefaultFramebuffers();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects(GetParamU32(AppConstants::MaxFramesInFlight));
        CreateCommandBuffers();
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

    const uint32_t indexCount = indices.size();
    RecordCommandBuffers(imageIndex, indexCount);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[currentIndex_]}, inFlightFences_[currentIndex_], {
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                   });

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[currentIndex_]});

    currentIndex_ = (currentIndex_ + 1) % GetParamU32(AppConstants::MaxFramesInFlight);
}

void VulkanApplication::CreatePipeline()
{
    VkPushConstantRange colorPushConstant;
    colorPushConstant.offset = 0;
    colorPushConstant.size = sizeof(PushConstantObject);
    colorPushConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineLayout_ = device_->CreatePipelineLayout({}, {colorPushConstant});

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

void VulkanApplication::RecordCommandBuffers(const std::uint32_t currentImageIndex, const std::uint32_t indexCount)
{
    VkClearValue clearColor;
    clearColor.color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    if (!cmdBuffers_[currentImageIndex]->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    cmdBuffers_[currentImageIndex]->BeginRenderPass([&](auto &beginInfo) {
        beginInfo.renderPass = renderPass_->GetHandle();
        beginInfo.framebuffer = framebuffers_[currentImageIndex]->GetHandle();
        beginInfo.renderArea.offset = {0, 0};
        beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &clearColor;
    }, VK_SUBPASS_CONTENTS_INLINE);
    cmdBuffers_[currentImageIndex]->BindPipeline(pipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
    cmdBuffers_[currentImageIndex]->PushConstants(pipelineLayout_, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                  sizeof(PushConstantObject), &currentColor);
    cmdBuffers_[currentImageIndex]->BindVertexBuffers({
                                                          buffers_[GetParamStr(
                                                              AppConstants::MainVertexBuffer)]->GetBuffer()
                                                      }, 0, 1, {0});
    cmdBuffers_[currentImageIndex]->BindIndexBuffer(
        buffers_[GetParamStr(AppConstants::MainIndexBuffer)]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
    cmdBuffers_[currentImageIndex]->DrawIndexed(indexCount, 1, 0, 0, 0);
    cmdBuffers_[currentImageIndex]->EndRenderPass();
    if (!cmdBuffers_[currentImageIndex]->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}
} // namespace examples::fundamentals::descriptor_sets::basic_push_constants
