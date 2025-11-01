/**
 * @file    VulkanDevice.h
 * @brief   This file contains wrapper class implementation for VkDevice.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <functional>
#include <optional>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanBuffer;
class VulkanBufferBuilder;
class VulkanCommandPool;
class VulkanDescriptorPool;
class VulkanDescriptorSetLayout;
class VulkanDeviceMemory;
class VulkanFence;
class VulkanFramebuffer;
class VulkanFramebufferBuilder;
class VulkanGraphicsPipelineBuilder;
class VulkanImage;
class VulkanImageBuilder;
class VulkanImageView;
class VulkanImageViewBuilder;
class VulkanPhysicalDevice;
class VulkanPipeline;
class VulkanPipelineLayout;
class VulkanQueue;
class VulkanRenderPass;
class VulkanRenderPassBuilder;
class VulkanSampler;
class VulkanSamplerBuilder;
class VulkanSemaphore;
class VulkanShaderModule;
class VulkanSurface;
class VulkanSwapChain;
class VulkanSwapChainBuilder;

class VulkanDevice final : public VulkanObject<VulkanPhysicalDevice, VkDevice>,
                           public std::enable_shared_from_this<VulkanDevice>
{
public:
    COMMON_API VulkanDevice(std::shared_ptr<VulkanPhysicalDevice> physicalDevice, VkDevice device);

    COMMON_API ~VulkanDevice() override;

    COMMON_API std::shared_ptr<VulkanQueue> CreateQueue(std::uint32_t queueFamilyIndex, std::uint32_t queueIndex);

    COMMON_API std::shared_ptr<VulkanSemaphore> CreateSemaphore();

    COMMON_API std::shared_ptr<VulkanFence> CreateFence(const VkFenceCreateFlags& flags);

    COMMON_API std::shared_ptr<VulkanCommandPool> CreateCommandPool(std::uint32_t queueFamilyIndex,
                                                         const VkCommandPoolCreateFlags& flags = 0);

    COMMON_API std::shared_ptr<VulkanDescriptorPool> CreateDescriptorPool(std::uint32_t maxSets,
                                                               const std::vector<VkDescriptorPoolSize>& poolSizes,
                                                               const VkDescriptorPoolCreateFlags& flags = 0);

    COMMON_API std::shared_ptr<VulkanDescriptorSetLayout>
    CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                              const VkDescriptorSetLayoutCreateFlags& flags = 0);

    COMMON_API std::shared_ptr<VulkanSwapChain> CreateSwapChain(const std::shared_ptr<VulkanSurface>& surface,
                                                     const std::function<void(VulkanSwapChainBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanRenderPass>
    CreateRenderPass(const std::function<void(VulkanRenderPassBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanFramebuffer>
    CreateFramebuffer(const std::shared_ptr<VulkanRenderPass>& renderPass,
                      const std::vector<std::shared_ptr<VulkanImageView>>& attachments,
                      const std::function<void(VulkanFramebufferBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanImage> CreateImage(const std::function<void(VulkanImageBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanImageView> CreateImageView(const std::shared_ptr<VulkanImage>& image,
                                                     const std::function<void(VulkanImageViewBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanShaderModule> CreateShaderModule(const std::vector<std::uint32_t>& moduleCode);

    COMMON_API std::shared_ptr<VulkanPipelineLayout>
    CreatePipelineLayout(const std::vector<std::shared_ptr<VulkanDescriptorSetLayout>>& descSetLayouts = {},
                         const std::vector<VkPushConstantRange>& pushConstantRanges = {},
                         const VkPipelineLayoutCreateFlags& createFlags = 0);

    COMMON_API std::shared_ptr<VulkanPipeline>
    CreateGraphicsPipeline(const std::shared_ptr<VulkanPipelineLayout>& layout,
                           const std::shared_ptr<VulkanRenderPass>& renderPass,
                           const std::function<void(VulkanGraphicsPipelineBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanBuffer> CreateBuffer(const std::function<void(VulkanBufferBuilder&)>& builderFunc);

    COMMON_API std::shared_ptr<VulkanDeviceMemory> AllocateMemory(const VkDeviceSize& size, std::uint32_t memoryTypeIndex);

    COMMON_API void UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writeDescriptorSets,
                              const std::vector<VkCopyDescriptorSet>& copyDescriptorSets = {}) const;

    COMMON_API std::shared_ptr<VulkanSampler> CreateSampler(const std::function<void(VulkanSamplerBuilder&)>& builderFunc);

    COMMON_API void WaitIdle() const;
};

class COMMON_API VulkanDeviceBuilder
{
public:
    VulkanDeviceBuilder();

    VulkanDeviceBuilder& AddQueueInfo(const std::function<void(VkDeviceQueueCreateInfo&)>& setterFunc);

    VulkanDeviceBuilder& AddLayer(const std::string& layerName);

    VulkanDeviceBuilder& AddLayers(const std::vector<std::string>& layerNames);

    VulkanDeviceBuilder& AddExtension(const std::string& extensionName);

    VulkanDeviceBuilder& AddExtensions(const std::vector<std::string>& extensionNames);

    VulkanDeviceBuilder& SetDeviceFeatures(const VkPhysicalDeviceFeatures& features);

    std::shared_ptr<VulkanDevice> Build(const std::shared_ptr<VulkanPhysicalDevice>& physicalDevice);

private:
    VkDeviceCreateInfo createInfo{};
    std::vector<std::string> layers_;
    std::vector<const char*> layersStr_;
    std::vector<std::string> extensions_;
    std::vector<const char*> extensionsStr_;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos_;
    std::optional<VkPhysicalDeviceFeatures> deviceFeatures_;
};
} // namespace common::vulkan_wrapper
