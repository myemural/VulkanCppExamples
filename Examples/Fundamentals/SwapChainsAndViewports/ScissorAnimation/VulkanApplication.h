/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    8.10.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationSwapChainsAndViewports.h"
#include "PerspectiveCamera.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::swap_chains_and_viewports::scissor_animation
{
class VulkanApplication final : public base::ApplicationSwapChainsAndViewports
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void Cleanup() noexcept override;

private:

    void CreateResources();

    void InitResources() const;

    void CreateSwapChain();

    void CreateRenderPass();

    void CreatePipeline();

    void CreateFramebuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanImageView>& depthImageView);

    void UpdateDescriptorSets() const;

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex, std::uint32_t indexCount);

    void CalculateAndSetMvp();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;
    MvpData mvpData[NUM_CUBES] = {glm::mat4(1.0)};

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Swap chain related
    std::shared_ptr<common::vulkan_wrapper::VulkanSwapChain> swapChain_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanImageView>> swapChainImageViews_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Framebuffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> framebuffers_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Camera
    std::unique_ptr<common::utility::PerspectiveCamera> camera;
};
} // namespace examples::fundamentals::swap_chains_and_viewports::scissor_animation
