/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    27.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationPipelinesAndPasses.h"
#include "PerspectiveCamera.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::pipelines_and_passes::multiple_subpasses
{
class VulkanApplication final : public base::ApplicationPipelinesAndPasses
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void CreateResources();

    void InitResources() const;

    void CreateRenderPass();

    void CreatePipeline();

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void CalculateAndSetMvp();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    MvpData mvpData_[NUM_CUBES] = {glm::mat4(1.0)};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutObject_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayoutDepthObject_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipelineObject_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipelineDepthObject_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera_;
};
} // namespace examples::fundamentals::pipelines_and_passes::multiple_subpasses
