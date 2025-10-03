/**
 * @file    ApplicationDrawing3D.h
 * @brief   This file contains Vulkan application base class implementation for examples that in "Drawing3D"
 *          subcategory.
 * @author  Mustafa Yemural (myemural)
 * @date    08.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <vector>

#include "BufferResource.h"
#include "DescriptorRegistry.h"
#include "DescriptorUpdater.h"
#include "ImageResource.h"
#include "SamplerResource.h"
#include "ShaderResource.h"
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

namespace examples::fundamentals::drawing_3d::base
{

class ApplicationDrawing3D : public common::vulkan_framework::VulkanApplicationBase
{
public:
    using VulkanApplicationBase::VulkanApplicationBase;

    ~ApplicationDrawing3D() override = default;

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

    void CreateDefaultFramebuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanImageView>& depthImageView);

    void CreateDefaultCommandPool();

    void CreateDefaultSyncObjects(std::uint32_t maxFramesInFlight);

    void CreateBuffers(const std::vector<common::vulkan_framework::BufferResourceCreateInfo>& bufferCreateInfos);

    void SetBuffer(const std::string& name, const void* data, std::uint64_t dataSize);

    void CreateImages(const std::vector<common::vulkan_framework::ImageResourceCreateInfo>& imageCreateInfos);

    void CreateSamplers(const std::vector<common::vulkan_framework::SamplerResourceCreateInfo>& samplerCreateInfos);

    void SetImageFromBuffer(const std::string& name,
                            const std::shared_ptr<common::vulkan_wrapper::VulkanBuffer>& stagingBuffer,
                            const VkExtent3D& dimensions);

    void CreateShaderModules(const common::vulkan_framework::ShaderModulesCreateInfo& modulesInfo);

    void CreateDescriptorSets(const common::vulkan_framework::DescriptorResourceCreateInfo& descriptorSetInfo);

    void UpdateDescriptorSet(const common::vulkan_framework::DescriptorUpdateInfo& descriptorSetUpdateInfo) const;

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

    // All resources
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::BufferResource>> buffers_;
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::ImageResource>> images_;
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::SamplerResource>> samplers_;
    std::unique_ptr<common::vulkan_framework::ShaderResource> shaderResources_;
    std::unique_ptr<common::vulkan_framework::DescriptorRegistry> descriptorRegistry_;
    std::unique_ptr<common::vulkan_framework::DescriptorUpdater> descriptorUpdater_;
};
} // namespace examples::fundamentals::drawing_3d::base
