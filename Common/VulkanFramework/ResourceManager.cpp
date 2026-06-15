/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ResourceManager.h"

#include <cstring>

#include "TextureAsset.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanQueue.h"

namespace common::vulkan_framework
{
ResourceManager::ResourceManager(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice>& physicalDevice,
                                 const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device)
    : physicalDevice_{physicalDevice}, device_{device}
{
}

void ResourceManager::CreateBuffers(const std::vector<BufferResourceCreateInfo>& bufferCreateInfos)
{
    for (const auto& createInfo: bufferCreateInfos) {
        buffers_[createInfo.name] = std::make_unique<BufferResource>(physicalDevice_, device_);
        buffers_[createInfo.name]->CreateBuffer(createInfo);
    }
}

void ResourceManager::CreateImages(const std::vector<ImageResourceCreateInfo>& imageCreateInfos)
{
    for (const auto& createInfo: imageCreateInfos) {
        images_[createInfo.name] = std::make_unique<ImageResource>(physicalDevice_, device_);
        images_[createInfo.name]->CreateImage(createInfo);
    }
}

void ResourceManager::CreateSamplers(const std::vector<SamplerResourceCreateInfo>& samplerCreateInfos)
{
    for (const auto& createInfo: samplerCreateInfos) {
        samplers_[createInfo.name] = std::make_unique<SamplerResource>(device_);
        samplers_[createInfo.name]->CreateSampler(createInfo);
    }
}

void ResourceManager::CreateShaderModules(const ShaderModulesCreateInfo& modulesInfo)
{
    shaderResources_ = std::make_unique<ShaderResource>(device_);
    shaderResources_->CreateShaders(modulesInfo);
}

void ResourceManager::CreateDescriptorSets(const DescriptorResourceCreateInfo& descriptorSetInfo)
{
    descriptorRegistry_ = std::make_unique<DescriptorRegistry>(device_);
    descriptorRegistry_->CreateDescriptors(descriptorSetInfo);

    descriptorUpdater_ = std::make_unique<DescriptorUpdater>(device_, *descriptorRegistry_);
}

void ResourceManager::UpdateDescriptorSet(const DescriptorUpdateInfo& descriptorSetUpdateInfo) const
{
    for (const auto& bufferUpdateInfo: descriptorSetUpdateInfo.bufferWriteRequests) {
        descriptorUpdater_->AddBufferUpdate(bufferUpdateInfo);
    }

    for (const auto& imageUpdateInfo: descriptorSetUpdateInfo.imageWriteRequests) {
        descriptorUpdater_->AddImageUpdate(imageUpdateInfo);
    }

    for (const auto& texelUpdateInfo: descriptorSetUpdateInfo.texelBufferWriteRequests) {
        descriptorUpdater_->AddTexelBufferUpdate(texelUpdateInfo);
    }

    for (const auto& copyInfo: descriptorSetUpdateInfo.copySetRequests) {
        descriptorUpdater_->AddCopyRequest(copyInfo);
    }

    descriptorUpdater_->ApplyUpdates();
}

std::shared_ptr<vulkan_wrapper::VulkanBuffer> ResourceManager::GetBuffer(const std::string& bufferName) const
{
    if (!buffers_.contains(bufferName)) {
        throw std::runtime_error("Cannot get buffer \"" + bufferName + "\"");
    }

    return buffers_.at(bufferName)->GetBuffer();
}

std::shared_ptr<vulkan_wrapper::VulkanImage> ResourceManager::GetImage(const std::string& imageName) const
{
    if (!images_.contains(imageName)) {
        throw std::runtime_error("Cannot get image \"" + imageName + "\"");
    }

    return images_.at(imageName)->GetImage();
}

std::shared_ptr<vulkan_wrapper::VulkanImageView> ResourceManager::GetImageView(const std::string& imageName,
                                                                               const std::string& viewName) const
{
    if (!images_.contains(imageName)) {
        throw std::runtime_error("Cannot get image \"" + imageName + "\"");
    }

    return images_.at(imageName)->GetImageView(viewName);
}

std::shared_ptr<vulkan_wrapper::VulkanSampler> ResourceManager::GetSampler(const std::string& samplerName) const
{
    if (!samplers_.contains(samplerName)) {
        throw std::runtime_error("Cannot get sampler \"" + samplerName + "\"");
    }

    return samplers_.at(samplerName)->GetSampler();
}

std::shared_ptr<vulkan_wrapper::VulkanShaderModule>
ResourceManager::GetShaderModule(const std::string& shaderModuleName) const
{
    return shaderResources_->GetShaderModule(shaderModuleName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSetLayout>
ResourceManager::GetDescriptorLayout(const std::string& layoutName) const
{
    return descriptorRegistry_->GetDescriptorLayout(layoutName);
}

std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> ResourceManager::GetDescriptorSet(const std::string& setName) const
{
    return descriptorRegistry_->GetDescriptorSet(setName);
}

void ResourceManager::SetBuffer(const std::string& name,
                                const void* data,
                                const std::uint64_t dataSize,
                                const std::uint64_t memoryOffset,
                                const bool needUnmap)
{
    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(data, dataSize, memoryOffset);

    if (needUnmap) {
        buffers_[name]->UnmapMemory();
    }
}

void ResourceManager::SetBufferAlignedWithoutUnmap(const std::string& name,
                                                   const void* data,
                                                   const std::size_t dataUnitSize,
                                                   const std::size_t dataCount,
                                                   const std::size_t alignedSize)
{
    std::vector<std::uint8_t> temp(alignedSize * dataCount, 0);

    for (size_t i = 0; i < dataCount; ++i) {
        std::memcpy(temp.data() + i * alignedSize, static_cast<const std::uint8_t*>(data) + i * dataUnitSize,
                    dataUnitSize);
    }

    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(temp.data(), temp.size());
}

void ResourceManager::SetImageFromTexture(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                                          const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                                          const std::string& imageName,
                                          const asset_manager::TextureAsset& textureAsset,
                                          const std::uint32_t currentLayer)
{
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, currentLayer, 1});

    const BufferResourceCreateInfo stagingBufferCreateInfo{
        imageName + "_tempStagingBuffer", static_cast<std::uint32_t>(textureAsset.data.size()),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    buffers_[stagingBufferCreateInfo.name] = std::make_unique<BufferResource>(physicalDevice_, device_);
    buffers_[stagingBufferCreateInfo.name]->CreateBuffer(stagingBufferCreateInfo);

    SetBuffer(stagingBufferCreateInfo.name, textureAsset.data.data(), textureAsset.data.size());

    const VkBufferImageCopy copyRegion = {.bufferOffset = 0,
                                          .bufferRowLength = 0,
                                          .bufferImageHeight = 0,
                                          .imageSubresource =
                                                  {
                                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                      .mipLevel = 0,
                                                      .baseArrayLayer = currentLayer,
                                                      .layerCount = 1,
                                                  },
                                          .imageOffset = {0, 0, 0},
                                          .imageExtent = {textureAsset.width, textureAsset.height, 1}};
    images_[imageName]->CopyDataFromBuffer(cmdPool, queue, buffers_[stagingBufferCreateInfo.name]->GetBuffer(),
                                           {copyRegion});
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, currentLayer, 1});
}

void ResourceManager::SetImageFromTexture(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                                          const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                                          const std::string& imageName,
                                          const asset_manager::TextureAssetHDR& textureAsset,
                                          const std::uint32_t currentLayer)
{
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, currentLayer, 1});

    const std::uint32_t bufferSizeInBytes = textureAsset.data.size() * sizeof(float);

    const BufferResourceCreateInfo stagingBufferCreateInfo{
        imageName + "_tempStagingBuffer", bufferSizeInBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    buffers_[stagingBufferCreateInfo.name] = std::make_unique<BufferResource>(physicalDevice_, device_);
    buffers_[stagingBufferCreateInfo.name]->CreateBuffer(stagingBufferCreateInfo);

    SetBuffer(stagingBufferCreateInfo.name, textureAsset.data.data(), bufferSizeInBytes);

    const VkBufferImageCopy copyRegion = {.bufferOffset = 0,
                                          .bufferRowLength = 0,
                                          .bufferImageHeight = 0,
                                          .imageSubresource =
                                                  {
                                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                      .mipLevel = 0,
                                                      .baseArrayLayer = currentLayer,
                                                      .layerCount = 1,
                                                  },
                                          .imageOffset = {0, 0, 0},
                                          .imageExtent = {textureAsset.width, textureAsset.height, 1}};
    images_[imageName]->CopyDataFromBuffer(cmdPool, queue, buffers_[stagingBufferCreateInfo.name]->GetBuffer(),
                                           {copyRegion});
    images_[imageName]->ChangeImageLayout(cmdPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, currentLayer, 1});
}

void ResourceManager::GenerateMipmaps(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool>& cmdPool,
                                      const std::shared_ptr<vulkan_wrapper::VulkanQueue>& queue,
                                      const std::string& imageName,
                                      const std::uint32_t textureWidth,
                                      const std::uint32_t textureHeight,
                                      const std::uint32_t mipLevels,
                                      const std::uint32_t layerCount) const
{
    const auto cmdBufferMipmap = cmdPool->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferMipmap) {
        throw std::runtime_error("Failed to create command buffer for generate mipmaps!");
    }

    if (!cmdBufferMipmap->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    const auto image = GetImage(imageName);

    VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount};
    auto barrier = image->CreateImageMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

    subresourceRange.baseMipLevel = 1;
    subresourceRange.levelCount = mipLevels - 1;
    barrier.subresourceRange = subresourceRange;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    cmdBufferMipmap->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {barrier});

    auto mipWidth = static_cast<int32_t>(textureWidth);
    auto mipHeight = static_cast<int32_t>(textureHeight);

    for (uint32_t i = 1; i < mipLevels; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        if (i == 1) {
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // For first level only
        } else {
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }

        cmdBufferMipmap->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {barrier});

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = layerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;
        cmdBufferMipmap->BlitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {blit});

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        cmdBufferMipmap->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         {barrier});

        mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
        mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    cmdBufferMipmap->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {barrier});


    if (!cmdBufferMipmap->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    queue->Submit({cmdBufferMipmap});
    queue->WaitIdle();
}

void ResourceManager::GenerateMipmaps(const std::shared_ptr<vulkan_wrapper::VulkanCommandBuffer>& cmdBuffer,
                                      const std::string& imageName,
                                      const std::uint32_t textureWidth,
                                      const std::uint32_t textureHeight,
                                      const std::uint32_t mipLevels,
                                      const std::uint32_t layerCount) const
{
    const auto image = GetImage(imageName);

    VkImageSubresourceRange subresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount};
    auto barrier = image->CreateImageMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

    subresourceRange.baseMipLevel = 1;
    subresourceRange.levelCount = mipLevels - 1;
    barrier.subresourceRange = subresourceRange;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {barrier});

    auto mipWidth = static_cast<int32_t>(textureWidth);
    auto mipHeight = static_cast<int32_t>(textureHeight);

    for (uint32_t i = 1; i < mipLevels; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        if (i == 1) {
            barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // For first level only
        } else {
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }

        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {barrier});

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = layerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;
        cmdBuffer->BlitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             {blit});

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {barrier});

        mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
        mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    cmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {barrier});
}

void ResourceManager::DeleteBuffer(const std::string& bufferName) { buffers_.erase(bufferName); }

void ResourceManager::DeleteImage(const std::string& imageName) { images_.erase(imageName); }

void ResourceManager::DeleteSampler(const std::string& samplerName) { samplers_.erase(samplerName); }

void ResourceManager::DeleteShaderModule(const std::string& shaderModule) const
{
    shaderResources_->DeleteShaderModule(shaderModule);
}

void ResourceManager::DeleteDescriptorLayout(const std::string& layoutName) const
{
    descriptorRegistry_->DeleteDescriptorLayout(layoutName);
}

void ResourceManager::DeleteDescriptorSet(const std::string& setName) const
{
    descriptorRegistry_->DeleteDescriptorSet(setName);
}

} // namespace common::vulkan_framework
