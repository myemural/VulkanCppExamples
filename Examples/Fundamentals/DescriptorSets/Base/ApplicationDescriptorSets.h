/**
 * @file    ApplicationDescriptorSets.h
 * @brief   This file contains Vulkan application base class implementation for examples that in "DescriptorSets"
 *          subcategory.
 * @author  Mustafa Yemural (myemural)
 * @date    25.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <vector>

#include "VulkanApplicationBase.h"
#include "BufferResource.h"
#include "ShaderLoader.h"
#include "VulkanCommandPool.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFramebuffer.h"
#include "VulkanQueue.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapChain.h"
#include "Window.h"

namespace examples::fundamentals::descriptor_sets::base
{
struct BufferCreateInfo
{
    std::string Name;
    std::uint32_t BufferSizeInBytes;
    VkBufferUsageFlags UsageFlags;
    VkMemoryPropertyFlags MemoryProperties;
};

struct ShaderModulesCreateInfo
{
    struct Module
    {
        std::string Name;
        std::string FileName;
    };

    std::string BasePath;
    common::utility::ShaderBaseType ShaderType;
    std::vector<Module> Modules;
};

class ApplicationDescriptorSets : public common::vulkan_framework::VulkanApplicationBase
{
public:
    using VulkanApplicationBase::VulkanApplicationBase;

    void SetWindow(const std::shared_ptr<common::window_wrapper::Window> &window);

protected:
    void Update() override;

    void Cleanup() noexcept override
    {
    }

    bool ShouldClose() override;

    void CreateDefaultSurface();

    void SelectDefaultPhysicalDevice();

    void CreateDefaultLogicalDevice();

    void CreateDefaultQueue();

    void CreateDefaultSwapChain();

    void CreateDefaultRenderPass();

    void CreateDefaultFramebuffers();

    void CreateDefaultCommandPool();

    void CreateDefaultSyncObjects(std::uint32_t maxFramesInFlight);

    void CreateBuffers(const std::vector<BufferCreateInfo> &bufferCreateInfos);

    void SetBuffer(const std::string &name, const void *data, std::uint64_t dataSize);

    void CreateShaderModules(const ShaderModulesCreateInfo &modulesInfo);

    std::shared_ptr<common::window_wrapper::Window> window_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSurface> surface_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::uint32_t currentQueueFamilyIndex_ = UINT32_MAX;
    std::shared_ptr<common::vulkan_wrapper::VulkanDevice> device_;
    std::shared_ptr<common::vulkan_wrapper::VulkanQueue> queue_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSwapChain> swapChain_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanImageView> > swapChainImageViews_;
    std::shared_ptr<common::vulkan_wrapper::VulkanRenderPass> renderPass_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFramebuffer> > framebuffers_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandPool> cmdPool_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanSemaphore> > imageAvailableSemaphores_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanSemaphore> > renderFinishedSemaphores_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFence> > inFlightFences_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanFence> > swapImagesFences_;

    // Resources
    std::unordered_map<std::string, std::shared_ptr<common::vulkan_wrapper::VulkanShaderModule> > shaderModules_;
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::BufferResource> > buffers_;
};
} // namespace examples::fundamentals::descriptor_sets::base
