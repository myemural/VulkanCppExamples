/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    19.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAdvancedComputeShaders.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
{
class VulkanApplication final : public base::ApplicationAdvancedComputeShaders
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

    void PostUpdate() override;

private:
    void InitAssetManager();

    void CreateInitialResources();

    void BuildScene();

    void UploadObjectBounds() const;

    void SetAndMapCullingResultsBuffer();

    void CreateAndUpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCommandBuffers();

    void
    RecordComputeCommandBuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer) const;

    void RecordAllCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateLightUBO() const;

    void ProcessInput() const;

    void ReportStats();

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> hizCopyComputePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> hizReduceComputePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> occlusionComputePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> scenePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> hizCopyComputePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> hizReduceComputePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> occlusionComputePipeline_;

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

    // Other variables
    std::uint32_t frameCounter_ = 0; // For testing first frame
    std::uint32_t hizMipCount_ = 0;
    std::uint32_t* cullingResultsData_ = nullptr;
    std::uint32_t drawnObjectCount_ = 0;
    bool isFrustumCullingEnabled_ = true;
    bool isOcclusionCullingEnabled_ = true;
    double lastStatsReportTime_ = 0.0;
};
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::occlusion_culling_compute
