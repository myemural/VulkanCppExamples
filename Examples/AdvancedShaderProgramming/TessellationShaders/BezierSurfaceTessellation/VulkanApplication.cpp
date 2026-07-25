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
#include "MathUtils.h"
#include "SceneObjectBuilder.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "VulkanHelpers.h"
#include "VulkanShaderModule.h"

namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
{
using namespace constants;
using namespace common::asset_manager;
using namespace common::camera;
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

VulkanApplication::VulkanApplication(ParameterServer&& params) : ApplicationTessellationShaders(std::move(params))
{
    // Initialize control points
    constexpr int kGridSize = 4;
    for (int row = 0; row < kGridSize; ++row) {
        for (int col = 0; col < kGridSize; ++col) {
            constexpr float kSpacing = 2.0f;
            const float x = (static_cast<float>(col) - 1.5f) * kSpacing;
            const float z = (static_cast<float>(row) - 1.5f) * kSpacing;
            const float y = GenerateRandomValue(-1.0f, 1.0f); // Randomize Z height between -1.0 and 1.0
            const VertexPos3 currentPos{.Position = {glm::vec3(x, y, z)}};
            controlPoints_.emplace_back(currentPos);
        }
    }
}

bool VulkanApplication::Init()
{
    if (!ApplicationTessellationShaders::Init()) {
        std::cerr << "Application pre-init failed!" << '\n';
        return false;
    }

    try {
        InitAssetManager();
        CreateInitialResources();
        BuildCamera();

        InitInputSystem();

        CreateRenderPass();
        CreatePipelines();
        CreateDefaultFramebuffers(resources_->GetImageView(kDepthImage, kDepthImageView));
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
    const auto controlPointsVertexShaderAsset = assetManager_->Load<ShaderAsset>(kControlPointsVertexShaderFile);
    const auto controlPointsFragmentShaderAsset = assetManager_->Load<ShaderAsset>(kControlPointsFragmentShaderFile);

    resourceCreateInfo.shaders = {
        .modules = {
            {.name = kMainVertexShaderKey, .asset = assetManager_->Get(mainVertexShaderAsset)},
            {.name = kMainTessControlShaderKey, .asset = assetManager_->Get(mainTessControlShaderAsset)},
            {.name = kMainTessEvaluationShaderKey, .asset = assetManager_->Get(mainTessEvaluationShaderAsset)},
            {.name = kMainFragmentShaderKey, .asset = assetManager_->Get(mainFragmentShaderAsset)},
            {.name = kControlPointsVertexShaderKey, .asset = assetManager_->Get(controlPointsVertexShaderAsset)},
            {.name = kControlPointsFragmentShaderKey, .asset = assetManager_->Get(controlPointsFragmentShaderAsset)}}};

    resourceCreateInfo.images = {ImageResourceCreateInfo{
        .name = kDepthImage,
        .memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .format = depthImageFormat_,
        .dimensions = {currentWindowWidth_, currentWindowHeight_, 1},
        .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .views = {ImageViewCreateInfo{.viewName = kDepthImageView,
                                      .format = depthImageFormat_,
                                      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                                           .baseMipLevel = 0,
                                                           .levelCount = 1,
                                                           .baseArrayLayer = 0,
                                                           .layerCount = 1}}}}};

    CreateVulkanResources(resourceCreateInfo);
}

void VulkanApplication::BuildCamera()
{
    const float aspectRatio = static_cast<float>(currentWindowWidth_) / static_cast<float>(currentWindowHeight_);
    camera_ = std::make_shared<PerspectiveCamera>(glm::vec3(0.0f, 0.0f, orbitDistance_), aspectRatio);
}

void VulkanApplication::InitInputSystem()
{
    window_->OnMouseButton([&](const MouseButtonEvent& event) {
        // If you hold down the right mouse button, the camera can move
        if (event.button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (event.action == GLFW_PRESS) {
                window_->DisableCursor();
                firstMouseTriggered_ = true;
                isOrbiting_ = true;
            } else if (event.action == GLFW_RELEASE) {
                window_->EnableCursor();
                firstMouseTriggered_ = false;
                isOrbiting_ = false;
            }
        }

        // If you hold down the left mouse button, you can move a selected control point
        if (event.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (event.action == GLFW_PRESS) {
                const Ray ray = ScreenPointToWorldRay(lastX_, lastY_);
                selectedControlPointIndex_ = PickClosestControlPoint(ray);

                if (selectedControlPointIndex_ != -1) {
                    isDraggingControlPoint_ = true;

                    // Drag plane: The plane passing through the selected point and facing the camera
                    dragPlaneNormal_ = glm::normalize(camera_->GetPosition() -
                                                      controlPoints_[selectedControlPointIndex_].Position.data);
                }
            } else if (event.action == GLFW_RELEASE) {
                isDraggingControlPoint_ = false;
                selectedControlPointIndex_ = -1;
            }
        }
    });

    window_->OnMouseMove([this](const MouseMoveEvent& event) {
        const auto xPos = static_cast<float>(event.x);
        const auto yPos = static_cast<float>(event.y);

        if (firstMouseTriggered_) {
            lastX_ = xPos;
            lastY_ = yPos;
            firstMouseTriggered_ = false;
            return;
        }

        // Dragging control points
        if (isDraggingControlPoint_ && selectedControlPointIndex_ != -1) {
            const auto [origin, direction] = ScreenPointToWorldRay(xPos, yPos);

            const glm::vec3 planePoint = controlPoints_[selectedControlPointIndex_].Position.data;
            if (const float denom = glm::dot(direction, dragPlaneNormal_); glm::abs(denom) > 1e-6f) {
                if (const float t = glm::dot(planePoint - origin, dragPlaneNormal_) / denom; t >= 0.0f) {
                    const glm::vec3 newPosition = origin + direction * t;
                    controlPoints_[selectedControlPointIndex_].Position.data = newPosition;
                }
            }

            lastX_ = xPos;
            lastY_ = yPos;
            return;
        }

        // If the right-click button isn't pressed, ignore the camera movement, but keep the position updated
        if (!isOrbiting_) {
            lastX_ = xPos;
            lastY_ = yPos;
            return;
        }

        const float xOffset = xPos - lastX_;
        const float yOffset = lastY_ - yPos;

        lastX_ = xPos;
        lastY_ = yPos;

        const float sensitivity = GetParamFloat(AppSettings::CameraMoveSensitivity) * static_cast<float>(deltaTime_);

        const float yawSensitivity = sensitivity;
        const float pitchSensitivity = sensitivity * 0.5f; // Should less sensitive than yaw

        orbitYaw_ += xOffset * yawSensitivity;
        orbitPitch_ += yOffset * pitchSensitivity;
        orbitPitch_ = glm::clamp(orbitPitch_, -89.0f, 89.0f);
    });

    // For zooming to target
    window_->OnMouseScroll([this](const MouseScrollEvent& event) {
        constexpr auto minZoom = 0.2f;
        constexpr auto maxZoom = 90.0f;
        const auto zoomSpeed = GetParamFloat(AppSettings::CameraZoomSpeed);
        orbitDistance_ = glm::clamp(orbitDistance_ - static_cast<float>(event.deltaY) * zoomSpeed, minZoom, maxZoom);
    });
}

void VulkanApplication::CreateRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference depthAttachmentRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

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
                .AddAttachment([&](auto& attachmentCreateInfo) {
                    attachmentCreateInfo.format = depthImageFormat_;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                })
                .AddSubpass([&](auto& subpassCreateInfo) {
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

void VulkanApplication::CreatePipelines()
{
    VkViewport viewport{0,    0,   static_cast<float>(currentWindowWidth_), static_cast<float>(currentWindowHeight_),
                        0.0f, 1.0f};
    VkRect2D scissor{0, 0, currentWindowWidth_, currentWindowHeight_};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    constexpr uint32_t bindingIndex = 0;
    auto bindingDescription = GenerateBindingDescription<VertexPos3>(bindingIndex);
    const auto posAttribDescription = GenerateAttributeDescription(VertexPos3, Position, bindingIndex);
    const std::array attributeDescriptions{posAttribDescription};

    VkPushConstantRange controlPointsPushConstants;
    controlPointsPushConstants.offset = 0;
    controlPointsPushConstants.size = sizeof(ControlPointsPushConstants);
    controlPointsPushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    controlPointsPipelineLayout_ = device_->CreatePipelineLayout({}, {controlPointsPushConstants});

    if (!controlPointsPipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for control points)!");
    }

    controlPointsPipeline_ =
            device_->CreateGraphicsPipeline(controlPointsPipelineLayout_, renderPass_, [&](auto& builder) {
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kControlPointsVertexShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kControlPointsFragmentShaderKey)->GetHandle();
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
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                });
            });

    if (!controlPointsPipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for control points)!");
    }

    VkPushConstantRange bezierSurfacePushConstants;
    bezierSurfacePushConstants.offset = 0;
    bezierSurfacePushConstants.size = sizeof(SurfacePushConstants);
    bezierSurfacePushConstants.stageFlags =
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    bezierSurfacePipelineLayout_ = device_->CreatePipelineLayout({}, {bezierSurfacePushConstants});

    if (!bezierSurfacePipelineLayout_) {
        throw std::runtime_error("Failed to create pipeline layout (for bezier surface)!");
    }

    bezierSurfacePipeline_ =
            device_->CreateGraphicsPipeline(bezierSurfacePipelineLayout_, renderPass_, [&](auto& builder) {
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
                    shaderStageCreateInfo.module =
                            resources_->GetShaderModule(kMainTessEvaluationShaderKey)->GetHandle();
                });
                builder.AddShaderStage([&](auto& shaderStageCreateInfo) {
                    shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shaderStageCreateInfo.module = resources_->GetShaderModule(kMainFragmentShaderKey)->GetHandle();
                });
                builder.SetVertexInputState([&](auto& vertexInputStateCreateInfo) {
                    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
                    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
                    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
                    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
                });
                builder.SetInputAssemblyState([&](auto& inputAssemblyStateCreateInfo) {
                    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
                    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
                });
                builder.SetTessellationState([&](auto& tessellationStateCreateInfo) {
                    tessellationStateCreateInfo.patchControlPoints = kPatchCount;
                });
                builder.SetViewportState([&](auto& viewportStateCreateInfo) {
                    viewportStateCreateInfo.viewportCount = 1;
                    viewportStateCreateInfo.pViewports = &viewport;
                    viewportStateCreateInfo.scissorCount = 1;
                    viewportStateCreateInfo.pScissors = &scissor;
                });
                builder.SetRasterizationState([&](auto& rasterizationStateCreateInfo) {
                    rasterizationStateCreateInfo.polygonMode = params_.Get<VkPolygonMode>(AppSettings::PolygonMode);
                    rasterizationStateCreateInfo.lineWidth = 1.5f;
                });
                builder.SetColorBlendState([&](auto& blendStateCreateInfo) {
                    blendStateCreateInfo.attachmentCount = 1;
                    blendStateCreateInfo.pAttachments = &colorBlendAttachment;
                });
                builder.SetDepthStencilState([&](auto& depthStencilStateCreateInfo) {
                    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
                    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                });
            });

    if (!bezierSurfacePipeline_) {
        throw std::runtime_error("Failed to create graphics pipeline (for bezier surface)!");
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
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = params_.Get<VkClearColorValue>(AppSettings::ClearColor);
    clearValues[1].depthStencil = {1.0f, 0};

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
        ControlPointsPushConstants pushConstants{};
        pushConstants.view = camera_->GetViewMatrix();
        pushConstants.projection = camera_->GetProjectionMatrix();
        pushConstants.pointColor = glm::vec4(params_.Get<glm::vec3>(AppSettings::ControlPointColor), 1.0f);
        pushConstants.pointSize = GetParamFloat(AppSettings::ControlPointSize);
        currentCmdBuffer->PushConstants(controlPointsPipelineLayout_,
                                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                        sizeof(pushConstants), &pushConstants);
        currentCmdBuffer->Draw(kPatchCount, 1, 0, 0);
    }

    // Draw bezier curve
    {
        currentCmdBuffer->BindPipeline(bezierSurfacePipeline_, VK_PIPELINE_BIND_POINT_GRAPHICS);
        SurfacePushConstants pushConstants{};
        pushConstants.view = camera_->GetViewMatrix();
        pushConstants.projection = camera_->GetProjectionMatrix();
        pushConstants.tessLevel = GetParamFloat(AppSettings::TessellationLevel);
        currentCmdBuffer->PushConstants(bezierSurfacePipelineLayout_,
                                        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
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
    // Update orientation of the orbit camera in every frame
    const float yawRad = glm::radians(orbitYaw_);
    const float pitchRad = glm::radians(orbitPitch_);

    glm::vec3 position;
    position.x = orbitTarget_.x + orbitDistance_ * cosf(pitchRad) * cosf(yawRad);
    position.y = orbitTarget_.y + orbitDistance_ * sinf(pitchRad);
    position.z = orbitTarget_.z + orbitDistance_ * cosf(pitchRad) * sinf(yawRad);

    camera_->SetPosition(position);

    const glm::vec3 front = glm::normalize(orbitTarget_ - position);

    const float viewYaw = glm::degrees(atan2(front.z, front.x));
    const float viewPitch = glm::degrees(asin(front.y));

    camera_->SetRotation(viewYaw, viewPitch);

    // Update control points in every frame
    const uint32_t controlPointsDataSize = controlPoints_.size() * sizeof(controlPoints_[0]);
    resources_->SetBuffer(kControlPointsBuffer, controlPoints_.data(), controlPointsDataSize);
}

Ray VulkanApplication::ScreenPointToWorldRay(const float screenX, const float screenY) const
{
    const float ndcX = (2.0f * screenX) / static_cast<float>(currentWindowWidth_) - 1.0f;
    const float ndcY = (2.0f * screenY) / static_cast<float>(currentWindowHeight_) - 1.0f;

    const glm::vec4 nearClip(ndcX, ndcY, 0.0f, 1.0f);
    const glm::vec4 farClip(ndcX, ndcY, 1.0f, 1.0f);

    const glm::mat4 invViewProj = glm::inverse(camera_->GetProjectionMatrix() * camera_->GetViewMatrix());

    glm::vec4 nearWorld = invViewProj * nearClip;
    glm::vec4 farWorld = invViewProj * farClip;
    nearWorld /= nearWorld.w;
    farWorld /= farWorld.w;

    const auto origin = glm::vec3(nearWorld);
    const auto direction = glm::normalize(glm::vec3(farWorld) - glm::vec3(nearWorld));
    return Ray{origin, direction};
}

int VulkanApplication::PickClosestControlPoint(const Ray& ray) const
{
    int closestIndex = -1;
    float closestDistSq = kControlPointPickRadius * kControlPointPickRadius;

    for (int i = 0; i < static_cast<int>(controlPoints_.size()); ++i) {
        const glm::vec3 point = controlPoints_[i].Position.data;
        const glm::vec3 toPoint = point - ray.origin;

        const float t = glm::dot(toPoint, ray.direction);
        if (t < 0.0f) {
            continue; // If it is behind of the camera, continue
        }

        const glm::vec3 closestPointOnRay = ray.origin + ray.direction * t;
        const glm::vec3 diff = point - closestPointOnRay;
        if (const float distSq = glm::dot(diff, diff); distSq < closestDistSq) {
            closestDistSq = distSq;
            closestIndex = i;
        }
    }

    return closestIndex;
}

} // namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
