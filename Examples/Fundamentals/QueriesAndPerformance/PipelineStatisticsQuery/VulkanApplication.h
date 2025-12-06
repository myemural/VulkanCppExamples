/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    22.11.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <array>
#include <memory>

#include "ApplicationData.h"
#include "ApplicationQueriesAndPerformance.h"
#include "PerspectiveCamera.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::queries_and_performance::pipeline_statistics_query
{
class VulkanApplication final : public base::ApplicationQueriesAndPerformance
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

    void CreateResources();

    void InitResources() const;

    void CreateRenderPass();

    void CreatePipeline();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CreateQueryPools();

    void PrintQueryResults();

    void CalculateAndSetMvp();

    void ProcessInput() const;

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    std::array<MvpData, NUM_CUBES> cubeMvp_ = {glm::mat4(1.0)};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline1_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline2_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline3_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline4_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Query pools
    std::shared_ptr<common::vulkan_wrapper::VulkanQueryPool> pipelineStatQueryPool_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera_;

    // For control output occurrence
    unsigned long long frameCount_ = 0;
};
} // namespace examples::fundamentals::queries_and_performance::pipeline_statistics_query
