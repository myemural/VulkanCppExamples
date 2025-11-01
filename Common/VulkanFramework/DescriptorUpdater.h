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

#include "CoreDefines.h"
#include "DescriptorRegistry.h"

namespace common::vulkan_framework
{
struct COMMON_API BufferWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    VkDescriptorType Type; // UNIFORM_BUFFER, STORAGE_BUFFER etc.
    std::vector<VkDescriptorBufferInfo> Buffers;
    uint32_t ArrayElement = 0;
};

struct COMMON_API ImageWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    std::vector<VkDescriptorImageInfo> Images;
    VkDescriptorType Type; // COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE etc.
    uint32_t ArrayElement = 0;
};

struct COMMON_API TexelBufferWriteRequest
{
    std::string LayoutName;
    uint32_t BindingIndex;
    VkDescriptorType Type; // UNIFORM_TEXEL_BUFFER, STORAGE_TEXEL_BUFFER etc.
    std::vector<VkBufferView> BufferViews;
    uint32_t ArrayElement = 0;
};

struct COMMON_API CopySetRequest
{
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> SourceSet;
    uint32_t SourceBindingIndex;
    uint32_t SourceArrayElement;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> DestSet;
    uint32_t DestBindingIndex;
    uint32_t DestArrayElement;
    uint32_t Count;
};

struct COMMON_API DescriptorUpdateInfo
{
    std::vector<BufferWriteRequest> BufferWriteRequests;
    std::vector<ImageWriteRequest> ImageWriteRequests;
    std::vector<TexelBufferWriteRequest> TexelBufferWriteRequests;
    std::vector<CopySetRequest> CopySetRequests;
};

class COMMON_API DescriptorUpdater
{
public:
    DescriptorUpdater(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device, DescriptorRegistry& registry);

    /**
     * @brief Add a buffer write request to the update query.
     * @param request Buffer write request.
     */
    void AddBufferUpdate(const BufferWriteRequest& request) { bufferRequests_.push_back(request); }

    /**
     * @brief Add a image write request to the update query.
     * @param request Image write request.
     */
    void AddImageUpdate(const ImageWriteRequest& request) { imageRequests_.push_back(request); }

    /**
     * @brief Add a texel buffer write request to the update query.
     * @param request Texel buffer write request.
     */
    void AddTexelBufferUpdate(const TexelBufferWriteRequest& request) { texelRequests_.push_back(request); }

    /**
     * @brief Add a copy request to the update query.
     * @param request Copy request.
     */
    void AddCopyRequest(const CopySetRequest& request) { copyRequests_.push_back(request); }

    /**
     * @brief Applies all requests at once.
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
} // namespace common::vulkan_framework
