/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    05.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <memory>

#include "ApplicationMeshAndTaskShaders.h"
#include "AssetManager.h"
#include "ModelLoader.h"
#include "PerspectiveCamera.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
{
class VulkanApplication final : public base::ApplicationMeshAndTaskShaders
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void InitInputSystem();

    void InitAssetManager();

    void CreateResources();

    void InitResources() const;

    void InitTransformsAndCameras();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCullPipeline();

    void CreateLinePipeline();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateResources() const;

    void ProcessInput() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutLines_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipelineLines_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Cameras
    std::unique_ptr<common::camera::PerspectiveCamera> camera_;
    std::unique_ptr<common::camera::PerspectiveCamera> debugCamera_;

    // Debug viewport data
    std::uint32_t debugViewportWidth_ = 0;
    std::uint32_t debugViewportHeight_ = 0;
    std::uint32_t debugViewportX_ = 0;
    std::uint32_t debugViewportY_ = 0;

    // Object model matrices
    std::vector<glm::mat4> modelMatrices_;

    // Model specific data
    std::unique_ptr<common::asset_manager::GltfModelAsset> suzanneModelAsset_;
    common::asset_manager::MeshletData suzanneMeshletData_;
    std::vector<std::uint32_t> meshletTrianglesExpanded_;
    std::uint32_t totalMeshletCount_ = 0;
};
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::frustum_culling_task_shader
