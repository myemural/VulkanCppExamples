/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ResourceManager.h"

namespace common::vulkan_framework
{
ResourceManager::ResourceManager(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice> &physicalDevice,
                                 const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device)
    : physicalDevice_{physicalDevice}, device_{device}
{
}

void ResourceManager::CreateBuffers(const std::vector<BufferResourceCreateInfo> &bufferCreateInfos)
{
    for (const auto &createInfo: bufferCreateInfos) {
        buffers_[createInfo.Name] = std::make_unique<BufferResource>(physicalDevice_, device_);
        buffers_[createInfo.Name]->CreateBuffer(createInfo);
    }
}

void ResourceManager::CreateImages(const std::vector<ImageResourceCreateInfo> &imageCreateInfos)
{
    for (const auto &createInfo: imageCreateInfos) {
        images_[createInfo.Name] = std::make_unique<ImageResource>(physicalDevice_, device_);
        images_[createInfo.Name]->CreateImage(createInfo);
    }
}

void ResourceManager::CreateSamplers(const std::vector<SamplerResourceCreateInfo> &samplerCreateInfos)
{
    for (const auto &createInfo: samplerCreateInfos) {
        samplers_[createInfo.Name] = std::make_unique<SamplerResource>(device_);
        samplers_[createInfo.Name]->CreateSampler(createInfo);
    }
}

void ResourceManager::CreateShaderModules(const ShaderModulesCreateInfo &modulesInfo)
{
    shaderResources_ = std::make_unique<ShaderResource>(device_);
    shaderResources_->CreateShaders(modulesInfo);
}

void ResourceManager::CreateDescriptorSets(const DescriptorResourceCreateInfo &descriptorSetInfo)
{
    descriptorRegistry_ = std::make_unique<DescriptorRegistry>(device_);
    descriptorRegistry_->CreateDescriptors(descriptorSetInfo);

    descriptorUpdater_ = std::make_unique<DescriptorUpdater>(device_, *descriptorRegistry_);
}

void ResourceManager::UpdateDescriptorSet(const DescriptorUpdateInfo &descriptorSetUpdateInfo) const
{
    for (const auto &bufferUpdateInfo: descriptorSetUpdateInfo.BufferWriteRequests) {
        descriptorUpdater_->AddBufferUpdate(bufferUpdateInfo);
    }

    for (const auto &imageUpdateInfo: descriptorSetUpdateInfo.ImageWriteRequests) {
        descriptorUpdater_->AddImageUpdate(imageUpdateInfo);
    }

    for (const auto &texelUpdateInfo: descriptorSetUpdateInfo.TexelBufferWriteRequests) {
        descriptorUpdater_->AddTexelBufferUpdate(texelUpdateInfo);
    }

    for (const auto &copyInfo: descriptorSetUpdateInfo.CopySetRequests) {
        descriptorUpdater_->AddCopyRequest(copyInfo);
    }

    descriptorUpdater_->ApplyUpdates();
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> ResourceManager::GetBuffer(const std::string &bufferName) const
{
    return buffers_.at(bufferName)->GetBuffer();
}

std::shared_ptr<vulkan_wrapper::VulkanImage> ResourceManager::GetImage(const std::string &imageName) const
{
    return images_.at(imageName)->GetImage();
}

std::shared_ptr<vulkan_wrapper::VulkanImageView> ResourceManager::GetImageView(const std::string &imageName,
                                                                               const std::string &viewName) const
{
    return images_.at(imageName)->GetImageView(viewName);
}

std::shared_ptr<vulkan_wrapper::VulkanSampler> ResourceManager::GetSampler(const std::string &samplerName) const
{
    return samplers_.at(samplerName)->GetSampler();
}

std::shared_ptr<vulkan_wrapper::VulkanShaderModule> ResourceManager::GetShaderModule(
    const std::string &shaderModuleName) const
{
    return shaderResources_->GetShaderModule(shaderModuleName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout> ResourceManager::GetDescriptorLayout(
    const std::string &layoutName) const
{
    return descriptorRegistry_->GetDescriptorLayout(layoutName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> ResourceManager::GetDescriptorSet(const std::string &setName) const
{
    return descriptorRegistry_->GetDescriptorSet(setName);
}

void ResourceManager::SetBuffer(const std::string &name, const void *data, const std::uint64_t dataSize)
{
    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(data, dataSize);
    buffers_[name]->UnmapMemory();
}

void ResourceManager::SetImageFromBuffer(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool> &cmdPool,
                                         const std::shared_ptr<vulkan_wrapper::VulkanQueue> &queue,
                                         const std::string &imageName,
                                         const std::shared_ptr<vulkan_wrapper::VulkanBuffer> &stagingBuffer,
                                         const VkExtent3D &dimensions)
{
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /// TODO: It is constant for now
    const VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = dimensions
    };
    images_[imageName]->CopyDataFromBuffer(cmdPool, queue, stagingBuffer, copyRegion);
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
} // namespace common::vulkan_framework
