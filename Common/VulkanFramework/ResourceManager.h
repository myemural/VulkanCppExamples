/**
 * @file    ResourceManager.h
 * @brief   TODO: Write brief
 * @author  Mustafa Yemural (myemural)
 * @date    23.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "BufferResource.h"
#include "DescriptorRegistry.h"
#include "DescriptorUpdater.h"
#include "ImageResource.h"
#include "SamplerResource.h"
#include "ShaderResource.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"

namespace common::vulkan_framework
{
struct ResourceDescriptor
{
    std::optional<std::vector<BufferResourceCreateInfo>> Buffers;
    std::optional<std::vector<ImageResourceCreateInfo>> Images;
    std::optional<std::vector<SamplerResourceCreateInfo>> Samplers;
    std::optional<ShaderModulesCreateInfo> Shaders;
    std::optional<DescriptorResourceCreateInfo> Descriptors;
};

class ResourceManager
{
public:
    ResourceManager(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                    const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    void CreateBuffers(const std::vector<BufferResourceCreateInfo>& bufferCreateInfos);

    void CreateImages(const std::vector<ImageResourceCreateInfo>& imageCreateInfos);

    void CreateSamplers(const std::vector<SamplerResourceCreateInfo>& samplerCreateInfos);

    void CreateShaderModules(const ShaderModulesCreateInfo& modulesInfo);

    void CreateDescriptorSets(const DescriptorResourceCreateInfo& descriptorSetInfo);

    void UpdateDescriptorSet(const DescriptorUpdateInfo& descriptorSetUpdateInfo) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetBuffer(const std::string& bufferName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImage> GetImage(const std::string& imageName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImageView> GetImageView(const std::string& imageName,
                                                                                const std::string& viewName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanSampler> GetSampler(const std::string& samplerName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanShaderModule>
    GetShaderModule(const std::string& shaderModuleName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>
    GetDescriptorLayout(const std::string& layoutName) const;

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet>
    GetDescriptorSet(const std::string& setName) const;

    void SetBuffer(const std::string& name, const void* data, std::uint64_t dataSize);

    void SetImageFromBuffer(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                            const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                            const std::string& imageName,
                            const std::shared_ptr<vulkan_wrapper::VulkanBuffer>& stagingBuffer,
                            const VkExtent3D& dimensions);

    void DeleteImage(const std::string& name);

private:
    std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice> physicalDevice_;
    std::shared_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::unordered_map<std::string, std::unique_ptr<BufferResource>> buffers_;
    std::unordered_map<std::string, std::unique_ptr<ImageResource>> images_;
    std::unordered_map<std::string, std::unique_ptr<SamplerResource>> samplers_;
    std::unique_ptr<ShaderResource> shaderResources_;
    std::unique_ptr<DescriptorRegistry> descriptorRegistry_;
    std::unique_ptr<DescriptorUpdater> descriptorUpdater_;
};
} // namespace common::vulkan_framework
