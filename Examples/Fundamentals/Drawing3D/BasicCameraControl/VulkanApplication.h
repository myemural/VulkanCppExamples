/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    09.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationDrawing3D.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::drawing_3d::basic_camera_control
{
class VulkanApplication final : public base::ApplicationDrawing3D
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void InitInputSystem();

    void CreateResources();

    void InitResources();

    void CreateRenderPass();

    void CreatePipeline();

    void UpdateDescriptorSets();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CalculateAndSetMvp();

    void ProcessInput();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    MvpData mvpData_[NUM_CUBES] = {glm::mat4(1.0)};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Camera values
    glm::vec3 cameraPos_ = glm::vec3(0.0f, 0.0f, 4.0f);
    glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);

    // Delta time related values
    double deltaTime_ = 0.0f;
    double lastFrame_ = 0.0f;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float yawAngle_ = -90.0f;
    float pitchAngle_ = 0.0f;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;
};
} // namespace examples::fundamentals::drawing_3d::basic_camera_control
