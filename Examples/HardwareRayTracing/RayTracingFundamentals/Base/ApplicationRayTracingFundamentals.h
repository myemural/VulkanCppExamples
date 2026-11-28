/**
 * @file    ApplicationRayTracingFundamentals.h
 * @brief   This file contains Vulkan application base class implementation for examples that in
 *          "RayTracingFundamentals" subcategory.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <vector>

#include "ImageResource.h"
#include "ResourceManager.h"
#include "VulkanApplicationBase.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFramebuffer.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQueue.h"
#include "VulkanRenderPass.h"
#include "VulkanSurface.h"
#include "VulkanSwapChain.h"
#include "Window.h"

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::base
{

class ApplicationRayTracingFundamentals : public common::vulkan_framework::VulkanApplicationBase
{
public:
    using VulkanApplicationBase::VulkanApplicationBase;

    ~ApplicationRayTracingFundamentals() override = default;

    void SetWindow(const std::shared_ptr<common::window_wrapper::Window>& window);

protected:
    bool Init() override;

    void PreUpdate() override;

    void PostUpdate() override;

    void Cleanup() noexcept override {}

    bool ShouldClose() override;

    void CreateDefaultSurface();

    void SelectDefaultPhysicalDevice();

    void QueryRayTracingProperties();

    void CreateDefaultLogicalDevice();

    void CreateDefaultQueue();

    void CreateDefaultSwapChain();

    void CreateDefaultFramebuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanImageView>& depthImageView);

    void CreateDefaultCommandPool();

    void CreateDefaultSyncObjects();

    void CreateVulkanResources(const common::vulkan_framework::ResourceDescriptor& resourceCreateInfo) const;

    void ExecuteOneTimeCommands(
            const std::function<void(const std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>&)>& recordFunc)
            const;

    // Device extensions that are required for all examples in this subcategory
    inline static const std::vector<std::string> kRequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};

    std::shared_ptr<common::window_wrapper::Window> window_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSurface> surface_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::uint32_t currentQueueFamilyIndex_ = UINT32_MAX;
    std::shared_ptr<common::vulkan_wrapper::VulkanDevice> device_;
    std::shared_ptr<common::vulkan_wrapper::VulkanQueue> queue_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSwapChain> swapChain_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanImageView>> swapChainImageViews_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> renderPass_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer>> framebuffers_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandPool> cmdPool_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanSemaphore>> imageAvailableSemaphores_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanSemaphore>> renderFinishedSemaphores_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFence>> inFlightFences_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFence>> swapImagesFences_;

    std::unique_ptr<common::vulkan_framework::ResourceManager> resources_;

    // Ray tracing related device properties (SBT alignments, recursion depth, AS limits etc.)
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties_{};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties_{};

    // Window-related values
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    VkFormat depthImageFormat_ = VK_FORMAT_UNDEFINED;

    // Delta time related values
    double deltaTime_ = 0.0f;
    double lastFrame_ = 0.0f;
    double fpsTimer_ = 0.0;
    uint32_t frameCounter_ = 0;

    // Current frame index
    std::uint32_t currentFrameIndex_ = 0;
};
} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::base
