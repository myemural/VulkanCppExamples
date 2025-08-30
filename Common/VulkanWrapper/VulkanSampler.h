/**
 * @file    VulkanSampler.h
 * @brief   This file contains wrapper class implementation for VkSampler.
 * @author  Mustafa Yemural (myemural)
 * @date    29.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "VulkanObject.h"

namespace common::vulkan_wrapper
{

class VulkanDevice;

class VulkanSampler final : public VulkanObject<VulkanDevice, VkSampler>
{
public:
    explicit VulkanSampler(std::shared_ptr<VulkanDevice> device, VkSampler sampler);

    ~VulkanSampler() override;
};

class VulkanSamplerBuilder
{
public:
    VulkanSamplerBuilder();

    VulkanSamplerBuilder &SetCreateFlags(const VkSamplerCreateFlags &flags);

    VulkanSamplerBuilder &SetFilters(const VkFilter &magFilter, const VkFilter &minFilter);

    VulkanSamplerBuilder &SetMipmapMode(const VkSamplerMipmapMode& mipmapMode);

    VulkanSamplerBuilder &SetAddressModes(const VkSamplerAddressMode &addressModeU, const VkSamplerAddressMode &addressModeV,
                                          const VkSamplerAddressMode &addressModeW);

    VulkanSamplerBuilder &SetMipmapLodBias(float mipLodBias);

    VulkanSamplerBuilder &EnableAnisotropy(bool isEnabled);

    VulkanSamplerBuilder &SetMaxAnisotropy(float maxAnisotropy);

    VulkanSamplerBuilder &EnableComparing(bool isEnabled);

    VulkanSamplerBuilder &SetCompareOp(const VkCompareOp &compareOp);

    VulkanSamplerBuilder &SetMipmapLodRange(float minLod, float maxLod);

    VulkanSamplerBuilder &SetBorderColor(const VkBorderColor &borderColor);

    VulkanSamplerBuilder &EnableUnnormalizedCoordinates(bool isEnabled);

    [[nodiscard]] std::shared_ptr<VulkanSampler> Build(std::shared_ptr<VulkanDevice> device) const;

private:
    VkSamplerCreateInfo createInfo_;
};

} // common::vulkan_wrapper