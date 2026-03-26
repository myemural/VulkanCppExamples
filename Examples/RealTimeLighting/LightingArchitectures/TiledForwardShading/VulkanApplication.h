/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    11.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "PerspectiveCamera.h"
#include "ApplicationLightingArchitectures.h"
#include "AssetManager.h"
#include "MaterialManager.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::real_time_lighting::lighting_architectures::tiled_forward_shading
{
class VulkanApplication final : public base::ApplicationLightingArchitectures
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

    void ProcessInput() const;

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> depthPrePassRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> forwardRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> depthPrePassPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> forwardPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightCullPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> depthPrePassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> tiledForwardPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> tileLightCullPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> depthPrePassFramebuffer_;
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
    std::unique_ptr<common::vulkan_framework::MaterialManager> materialManager_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Light
    std::vector<glm::vec4> lightPositions_;
    std::vector<glm::vec3> lightColors_;
};
} // namespace examples::real_time_lighting::lighting_architectures::tiled_forward_shading
