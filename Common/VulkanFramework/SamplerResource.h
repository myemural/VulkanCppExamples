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

#include "CoreDefines.h"
#include "VulkanDevice.h"
#include "VulkanSampler.h"

namespace common::vulkan_framework
{
struct COMMON_API SamplerResourceCreateInfo
{
    std::string name;
    VkSamplerCreateFlags createFlags = 0;

    struct Filtering
    {
        VkFilter magFilter = VK_FILTER_NEAREST;
        VkFilter minFilter = VK_FILTER_NEAREST;
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        VkBool32 anisotropyEnable = VK_FALSE;
        float maxAnisotropy = 1.0f;
    };

    struct AddressModes
    {
        VkSamplerAddressMode u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkBorderColor borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    };

    struct LodControl
    {
        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = 0.0f;
    };

    struct Comparison
    {
        VkBool32 compareEnable = VK_FALSE;
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
    };

    Filtering filtering{};
    AddressModes addressModes{};
    LodControl lod{};
    Comparison comparisonBehavior{};
    VkBool32 unnormalizedCoordinates = VK_FALSE;
};

class COMMON_API SamplerResource
{
public:
    /**
     * @param device Refers VulkanDevice object.
     */
    explicit SamplerResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Creates a sampler resource from given information.
     * @param createInfo Sampler create information.
     */
    void CreateSampler(const SamplerResourceCreateInfo& createInfo);

    /**
     * @brief Returns name of the sampler resource.
     * @return Returns name of the sampler resource.
     */
    [[nodiscard]] std::string GetName() const { return name_; }

    /**
     * @brief Returns the sampler resource.
     * @return Returns the sampler resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanSampler> GetSampler() const { return sampler_; }

private:
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::string name_;
    std::shared_ptr<vulkan_wrapper::VulkanSampler> sampler_ = nullptr;
};
} // namespace common::vulkan_framework
