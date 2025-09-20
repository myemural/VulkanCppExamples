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

#include "VulkanApplicationBase.h"
#include "BufferResource.h"
#include "ImageResource.h"
#include "DescriptorRegistry.h"
#include "DescriptorUpdater.h"
#include "SamplerResource.h"
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

namespace examples::fundamentals::drawing_3d::base
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

struct DescriptorSetCreateInfo
{
    struct Layout
    {
        std::string Name;
        std::vector<VkDescriptorSetLayoutBinding> Bindings;
    };

    std::uint32_t MaxSets;
    std::vector<VkDescriptorPoolSize> PoolSizes;
    std::vector<Layout> Layouts;
};

struct DescriptorSetUpdateInfo
{
    std::vector<common::vulkan_framework::BufferWriteRequest> BufferWriteRequests;
    std::vector<common::vulkan_framework::ImageWriteRequest> ImageWriteRequests;
    std::vector<common::vulkan_framework::TexelBufferWriteRequest> TexelBufferWriteRequests;
    std::vector<common::vulkan_framework::CopySetRequest> CopySetRequests;
};

class ApplicationDrawing3D : public common::vulkan_framework::VulkanApplicationBase
{
public:
    using VulkanApplicationBase::VulkanApplicationBase;

    ~ApplicationDrawing3D() override = default;

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

    void CreateDefaultFramebuffers(const std::shared_ptr<common::vulkan_wrapper::VulkanImageView> &depthImageView);

    void CreateDefaultCommandPool();

    void CreateDefaultSyncObjects(std::uint32_t maxFramesInFlight);

    void CreateBuffers(const std::vector<BufferCreateInfo> &bufferCreateInfos);

    void SetBuffer(const std::string &name, const void *data, std::uint64_t dataSize);

    void CreateImages(const std::vector<common::vulkan_framework::ImageResourceCreateInfo> &imageCreateInfos);

    void CreateSamplers(const std::vector<common::vulkan_framework::SamplerResourceCreateInfo> &samplerCreateInfos);

    void SetImageFromBuffer(const std::string &name,
                            const std::shared_ptr<common::vulkan_wrapper::VulkanBuffer> &stagingBuffer,
                            const VkExtent3D &dimensions);

    void CreateShaderModules(const ShaderModulesCreateInfo &modulesInfo);

    void CreateDescriptorSets(const DescriptorSetCreateInfo &descriptorSetInfo);

    void UpdateDescriptorSet(const DescriptorSetUpdateInfo &descriptorSetUpdateInfo) const;

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

    // All resources
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::BufferResource> > buffers_;
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::ImageResource> > images_;
    std::unordered_map<std::string, std::unique_ptr<common::vulkan_framework::SamplerResource> > samplers_;
    std::unordered_map<std::string, std::shared_ptr<common::vulkan_wrapper::VulkanShaderModule> > shaderModules_;
    std::unique_ptr<common::vulkan_framework::DescriptorRegistry> descriptorRegistry_;
    std::unique_ptr<common::vulkan_framework::DescriptorUpdater> descriptorUpdater_;
};
} // namespace examples::fundamentals::drawing_3d::base
