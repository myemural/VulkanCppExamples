/**
 * @file    DescriptorUpdater.h
 * @brief   This class contains creating descriptor set write or copy requests and apply all it at once.
 * @author  Mustafa Yemural (myemural)
 * @date    30.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "DescriptorRegistry.h"

namespace common::vulkan_framework
{
struct BufferWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    VkDescriptorType Type; // UNIFORM_BUFFER, STORAGE_BUFFER etc.
    std::vector<VkDescriptorBufferInfo> Buffers;
    uint32_t ArrayElement = 0;
};

struct ImageWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    std::vector<VkDescriptorImageInfo> Images;
    VkDescriptorType Type; // COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE etc.
    uint32_t ArrayElement = 0;
};

struct TexelBufferWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    VkDescriptorType Type; // UNIFORM_TEXEL_BUFFER, STORAGE_TEXEL_BUFFER etc.
    std::vector<VkBufferView> BufferViews;
    uint32_t ArrayElement = 0;
};

struct CopySetRequest
{
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> SourceSet;
    uint32_t SourceBindingIndex;
    uint32_t SourceArrayElement;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> DestSet;
    uint32_t DestBindingIndex;
    uint32_t DestArrayElement;
    uint32_t Count;
};

class DescriptorUpdater
{
public:
    DescriptorUpdater(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device, DescriptorRegistry& registry);

    void AddBufferUpdate(const BufferWriteRequest& request) { bufferRequests_.push_back(request); }
    void AddImageUpdate(const ImageWriteRequest& request) { imageRequests_.push_back(request); }
    void AddTexelBufferUpdate(const TexelBufferWriteRequest& request) { texelRequests_.push_back(request); }
    void AddCopyRequest(const CopySetRequest& request) { copyRequests_.push_back(request); }

    /**
     * @brief Apply all requests at once
     */
    void ApplyUpdates();

private:
    std::shared_ptr<vulkan_wrapper::VulkanDevice> device_;
    DescriptorRegistry& registry_;
    std::vector<BufferWriteRequest> bufferRequests_;
    std::vector<ImageWriteRequest> imageRequests_;
    std::vector<TexelBufferWriteRequest> texelRequests_;
    std::vector<CopySetRequest> copyRequests_;
};

} // common::vulkan_framework
