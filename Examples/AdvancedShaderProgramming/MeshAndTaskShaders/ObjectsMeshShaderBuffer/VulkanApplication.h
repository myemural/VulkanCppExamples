/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    02.08.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <memory>

#include "ApplicationData.h"
#include "ApplicationMeshAndTaskShaders.h"
#include "AssetManager.h"
#include "BuiltinPrimitives.h"
#include "ModelLoader.h"
#include "PerspectiveCamera.h"
#include "SceneMesh.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
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

    void CreateResources() const;

    void InitResources();

    void InitModelMatricesAndCamera();

    void CreateRenderPass();

    void CreatePipeline();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneUBO() const;

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

    // Camera
    std::unique_ptr<common::camera::PerspectiveCamera> camera_;

    // GPU mesh and scene objects related data
    std::unordered_map<common::scene::BuiltinMeshType, GpuMesh> gpuMeshes_;
    std::vector<std::pair<common::scene::BuiltinMeshType, glm::mat4>> sceneObjects_;
};
} // namespace examples::advanced_shader_programming::mesh_and_task_shaders::objects_mesh_shader_buffer
