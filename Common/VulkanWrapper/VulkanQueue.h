/**
 * @file    VulkanQueue.h
 * @brief   This file contains wrapper class implementation for VkQueue.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanSwapChain;
class VulkanDevice;
class VulkanCommandBuffer;
class VulkanFence;
class VulkanSemaphore;

class VulkanQueue final : public VulkanObject<VulkanDevice, VkQueue>
{
public:
    VulkanQueue(std::shared_ptr<VulkanDevice> device, VkQueue queue);

    ~VulkanQueue() override = default;

    void Submit(const std::vector<std::shared_ptr<VulkanCommandBuffer> > &cmdBuffers,
                const std::vector<std::shared_ptr<VulkanSemaphore> > &waitSemaphores = {},
                const std::vector<std::shared_ptr<VulkanSemaphore> > &signalSemaphores = {},
                const std::shared_ptr<VulkanFence> &fence = VK_NULL_HANDLE,
                const std::vector<VkPipelineStageFlags> &waitStages = {}) const;

    void Present(const std::vector<std::shared_ptr<VulkanSwapChain> > &swapChains,
                 const std::vector<std::uint32_t> &swapChainImageIndices,
                 const std::vector<std::shared_ptr<VulkanSemaphore> > &waitSemaphores) const;

    void WaitIdle() const;
};
} // common::vulkan_wrapper
