/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    24.07.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <memory>

#include <glm/vec2.hpp>

#include "ApplicationTessellationShaders.h"
#include "AssetManager.h"
#include "Vertex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
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

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    [[nodiscard]] glm::vec2 ScreenToNDC(float mouseX, float mouseY) const;

    [[nodiscard]] int PickControlPoint(float mouseX, float mouseY) const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> controlPointsPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> bezierCurvePipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Asset manager
    std::unique_ptr<common::asset_manager::AssetManager> assetManager_;

    // Control points related values
    std::array<common::utility::VertexPos2, 4> controlPoints_{
        glm::vec3(-0.9f, -0.9f, 0.0f), glm::vec3(-0.9f, 0.9f, 0.0f), glm::vec3(0.9f, -0.9f, 0.0f),
        glm::vec3(0.9f, 0.9f, 0.0f)};
    int selectedControlPointIndex_ = -1; // -1 means not selected
};
} // namespace examples::advanced_shader_programming::tessellation_shaders::cubic_bezier_curve_tessellation
