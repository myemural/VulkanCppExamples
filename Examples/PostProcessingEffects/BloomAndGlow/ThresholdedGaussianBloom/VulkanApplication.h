/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    28.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBloomAndGlow.h"
#include "ApplicationData.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::post_processing_effects::bloom_and_glow::thresholded_gaussian_bloom
{
class VulkanApplication final : public base::ApplicationBloomAndGlow
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

    void RecordFullscreenPass(const std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer,
                              const std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass>& renderPass,
                              const std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>& framebuffer,
                              const VkExtent2D& extent,
                              const std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout>& pipelineLayout,
                              const std::shared_ptr<common::vulkan_wrapper::VulkanPipeline>& pipeline,
                              const std::string& descriptorSetName,
                              const void* pushConstantData,
                              std::uint32_t pushConstantSize) const;

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    void ProcessInput() const;

    [[nodiscard]] VkExtent2D GetBloomExtent() const;

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightingRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> bloomRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> postProcessingRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> brightPassPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> blurPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> compositePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> brightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> blurPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> compositePassPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> lightingFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> bloomFramebufferA_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> bloomFramebufferB_;
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

    // Light related values
    std::unordered_map<std::uint32_t, std::int32_t> lightSphereIndices_;

    // Control values
    bool isBloomEnabled_ = true;
    float exposure_ = 1.0f;
    float bloomThreshold_ = 1.0f;
    float bloomSoftKnee_ = 0.5f;
    float bloomIntensity_ = 0.5f;
    float blurSigma_ = 3.0f;
    std::uint32_t blurIterationCount_ = 2U;
    std::uint32_t brightPassSource_ = kBrightPassSourceThreshold;
    std::uint32_t debugView_ = kDebugViewFinal;
};
} // namespace examples::post_processing_effects::bloom_and_glow::thresholded_gaussian_bloom
