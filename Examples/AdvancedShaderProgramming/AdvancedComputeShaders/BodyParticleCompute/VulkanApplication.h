/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    21.08.2026
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
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
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

    void CreateInitialResources();

    void CreateAndUpdateDescriptorSets() const;

    void CreateRenderPass();

    void CreatePipelines();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateCamera() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> particlePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> computePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> particlePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> computePipeline_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::shared_ptr<common::camera::PerspectiveCamera> camera_ = nullptr;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Frame counter variable
    std::uint32_t frameCounter_ = 0;
};
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::body_particle_compute
