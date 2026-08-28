/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    28.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAdvancedComputeShaders.h"
#include "ApplicationData.h"
#include "AssetManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
{
class VulkanApplication final : public base::ApplicationAdvancedComputeShaders
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void InitAssetManager();

    void CreateInitialResources() const;

    void CreateAndUpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void CreateQueryPools();

    void PrintPerformanceStats();

    void RecordSimulationStep(const std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer,
                              std::uint32_t stepIndex);

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> quadPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> computePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> quadPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> collideStreamPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> advectDyePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> paintObstaclePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> colorizePipeline_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Timestamp query pools
    std::shared_ptr<common::vulkan_wrapper::VulkanQueryPool> timestampQueryPool_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Simulation state
    std::uint32_t lbmStepCounter_ = 0U;
    bool isSimulationPaused_ = false;
    bool isFirstFrameDone_ = false;
    DisplayMode displayMode_ = DisplayMode::VORTICITY;

    // Mouse state
    glm::vec2 mousePos_ = glm::vec2(0.0f);
    MouseMode mouseMode_ = MouseMode::NONE;

    // Performance statistics
    std::uint32_t statFrameCounter_ = 0U;
};
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_lbm
