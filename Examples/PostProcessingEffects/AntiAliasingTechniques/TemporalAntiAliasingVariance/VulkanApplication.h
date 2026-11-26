/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    26.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAntiAliasingTechniques.h"
#include "ApplicationData.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::post_processing_effects::anti_aliasing_techniques::temporal_anti_aliasing_variance
{
class VulkanApplication final : public base::ApplicationAntiAliasingTechniques
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

    void CreateFramebuffers();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    void UpdateObjectMotion();

    void ProcessInput() const;

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightingRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> taaResolveRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> postProcessingRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> taaResolvePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> postProcessingPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> taaResolvePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> postProcessingPassPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> lightingFramebuffer_;
    std::array<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>, 2U> historyFramebuffers_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> presentFramebuffers_;

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

    // Material registry
    std::unordered_map<std::string, common::scene::Material> materialRegistry_;

    // All scene objects (for the previous model matrices) and the moving objects with their motion descriptions
    std::vector<std::shared_ptr<common::scene::SceneObject>> sceneObjects_;
    std::vector<std::pair<std::shared_ptr<common::scene::SceneObject>, MovingObjectDesc>> movingObjectList_;

    // Previous frame data: [0] = Previous view-projection matrix, [1 + objectId] = Previous model matrix
    std::vector<glm::mat4> previousFrameData_ = std::vector<glm::mat4>(kMaxSceneObjectCount + 1, glm::mat4(1.0f));

    // Animation time of the moving objects
    float animationTime_ = 0.0f;

    // Control values
    bool isTaaEnabled_ = true;
    bool isAnimationPaused_ = false;
    HistoryRejectionMode rejectionMode_ = HistoryRejectionMode::VARIANCE_CLIPPING;
    std::size_t currentFrameWeightIndex_ = 2U;
    float exposure_ = 1.0f;

    // TAA State: Index of the history image written in this frame (ping-pong), index in the jitter sequence
    std::uint32_t historyIndex_ = 0U;
    std::uint32_t jitterIndex_ = 0U;
    bool isHistoryValid_ = false;
    bool areHistoryImagesInitialized_ = false;

    glm::mat4 previousViewProjection_{1.0f}; // View-projection matrix of the previous frame (for depth reprojection)
    bool hasPreviousFrame_ = false;
};
} // namespace examples::post_processing_effects::anti_aliasing_techniques::temporal_anti_aliasing_variance
