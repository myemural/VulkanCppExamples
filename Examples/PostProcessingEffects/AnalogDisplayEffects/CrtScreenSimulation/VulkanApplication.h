/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    31.10.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationAnalogDisplayEffects.h"
#include "ApplicationData.h"
#include "AssetManager.h"
#include "PerspectiveCamera.h"
#include "Scene.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::post_processing_effects::analog_display_effects::crt_screen_simulation
{
class VulkanApplication final : public base::ApplicationAnalogDisplayEffects
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

    [[nodiscard]] VkExtent2D GetVirtualImageExtent() const;

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightingRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> virtualRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> virtualClearRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> halationRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> postProcessingRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> virtualImagePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> blurPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> crtPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> virtualImagePassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> blurPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> crtPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> lightingFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> virtualFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> halationFramebufferA_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> halationFramebufferB_;
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

    // Interlacing state
    bool isVirtualImageInitialized_ = false; // First frame clears the persistent virtual image
    std::uint64_t frameCounter_ = 0; // Selects the field (even/odd lines) of the current frame

    // Control values
    bool isCrtEnabled_ = true;
    bool isInterlaced_ = false;
    float exposure_ = 1.0f;
    float curvature_ = 1.0f;
    float halationStrength_ = 0.15f;
};
} // namespace examples::post_processing_effects::analog_display_effects::crt_screen_simulation
