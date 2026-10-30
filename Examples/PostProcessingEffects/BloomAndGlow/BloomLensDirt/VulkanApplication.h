/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    30.10.2026
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

namespace examples::post_processing_effects::bloom_and_glow::bloom_lens_dirt
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

    void CreateInitialResources();

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

    [[nodiscard]] VkExtent2D GetBloomMipExtent(std::uint32_t mipLevel) const;

    [[nodiscard]] VkFormat SelectBloomFormat() const;

    // Render Passes
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> geometryRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> lightingRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> bloomDownsampleRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> bloomUpsampleRenderPass_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> postProcessingRenderPass_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> geometryPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> lightPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> bloomPipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> compositePipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> geometryPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> downsamplePassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> upsamplePassPipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> compositePassPipeline_;

    // Framebuffers
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> geometryFramebuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> lightingFramebuffer_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> bloomDownsampleFramebuffers_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> bloomUpsampleFramebuffers_;
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
    VkFormat bloomFormat_ = VK_FORMAT_UNDEFINED;
    bool isBloomEnabled_ = true;
    float exposure_ = 1.0f;
    float bloomStrength_ = 0.15f;
    float lensDirtIntensity_ = 10.0f;
    float anamorphicStretch_ = 2.0f; // 1: Uniform bloom, >1: Horizontally stretched bloom
    std::uint32_t debugView_ = kDebugViewFinal;
};
} // namespace examples::post_processing_effects::bloom_and_glow::bloom_lens_dirt
