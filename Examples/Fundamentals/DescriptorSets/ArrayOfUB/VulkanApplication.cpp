/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "TimeUtils.h"
#include "VulkanDescriptorPool.h"
#include "VulkanHelpers.h"
#include "VulkanShaderModule.h"

namespace examples::fundamentals::descriptor_sets::array_of_ub
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationDescriptorSets(std::move(params)) {}

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
        CreateDefaultSyncObjects();

        InitAssetManager();
        CreateResources();
        InitResources();

        CreateDefaultRenderPass();
        CreatePipeline();
        CreateDefaultFramebuffers();

        const uint32_t indexCount = indices.size();
        CreateCommandBuffers();
        RecordCommandBuffers(indexCount); // Recording in Init for this example
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void VulkanApplication::DrawFrame()
{
    inFlightFences_[currentIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentIndex_];

    UpdateUniformBuffers();

    queue_->Submit({cmdBuffers_[imageIndex]}, {imageAvailableSemaphores_[currentIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentIndex_ = (currentIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
}

void VulkanApplication::CreateResources()
{
    const std::uint32_t vertexBufferSize = vertices.size() * sizeof(VertexPos2);
    const std::uint32_t indexBufferSize = indices.size() * sizeof(std::uint16_t);
    constexpr std::uint32_t uniformBufferSize = sizeof(UniformBufferObject);

    const std::vector<BufferResourceCreateInfo> bufferCreateInfos = {
        {kMainVertexBuffer, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kMainIndexBuffer, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kTopLeftUB, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kTopRightUB, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kBottomLeftUB, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
        {kBottomRightUB, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};
    CreateBuffers(bufferCreateInfos);

    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    const ShaderModulesCreateInfo shaderModuleCreateInfo = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};
    CreateShaderModules(shaderModuleCreateInfo);

    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    CreateDescriptorSet();
}

void VulkanApplication::InitResources()
{
    modelUbObject[TOP_LEFT_QUAD_INDEX].model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));
    modelUbObject[TOP_RIGHT_QUAD_INDEX].model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));
    modelUbObject[BOTTOM_LEFT_QUAD_INDEX].model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
    modelUbObject[BOTTOM_RIGHT_QUAD_INDEX].model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));

    SetBuffer(kMainVertexBuffer, vertices.data(), vertices.size() * sizeof(VertexPos2));
    SetBuffer(kMainIndexBuffer, indices.data(), indices.size() * sizeof(std::uint16_t));
    SetBuffer(kTopLeftUB, &modelUbObject[TOP_LEFT_QUAD_INDEX], sizeof(UniformBufferObject));
    SetBuffer(kTopRightUB, &modelUbObject[TOP_RIGHT_QUAD_INDEX], sizeof(UniformBufferObject));
    SetBuffer(kBottomLeftUB, &modelUbObject[BOTTOM_LEFT_QUAD_INDEX], sizeof(UniformBufferObject));
    SetBuffer(kBottomRightUB, &modelUbObject[BOTTOM_RIGHT_QUAD_INDEX], sizeof(UniformBufferObject));
}

void VulkanApplication::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 4;
    descriptorPool_ = device_->CreateDescriptorPool(1, {poolSize}, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    if (!descriptorPool_) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void VulkanApplication::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, VK_SHADER_STAGE_VERTEX_BIT, nullptr};

    descriptorSetLayout_ = device_->CreateDescriptorSetLayout({binding});

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

    VkDescriptorBufferInfo bufferInfoTopLeft;
    {
        bufferInfoTopLeft.buffer = buffers_[kTopLeftUB]->GetBuffer()->GetHandle();
        bufferInfoTopLeft.offset = 0;
        bufferInfoTopLeft.range = VK_WHOLE_SIZE;
    }

    VkDescriptorBufferInfo bufferInfoTopRight;
    {
        bufferInfoTopRight.buffer = buffers_[kTopRightUB]->GetBuffer()->GetHandle();
        bufferInfoTopRight.offset = 0;
        bufferInfoTopRight.range = VK_WHOLE_SIZE;
    }

    VkDescriptorBufferInfo bufferInfoBottomLeft;
    {
        bufferInfoBottomLeft.buffer = buffers_[kBottomLeftUB]->GetBuffer()->GetHandle();
        bufferInfoBottomLeft.offset = 0;
        bufferInfoBottomLeft.range = VK_WHOLE_SIZE;
    }

    VkDescriptorBufferInfo bufferInfoBottomRight;
    {
        bufferInfoBottomRight.buffer = buffers_[kBottomRightUB]->GetBuffer()->GetHandle();
        bufferInfoBottomRight.offset = 0;
        bufferInfoBottomRight.range = VK_WHOLE_SIZE;
    }

    const std::vector<VkDescriptorBufferInfo> bufferInfos = {bufferInfoTopLeft, bufferInfoTopRight,
                                                             bufferInfoBottomLeft, bufferInfoBottomRight};

    const auto descriptorWriteModelMatrix =
            descriptorSet_->CreateWriteDescriptorSet(0, 0, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfos);

    device_->UpdateDescriptorSets({descriptorWriteModelMatrix});
}

void VulkanApplication::CreatePipeline()
{
    VkPushConstantRange indexPushConstant;
    indexPushConstant.offset = 0;
    indexPushConstant.size = sizeof(int);
    indexPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineLayout_ = device_->CreatePipelineLayout({descriptorSetLayout_}, {indexPushConstant});

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
    auto bindingDescription = GenerateBindingDescription<VertexPos2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2, Position, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription};

    pipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = shaderResources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
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

void VulkanApplication::RecordCommandBuffers(const std::uint32_t indexCount)
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
        cmdBuffers_[i]->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, {descriptorSet_});
        cmdBuffers_[i]->BindVertexBuffers({buffers_[kMainVertexBuffer]->GetBuffer()}, 0, 1, {0});
        cmdBuffers_[i]->BindIndexBuffer(buffers_[kMainIndexBuffer]->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
        for (int j = 0; j < 4; ++j) {
            cmdBuffers_[i]->PushConstants(pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(int), &j);
            cmdBuffers_[i]->DrawIndexed(indexCount, 1, 0, 0, 0);
        }
        cmdBuffers_[i]->EndRenderPass();
        if (!cmdBuffers_[i]->EndCommandBuffer()) {
            throw std::runtime_error("Failed to end recording command buffer!");
        }
    }
}

void VulkanApplication::UpdateUniformBuffers()
{
    const auto currentTime = static_cast<float>(GetCurrentTime());
    const float translation = std::sin(currentTime) * 0.2f;
    const float rotation = currentTime * 2.5f;
    const float scalingFactor = std::sin(currentTime) * 0.5f + 1.0f; // Range: 0.5 - 1.5

    UniformBufferObject tempObject{};
    tempObject.model = glm::rotate(modelUbObject[TOP_LEFT_QUAD_INDEX].model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    SetBuffer(kTopLeftUB, &tempObject, sizeof(UniformBufferObject));
    tempObject.model =
            glm::scale(modelUbObject[TOP_RIGHT_QUAD_INDEX].model, glm::vec3(scalingFactor, scalingFactor, 0.0f));
    SetBuffer(kTopRightUB, &tempObject, sizeof(UniformBufferObject));
    tempObject.model = glm::translate(modelUbObject[BOTTOM_LEFT_QUAD_INDEX].model, glm::vec3(translation, 0.0f, 0.0f));
    SetBuffer(kBottomLeftUB, &tempObject, sizeof(UniformBufferObject));
    tempObject.model = glm::translate(modelUbObject[BOTTOM_RIGHT_QUAD_INDEX].model, glm::vec3(0.0f, translation, 0.0f));
    SetBuffer(kBottomRightUB, &tempObject, sizeof(UniformBufferObject));
}
} // namespace examples::fundamentals::descriptor_sets::array_of_ub
