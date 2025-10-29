/**
 * @file    ApplicationMultisampling.h
 * @brief   This file contains Vulkan application base class implementation for examples that in "Multisampling"
 *          subcategory.
 * @author  Mustafa Yemural (myemural)
 * @date    28.10.2025
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

namespace examples::fundamentals::multisampling::base
{

class ApplicationMultisampling : public common::vulkan_framework::VulkanApplicationBase
{
public:
    using VulkanApplicationBase::VulkanApplicationBase;

    ~ApplicationMultisampling() override = default;

    void SetWindow(const std::shared_ptr<common::window_wrapper::Window>& window);

protected:
    void PreUpdate() override;

    void PostUpdate() override;

    void Cleanup() noexcept override {}

    bool ShouldClose() override;

    void CreateDefaultSurface();

    void SelectDefaultPhysicalDevice();

    void CreateDefaultLogicalDevice();

    void CreateDefaultQueue();

    void CreateDefaultSwapChain();

    void CreateDefaultCommandPool();

    void CreateDefaultSyncObjects(std::uint32_t maxFramesInFlight);

    void CreateVulkanResources(const common::vulkan_framework::ResourceDescriptor& resourceCreateInfo);

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

    // Delta time related values
    double deltaTime_ = 0.0f;
    double lastFrame_ = 0.0f;
};
} // namespace examples::fundamentals::multisampling::base
