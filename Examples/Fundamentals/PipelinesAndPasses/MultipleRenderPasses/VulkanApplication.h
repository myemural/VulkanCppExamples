/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    28.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationPipelinesAndPasses.h"
#include "PerspectiveCamera.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
{
class VulkanApplication final : public base::ApplicationPipelinesAndPasses
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

    void CreateRenderPasses();

    void CreatePipelines();

    void CreateFramebuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanImageView>& depthImageView);

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CalculateAndSetMvp();

    void ProcessInput() const;

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    MvpData cubeMvpData[NUM_CUBES] = {glm::mat4(1.0)};
    UniformBufferObject timeSpeedUbo{};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};
    common::utility::TextureHandler cloudTextureHandler_{};

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> backgroundRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> foregroundRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> backgroundPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> backgroundPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> cubePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> cubePipeline_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> backgroundFramebuffers_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> foregroundFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera;
};
} // namespace examples::fundamentals::pipelines_and_passes::multiple_render_passes
