/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    06.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationMeshAndTaskShaders.h"
#include "AssetManager.h"
#include "ModelLoader.h"
#include "PerspectiveCamera.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
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

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void ProcessInput() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

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

    // Object model matrices
    std::vector<glm::mat4> modelMatrices_;

    // Model specific data
    std::unique_ptr<common::asset_manager::GltfModelAsset> suzanneModelAsset_;
    common::asset_manager::MeshletLodData suzanneMeshletLodData_;
    std::vector<std::uint32_t> meshletTrianglesExpanded_;
    std::uint32_t totalMeshletCount_ = 0;
};
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::lod_task_shader
