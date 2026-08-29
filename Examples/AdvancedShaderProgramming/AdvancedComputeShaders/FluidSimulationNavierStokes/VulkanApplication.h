/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    29.08.2026
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
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
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

    void BuildScene();

    void CreateAndUpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCommandBuffers();

    void RecordSimulationPasses(const std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer) const;

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneUniforms() const;

    void UpdateOrbitCamera() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> advectPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> jacobiPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> projectPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> scenePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> smokePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> advectPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> jacobiPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> projectPipeline_;

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

    // Orbit camera state
    float orbitDistance_ = kOrbitDistance;
    float orbitYaw_ = kOrbitYaw;
    float orbitPitch_ = kOrbitPitch;

    // Simulation state
    bool volumesInitialized_ = false;
    bool seedRequested_ = true;
    bool simulationPaused_ = false;
};
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::fluid_simulation_navier_stokes
