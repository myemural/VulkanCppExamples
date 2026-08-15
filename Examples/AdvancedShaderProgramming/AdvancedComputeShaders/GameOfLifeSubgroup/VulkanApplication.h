/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAdvancedComputeShaders.h"
#include "AssetManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
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

    void CreateRenderPass();

    void CreatePipelines();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> quadPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> computePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> quadPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> computePipeline_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    std::uint32_t frameCounter_ = 0;
};
} // namespace examples::advanced_shader_programming::advanced_compute_shaders::game_of_life_subgroup
