/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanApplication.h"

#include <algorithm>
#include <array>

#include "AppCommonConfig.h"
#include "AppConfig.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanHelpers.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationTessellationShaders(std::move(params)) {}

bool VulkanApplication::Init()
{
    if (!ApplicationTessellationShaders::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
        CreateFramebuffers();
        CreateCommandBuffers();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void VulkanApplication::DrawFrame()
{
    inFlightFences_[currentFrameIndex_]->WaitForFence(true, UINT64_MAX);

    uint32_t imageIndex = swapChain_->AcquireNextImage(imageAvailableSemaphores_[currentFrameIndex_], nullptr);

    if (swapImagesFences_[imageIndex] != nullptr) {
        swapImagesFences_[imageIndex]->WaitForFence(true, UINT64_MAX);
    }

    inFlightFences_[currentFrameIndex_]->ResetFence();
    swapImagesFences_[imageIndex] = inFlightFences_[currentFrameIndex_];

    UpdateSceneTransforms();
    RecordPresentCommandBuffers(imageIndex);

    queue_->Submit({cmdBuffersPresent_[imageIndex]}, {imageAvailableSemaphores_[currentFrameIndex_]},
                   {renderFinishedSemaphores_[imageIndex]}, inFlightFences_[currentFrameIndex_],
                   {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});

    queue_->Present({swapChain_}, {imageIndex}, {renderFinishedSemaphores_[imageIndex]});

    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::InitAssetManager()
{
    assetManager_ = std::make_unique<AssetManager>();
    assetManager_->RegisterLoader<ShaderAsset>(std::make_unique<ShaderLoader>(SHADERS_DIR, SHADER_TYPE));
}

void VulkanApplication::CreateInitialResources() const
{
    ResourceDescriptor resourceCreateInfo;

    // Fill buffer create infos
    const uint32_t controlPointsDataSize = controlPoints_.size() * sizeof(controlPoints_[0]);
    resourceCreateInfo.buffers = {{kControlPointsBuffer, controlPointsDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}};

    // Fill shader module create infos
    const auto mainVertexShaderAsset = assetManager_->Load<ShaderAsset>(kMainVertexShaderFile);
    const auto mainTessControlShaderAsset = assetManager_->Load<ShaderAsset>(kMainTessControlShaderFile);
    const auto mainTessEvaluationShaderAsset = assetManager_->Load<ShaderAsset>(kMainTessEvaluationShaderFile);
    const auto mainFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kMainFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {{.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
                    {.name = kMainTessControlShaderKey, .asset = assetManager_->Get(mainTessControlShaderAsset)},
                    {.name = kMainTessEvaluationShaderKey, .asset = assetManager_->Get(mainTessEvaluationShaderAsset)},
                    {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::InitInputSystem()
{
    lastX_ = static_cast<float>(currentWindowWidth_) / 2.0f;
    lastY_ = static_cast<float>(currentWindowHeight_) / 2.0f;

    window_->OnMouseButton([&](const MouseButtonEvent& event) {
        if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (event.action == GLFW_PRESS) {
                selectedControlPointIndex_ = PickControlPoint(lastX_, lastY_);
            } else if (event.action == GLFW_RELEASE) {
                selectedControlPointIndex_ = -1;
            }
        }
    });

    window_->OnMouseMove([&](const MouseMoveEvent& event) {
        lastX_ = static_cast<float>(event.x);
        lastY_ = static_cast<float>(event.y);

        // Bounds check for mouse move
        if (lastX_ < 0 || lastX_ > static_cast<float>(currentWindowWidth_) || lastY_ < 0 ||
            lastY_ > static_cast<float>(currentWindowHeight_)) {
            return;
        }

        if (selectedControlPointIndex_ >= 0) {
            controlPoints_[selectedControlPointIndex_] = VertexPos2{.Position = {.data{ScreenToNDC(lastX_, lastY_)}}};
        }
    });
}

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
               })
                .AddSubpass([&](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                    subpassCreateInfo.pDepthStencilAttachment = nullptr;
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanApplication::CreatePipelines()
{
    VkPushConstantRange pushConstants;
    pushConstants.offset = 0;
    pushConstants.size = sizeof(PushConstants);
    pushConstants.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayout_ = device_->CreatePipelineLayout({}, {pushConstants});

    if (!pipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos2>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos2, Position, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription};

    controlPointsPipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
        });
        builder.SetInputAssemblyState([&](auto& inputAssemblyStateCreateInfo) {
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
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
        builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
            rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_POINT;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
    });

    if (!controlPointsPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for control points)!");
    }

    bezierCurvePipeline_ = device_->CreateGraphicsPipeline(pipelineLayout_, renderPass_, [&](auto& builder) {
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainVertexShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainTessControlShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainTessEvaluationShaderKey)->GetHandle();
        });
        builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
        });
        builder.SetInputAssemblyState([&](auto& inputAssemblyStateCreateInfo) {
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        });
        builder.SetTessellationState([&](auto& tessellationStateCreateInfo) {
            tessellationStateCreateInfo.patchControlPoints = kPatchCount;
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
        builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
            rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
            rasterizationStateCreateInfo.lineWidth = 2.0f;
        });
        builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
            blendStateCreateInfo.attachmentCount = 1;
            blendStateCreateInfo.pAttachments = &colorBlendAttachment;
        });
    });

    if (!bezierCurvePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for bezier curve)!");
    }
}

void VulkanApplication::CreateFramebuffers()
{
    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage}, [&](auto& builder) {
            builder.SetDimensions(currentWindowWidth_, currentWindowHeight_);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void VulkanApplication::CreateCommandBuffers()
{
    cmdBuffersPresent_ = cmdPool_->CreateCommandBuffers(framebuffers_.size(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    if (cmdBuffersPresent_.empty()) {
        throw std::runtime_error("Failed to create command buffers!");
    }
}

void VulkanApplication::RecordPresentCommandBuffers(const std::uint32_t currentImageIndex)
{
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);

    const auto& currentCmdBuffer = cmdBuffersPresent_[currentImageIndex];

    if (!currentCmdBuffer->BeginCommandBuffer(nullptr)) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    currentCmdBuffer->BeginRenderPass(
            [&](auto& beginInfo) {
                beginInfo.renderPass = renderPass_->GetHandle();
                beginInfo.framebuffer = framebuffers_[currentImageIndex]->GetHandle();
                beginInfo.renderArea.offset = {0, 0};
                beginInfo.renderArea.extent = VkExtent2D(currentWindowWidth_, currentWindowHeight_);
                beginInfo.clearValueCount = clearValues.size();
                beginInfo.pClearValues = clearValues.data();
            },
            VK_SUBPASS_CONTENTS_INLINE);

    const std::vector vertexBuffers{resources_->GetBuffer(kControlPointsBuffer)};
    currentCmdBuffer->BindVertexBuffers(vertexBuffers, 0, 1, {0});

    // Draw control points
    {
        currentCmdBuffer->BindPipeline(controlPointsPipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        PushConstants pushConstants{};
        pushConstants.controlPointSize = GetParamFloat(AppSettings::ControlPointSize);
        pushConstants.shapeColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::ControlPointColor), 1.0f);
        currentCmdBuffer->PushConstants(pipelineLayout_,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                        0, sizeof(pushConstants), &pushConstants);
        currentCmdBuffer->Draw(kPatchCount, 1, 0, 0);
    }

    // Draw bezier curve
    {
        currentCmdBuffer->BindPipeline(bezierCurvePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        PushConstants pushConstants{};
        pushConstants.tessLevel = GetParamFloat(AppSettings::TessellationLevel);
        pushConstants.shapeColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::CurveColor), 1.0f);
        currentCmdBuffer->PushConstants(pipelineLayout_,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                        0, sizeof(pushConstants), &pushConstants);
        currentCmdBuffer->Draw(kPatchCount, 1, 0, 0);
    }

    currentCmdBuffer->EndRenderPass();
    if (!currentCmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }
}

void VulkanApplication::UpdateSceneTransforms() const
{
    const uint32_t controlPointsDataSize = controlPoints_.size() * sizeof(controlPoints_[0]);
    resources_->SetBuffer(kControlPointsBuffer, controlPoints_.data(), controlPointsDataSize);
}

glm::vec2 VulkanApplication::ScreenToNDC(const float mouseX, const float mouseY) const
{
    const float ndcX = (2.0f * mouseX) / static_cast<float>(currentWindowWidth_) - 1.0f;
    const float ndcY = (2.0f * mouseY) / static_cast<float>(currentWindowHeight_) - 1.0f;
    return {ndcX, ndcY};
}

int VulkanApplication::PickControlPoint(const float mouseX, const float mouseY) const
{
    int closestIndex = -1;
    float closestDist = GetParamFloat(AppSettings::ControlPointSize);

    for (int i = 0; i < controlPoints_.size(); ++i) {
        const float px = (controlPoints_[i].Position.data.x * 0.5f + 0.5f) * static_cast<float>(currentWindowWidth_);
        const float py = (controlPoints_[i].Position.data.y * 0.5f + 0.5f) * static_cast<float>(currentWindowHeight_);

        if (const float dist = glm::length(glm::vec2(px - mouseX, py - mouseY)); dist < closestDist) {
            closestDist = dist;
            closestIndex = i;
        }
    }

    return closestIndex;
}
} // namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
