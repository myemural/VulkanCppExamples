/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    7.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationComputeShaders.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::compute_shaders::fullscreen_gradient
{
class VulkanApplication final : public base::ApplicationComputeShaders
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void InitAssetManager();

    void CreateResources();

    void InitResources() const;

    void CreateRenderPass();

    void CreatePipelines();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> gradientComputePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> gradientComputePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> graphicsPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> graphicsPipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;
};
} // namespace examples::fundamentals::compute_shaders::fullscreen_gradient
