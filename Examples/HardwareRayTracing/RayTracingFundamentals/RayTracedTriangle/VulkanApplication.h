/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationRayTracingFundamentals.h"
#include "AssetManager.h"
#include "ShaderBindingTable.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
{
class VulkanApplication final : public base::ApplicationRayTracingFundamentals
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

    void CreateAccelerationStructures() const;

    void CreateAndUpdateDescriptorSets() const;

    void CreateRenderPass();

    void CreatePipelines();

    void CreateShaderBindingTable();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t currentImageIndex) const;

    void UpdateSceneUniforms() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> rayTracingPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> presentPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> rayTracingPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> presentPipeline_;

    // Shader binding table of the ray tracing pipeline
    std::unique_ptr<common::vulkan_framework::ShaderBindingTable> shaderBindingTable_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;
};
} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::ray_traced_triangle
