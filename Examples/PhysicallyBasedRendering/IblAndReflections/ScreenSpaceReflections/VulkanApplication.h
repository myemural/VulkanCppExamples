/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    31.05.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationIblAndReflections.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
{
class VulkanApplication final : public base::ApplicationIblAndReflections
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void InitAssetManager();

    void CreateInitialResources() const;

    void BuildScene();

    void CreateAndUpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    void ProcessInput();

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> ssrRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> ssrPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> ssrPassPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> lightingFramebuffer_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::shared_ptr<common::camera::PerspectiveCamera> camera_ = nullptr;

    // Scene manager
    std::unique_ptr<common::scene::Scene> scene_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Light
    std::array<glm::vec4, kLightCount> lightPositions_{};
    std::array<glm::vec4, kLightCount> lightColorsAndIntensities_{};

    // SSR Debugging
    bool isSsrEnabled_ = true;
};
} // namespace examples::physically_based_rendering::ibl_and_reflections::screen_space_reflections
