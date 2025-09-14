/**
 * @file    SamplerResource.h
 * @brief   This file contains the implementation of the SamplerResource class, which performs operations related to
 *          creating a Vulkan sampler.
 * @author  Mustafa Yemural (myemural)
 * @date    11.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "VulkanSampler.h"
#include "VulkanDevice.h"

namespace common::vulkan_framework
{
struct SamplerResourceCreateInfo
{
    std::string Name;
    VkSamplerCreateFlags CreateFlags = 0;

    struct Filtering
    {
        VkFilter MagFilter = VK_FILTER_NEAREST;
        VkFilter MinFilter = VK_FILTER_NEAREST;
        VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        VkBool32 AnisotropyEnable = VK_FALSE;
        float MaxAnisotropy = 1.0f;
    };

    struct AddressModes
    {
        VkSamplerAddressMode U = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode V = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode W = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkBorderColor BorderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    };

    struct LodControl
    {
        float MipLodBias = 0.0f;
        float MinLod = 0.0f;
        float MaxLod = 0.0f;
    };

    struct Comparison
    {
        VkBool32 CompareEnable = VK_FALSE;
        VkCompareOp CompareOp = VK_COMPARE_OP_ALWAYS;
    };

    Filtering FilteringBehavior{};
    AddressModes AddressModes{};
    LodControl Lod{};
    Comparison ComparisonBehavior{};
    VkBool32 UnnormalizedCoordinates = VK_FALSE;
};

class SamplerResource
{
public:
    explicit SamplerResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device);

    void CreateSampler(const SamplerResourceCreateInfo &createInfo);

    [[nodiscard]] std::string GetName() const { return name_; }

    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanSampler> GetSampler() const { return sampler_; }

private:
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::string name_;
    std::shared_ptr<vulkan_wrapper::VulkanSampler> sampler_ = nullptr;
};
} // common::vulkan_framework
