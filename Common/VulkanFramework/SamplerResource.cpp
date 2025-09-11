/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SamplerResource.h"

#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"

namespace common::vulkan_framework
{
SamplerResource::SamplerResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice> &device)
    : device_{device}
{
}

void SamplerResource::CreateSampler(const SamplerResourceCreateInfo &createInfo)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    name_ = createInfo.Name;

    sampler_ = devicePtr->CreateSampler([&](auto &builder) {
        builder.SetCreateFlags(createInfo.CreateFlags);

        builder.SetFilters(createInfo.FilteringBehavior.MagFilter, createInfo.FilteringBehavior.MinFilter);
        builder.SetMipmapMode(createInfo.FilteringBehavior.MipmapMode);
        builder.EnableAnisotropy(createInfo.FilteringBehavior.AnisotropyEnable);
        builder.SetMaxAnisotropy(createInfo.FilteringBehavior.MaxAnisotropy);

        builder.SetAddressModes(createInfo.AddressModes.U, createInfo.AddressModes.V, createInfo.AddressModes.W);
        builder.SetBorderColor(createInfo.AddressModes.BorderColor);

        builder.SetMipmapLodBias(createInfo.Lod.MipLodBias);
        builder.SetMipmapLodRange(createInfo.Lod.MinLod, createInfo.Lod.MaxLod);

        builder.EnableComparing(createInfo.ComparisonBehavior.CompareEnable);
        builder.SetCompareOp(createInfo.ComparisonBehavior.CompareOp);

        builder.EnableUnnormalizedCoordinates(createInfo.UnnormalizedCoordinates);
    });

    if (!sampler_) {
        throw std::runtime_error("Failed to create sampler!");
    }
}
} // common::vulkan_framework