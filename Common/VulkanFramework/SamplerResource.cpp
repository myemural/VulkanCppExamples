/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "SamplerResource.h"

#include "VulkanCommandBuffer.h"

namespace common::vulkan_framework
{
SamplerResource::SamplerResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device) : device_{device} {}

void SamplerResource::CreateSampler(const SamplerResourceCreateInfo& createInfo)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    name_ = createInfo.name;

    sampler_ = devicePtr->CreateSampler([&](auto& builder) {
        builder.SetCreateFlags(createInfo.createFlags);

        builder.SetFilters(createInfo.filtering.magFilter, createInfo.filtering.minFilter);
        builder.SetMipmapMode(createInfo.filtering.mipmapMode);
        builder.EnableAnisotropy(createInfo.filtering.anisotropyEnable);
        builder.SetMaxAnisotropy(createInfo.filtering.maxAnisotropy);

        builder.SetAddressModes(createInfo.addressModes.u, createInfo.addressModes.v, createInfo.addressModes.w);
        builder.SetBorderColor(createInfo.addressModes.borderColor);

        builder.SetMipmapLodBias(createInfo.lod.mipLodBias);
        builder.SetMipmapLodRange(createInfo.lod.minLod, createInfo.lod.maxLod);

        builder.EnableComparing(createInfo.comparisonBehavior.compareEnable);
        builder.SetCompareOp(createInfo.comparisonBehavior.compareOp);

        builder.EnableUnnormalizedCoordinates(createInfo.unnormalizedCoordinates);
    });

    if (!sampler_) {
        throw std::runtime_error("Failed to create sampler!");
    }
}
} // namespace common::vulkan_framework
