/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    06.06.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationDisneyPrincipledBrdf.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::physically_based_rendering::disney_principled_brdf::disney_diffuse_specular
{
class VulkanApplication final : public base::ApplicationDisneyPrincipledBrdf
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void InitAssetManager();

    void CreateInitialResources() const;

    void BuildScene();

    void CreateAndUpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCubemapFramebuffers();

    void CreateCommandBuffers();

    void PrecalculateCubemapImages();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    void ProcessInput();

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutSkybox_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutBrdfLut_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutPrefilteredCubemap_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> scenePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> convertToCubemapPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> irradianceConvolutionPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> skyboxPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> brdfLutPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> prefilteredCubemapPipeline_;

    // Additional Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> cubemapRenderPass_;

    // Additional Framebuffers
    std::array<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>, 6> environmentCubemapFramebuffers_;
    std::array<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>, 6> irradianceCubemapFramebuffers_;

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

    // Shader options
    bool isIblEnabled_ = true;
};
} // namespace examples::physically_based_rendering::disney_principled_brdf::disney_diffuse_specular
