/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ImageResource.h"

#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"

namespace common::vulkan_framework
{
ImageResource::ImageResource(const std::shared_ptr<vulkan_wrapper::VulkanPhysicalDevice> &physicalDevice,
                             const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device)
    : physicalDevice_{physicalDevice}, device_{device}
{
}

void ImageResource::CreateImage(const ImageResourceCreateInfo &createInfo)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    name_ = createInfo.Name;

    image_ = devicePtr->CreateImage([&](auto &builder) {
        builder.SetCreateFlags(createInfo.CreateFlags);
        builder.SetImageType(createInfo.ImageType);
        builder.SetFormat(createInfo.Format);
        builder.SetDimensions(createInfo.Dimensions.width, createInfo.Dimensions.height, createInfo.Dimensions.depth);
        builder.SetMipLevels(createInfo.MipLevels);
        builder.SetArrayLayers(createInfo.ArrayLayers);
        builder.SetSampleCount(createInfo.Samples);
        builder.SetImageTiling(createInfo.Tiling);
        builder.SetImageUsageFlags(createInfo.UsageFlags);
        builder.SetInitialImageLayout(createInfo.InitialLayout);
    });

    if (!image_) {
        throw std::runtime_error("Failed to create image!");
    }

    AllocateImageMemory();

    for (const auto& imageViewInfos : createInfo.Views) {
        const auto imageView = devicePtr->CreateImageView(image_, [&](auto &builder) {
            builder.SetCreateFlags(imageViewInfos.CreateFlags);
            builder.SetImageViewType(imageViewInfos.ViewType);
            builder.SetFormat(imageViewInfos.Format);
            builder.SetComponents(imageViewInfos.Components);
            builder.SetSubresourceRange(imageViewInfos.SubresourceRange);
        });
        imageViews_[imageViewInfos.ViewName] = imageView;
    }

    memProps_ = createInfo.MemProperties;
}

std::shared_ptr<vulkan_wrapper::VulkanImageView> ImageResource::GetImageView(const std::string &viewName) const
{
    if (!imageViews_.contains(viewName)) {
        throw std::runtime_error("Image " + viewName + " does not exist");
    }

    return imageViews_.at(viewName);
}

void ImageResource::AllocateImageMemory()
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    const auto physicalDevicePtr = physicalDevice_.lock();
    if (!physicalDevicePtr) {
        throw std::runtime_error("Physical device object not found!");
    }

    const auto memoryReq = image_->GetImageMemoryRequirements();

    const uint32_t memoryTypeIndex = physicalDevicePtr->FindMemoryType(memoryReq.memoryTypeBits, memProps_);

    deviceMemory_ = devicePtr->AllocateMemory(memoryReq.size, memoryTypeIndex);

    if (!deviceMemory_) {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    image_->BindImageMemory(deviceMemory_, 0);
}

void ImageResource::ChangeImageLayout(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool> &cmdPool,
                                      const std::shared_ptr<vulkan_wrapper::VulkanQueue> &queue,
                                      const VkImageLayout &oldLayout, const VkImageLayout &newLayout) const
{
    const auto cmdBufferChangeLayout = cmdPool->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferChangeLayout) {
        throw std::runtime_error("Failed to create command buffer for transition image layout!");
    }

    if (!cmdBufferChangeLayout->BeginCommandBuffer([](auto &beginInfo) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        srcAccessMask = 0;
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout ==
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    const auto imageMemoryBarrier = image_->
            CreateImageMemoryBarrier(srcAccessMask, dstAccessMask, oldLayout, newLayout);

    cmdBufferChangeLayout->PipelineBarrier(srcStage, dstStage, {imageMemoryBarrier});

    if (!cmdBufferChangeLayout->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue->Submit({cmdBufferChangeLayout});
    queue->WaitIdle();
}

void ImageResource::CopyDataFromBuffer(const std::shared_ptr<vulkan_wrapper::VulkanCommandPool> &cmdPool,
                                       const std::shared_ptr<vulkan_wrapper::VulkanQueue> &queue,
                                       const std::shared_ptr<vulkan_wrapper::VulkanBuffer> &stagingBuffer,
                                       const VkBufferImageCopy &copyRegion) const
{
    const auto cmdBufferTransfer = cmdPool->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferTransfer->BeginCommandBuffer([](auto& beginInfo) {
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    cmdBufferTransfer->CopyBufferToImage(stagingBuffer, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {copyRegion});

    if (!cmdBufferTransfer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue->Submit({cmdBufferTransfer});
    queue->WaitIdle();
}
} // common::vulkan_framework