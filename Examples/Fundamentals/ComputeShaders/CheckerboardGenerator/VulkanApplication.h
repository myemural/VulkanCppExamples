/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    9.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationComputeShaders.h"
#include "ApplicationData.h"
#include "PerspectiveCamera.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::compute_shaders::checkerboard_generator
{
class VulkanApplication final : public base::ApplicationComputeShaders
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

    void InitResources() const;

    void CreateRenderPass();

    void CreatePipelines();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CalculateAndSetMvp();

    void ProcessInput() const;

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    std::uint32_t checkerboardTextureSize_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    MvpData mvpDataCubes_[NUM_CUBES] = {glm::mat4(1.0)};
    MvpData mvpDataSpheres_[NUM_SPHERES] = {glm::mat4(1.0)};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> checkerboardComputePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> checkerboardComputePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> graphicsPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> graphicsPipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera_;
};
} // namespace examples::fundamentals::compute_shaders::checkerboard_generator
