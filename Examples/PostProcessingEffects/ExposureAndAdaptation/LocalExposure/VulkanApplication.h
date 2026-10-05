/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    05.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationExposureAndAdaptation.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::post_processing_effects::exposure_and_adaptation::local_exposure
{
class VulkanApplication final : public base::ApplicationExposureAndAdaptation
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
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightingRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> postProcessingRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> autoExposurePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> localExposureSetupPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> localExposureFilterPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> postProcessingPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> luminanceHistogramPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> histogramAveragePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> localExposureSetupPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> localExposureFilterPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> postProcessingPassPipeline_;

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

    // Material registry
    std::unordered_map<std::string, common::scene::Material> materialRegistry_;

    // Current tone mapping operation value
    ToneMappingMode toneMappingMode_ = ToneMappingMode::REINHARD;

    // Control variables
    bool resetAdaptation_ = true; // First frame snaps to the target (buffer is uninitialized)
    bool isBrightRoomLightsOn_ = true;
    bool isLocalExposureEnabled_ = true;

    // Runtime-adjustable values
    float darkToBrightAdaptationSpeed_ = kDarkToBrightAdaptationSpeed;
    float brightToDarkAdaptationSpeed_ = kBrightToDarkAdaptationSpeed;
    float lowPercentile_ = kHistogramLowPercentile;
    float highPercentile_ = kHistogramHighPercentile;
    float localContrastScale_ = kLocalExposureContrastScale;
    float blurredLuminanceBlend_ = kLocalExposureBlurredLuminanceBlend;
};
} // namespace examples::post_processing_effects::exposure_and_adaptation::local_exposure
