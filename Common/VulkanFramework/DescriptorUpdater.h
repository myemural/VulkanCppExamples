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
    std::string DescriptorSetName;
    std::uint32_t BindingIndex = UINT32_MAX;
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM; // UNIFORM_BUFFER, STORAGE_BUFFER etc.
    std::vector<VkDescriptorBufferInfo> Buffers;
    std::uint32_t ArrayElement = 0;
};

struct COMMON_API ImageWriteRequest
{
    std::string DescriptorSetName;
    std::uint32_t BindingIndex = UINT32_MAX;
    std::vector<VkDescriptorImageInfo> Images;
    VkDescriptorType Type  = VK_DESCRIPTOR_TYPE_MAX_ENUM; // COMBINED_IMAGE_SAMPLER, SAMPLED_IMAGE etc.
    std::uint32_t ArrayElement = 0;
};

struct COMMON_API TexelBufferWriteRequest
{
    std::string DescriptorSetName;
    std::uint32_t BindingIndex = UINT32_MAX;
    VkDescriptorType Type  = VK_DESCRIPTOR_TYPE_MAX_ENUM; // UNIFORM_TEXEL_BUFFER, STORAGE_TEXEL_BUFFER etc.
    std::vector<VkBufferView> BufferViews;
    std::uint32_t ArrayElement = 0;
};

struct COMMON_API CopySetRequest
{
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> SourceSet;
    std::uint32_t SourceBindingIndex = UINT32_MAX;
    std::uint32_t SourceArrayElement = UINT32_MAX;
    std::shared_ptr<vulkan_wrapper::VulkanDescriptorSet> DestSet;
    std::uint32_t DestBindingIndex = UINT32_MAX;
    std::uint32_t DestArrayElement = UINT32_MAX;
    std::uint32_t Count = UINT32_MAX;
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
    /**
     * @param device Refers VulkanDevice object.
     * @param registry Descriptor registry object that will be updated.
     */
    DescriptorUpdater(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device, DescriptorRegistry& registry);

    /**
     * @brief Add a buffer write request to the update query.
     * @param request Buffer write request.
     */
    void AddBufferUpdate(const BufferWriteRequest& request) { bufferRequests_.push_back(request); }

    /**
     * @brief Add an image write request to the update query.
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
