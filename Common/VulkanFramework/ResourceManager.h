/**
 * @file    ResourceManager.h
 * @brief   This file contains the implementation of the ResourceManager class that manages all Vulkan resources in one
 *          place.
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
#include "TextureHandler.h"
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
    /**
     * @param physicalDevice Refers VulkanPhysicalDevice object.
     * @param device Refers VulkanDevice object.
     */
    ResourceManager(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                    const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Creates buffers.
     * @param bufferCreateInfos Create information for buffers.
     */
    void CreateBuffers(const std::vector<BufferResourceCreateInfo>& bufferCreateInfos);

    /**
     * @brief Creates images.
     * @param imageCreateInfos Create information for images.
     */
    void CreateImages(const std::vector<ImageResourceCreateInfo>& imageCreateInfos);

    /**
     * @brief Creates samplers.
     * @param samplerCreateInfos Create information for samplers.
     */
    void CreateSamplers(const std::vector<SamplerResourceCreateInfo>& samplerCreateInfos);

    /**
     * @brief Creates shader modules.
     * @param modulesInfo Create information for shader modules.
     */
    void CreateShaderModules(const ShaderModulesCreateInfo& modulesInfo);

    /**
     * @brief Creates descriptor sets.
     * @param descriptorSetInfo Create information for descriptor sets.
     */
    void CreateDescriptorSets(const DescriptorResourceCreateInfo& descriptorSetInfo);

    /**
     * @brief Updates existing descriptor sets.
     * @param descriptorSetUpdateInfo Update information for descriptor sets.
     */
    void UpdateDescriptorSet(const DescriptorUpdateInfo& descriptorSetUpdateInfo) const;

    /**
     * @brief Returns the buffer resource.
     * @param bufferName Name of the buffer resource.
     * @return Returns the buffer resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanBuffer> GetBuffer(const std::string& bufferName) const;

    /**
     * @brief Returns the image resource.
     * @param imageName Name of the image resource.
     * @return Returns the image resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImage> GetImage(const std::string& imageName) const;

    /**
     * @brief Returns the image view resource.
     * @param imageName Name of the image resource.
     * @param viewName Name of the image view resource.
     * @return Returns the image view resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanImageView> GetImageView(const std::string& imageName,
                                                                                const std::string& viewName) const;

    /**
     * @brief Returns the sampler resource.
     * @param samplerName Name of the sampler resource.
     * @return Returns the sampler resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanSampler> GetSampler(const std::string& samplerName) const;

    /**
     * @brief Returns the shader module resource.
     * @param shaderModuleName Name of the shader module resource.
     * @return Returns the shader module resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanShaderModule>
    GetShaderModule(const std::string& shaderModuleName) const;

    /**
     * @brief Returns the descriptor set layout resource.
     * @param layoutName Name of the descriptor set layout resource.
     * @return Returns the descriptor set layout resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>
    GetDescriptorLayout(const std::string& layoutName) const;

    /**
     * @brief Returns the descriptor set resource.
     * @param setName Name of the descriptor set resource.
     * @return Returns the descriptor set resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet>
    GetDescriptorSet(const std::string& setName) const;

    /**
     * @brief Sets a buffer resource with raw data.
     * @param name Name of the buffer resource.
     * @param data Data to be copied to buffer.
     * @param dataSize Size of the data to be copied to buffer.
     */
    void SetBuffer(const std::string& name, const void* data, std::uint64_t dataSize);

    /**
     * @brief Sets an image resource with texture data.
     * @param cmdPool Command pool that the command buffer will be created.
     * @param queue Queue that the command buffer will be sent.
     * @param imageName Name of the image resource to be updated.
     * @param textureHandler Handler of the texture resource.
     */
    void SetImageFromTexture(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                          const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                          const std::string& imageName,
                          const utility::TextureHandler& textureHandler);

    /**
     * @brief Deletes image resource from resource manager.
     * @param name Name of the image resource.
     */
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
