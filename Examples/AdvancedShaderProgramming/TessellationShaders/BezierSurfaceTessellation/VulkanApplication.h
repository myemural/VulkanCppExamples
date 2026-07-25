/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    25.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationTessellationShaders.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Vertex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
{
class VulkanApplication final : public base::ApplicationTessellationShaders
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

    void BuildCamera();

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    [[nodiscard]] Ray ScreenPointToWorldRay(float screenX, float screenY) const;

    [[nodiscard]] int PickClosestControlPoint(const Ray& ray) const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> controlPointsPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> bezierSurfacePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> controlPointsPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> bezierSurfacePipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::shared_ptr<common::camera::PerspectiveCamera> camera_ = nullptr;
    glm::vec3 orbitTarget_ = glm::vec3(0.0f);
    float orbitDistance_ = 8.0f;
    float orbitYaw_ = 90.0f;
    float orbitPitch_ = 20.0f;
    bool isOrbiting_ = false;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Control points related values
    std::vector<common::utility::VertexPos3> controlPoints_;
    int selectedControlPointIndex_ = -1; // -1 means not selected
    bool isDraggingControlPoint_ = false;
    glm::vec3 dragPlaneNormal_{0.0f};    // The normal of the plane facing the camera during dragging
};
} // namespace examples::advanced_shader_programming::tessellation_shaders::bezier_surface_tessellation
