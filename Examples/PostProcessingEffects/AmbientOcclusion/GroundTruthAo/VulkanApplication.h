/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    08.09.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAmbientOcclusion.h"
#include "ApplicationData.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::post_processing_effects::ambient_occlusion::ground_truth_ao
{
class VulkanApplication final : public base::ApplicationAmbientOcclusion
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
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> gtaoRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> gtaoPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> gtaoBlurPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> gtaoPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> gtaoBlurPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> gtaoFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> gtaoBlurFramebuffer_;
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

    // GTAO values
    bool isGtaoEnabled_ = true;
    float gtaoRadius_ = kInitialGtaoRadius;
    float gtaoThickness_ = kInitialGtaoThickness;
    float gtaoPower_ = kInitialGtaoPower;

    // Debug values
    DebugMode debugMode_ = DebugMode::OFF;
};
} // namespace examples::post_processing_effects::ambient_occlusion::ground_truth_ao
