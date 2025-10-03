/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanSampler.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
inline VkSamplerCreateInfo GetDefaultSamplerCreateInfo()
{
    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.mipLodBias = 0.0f;
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy = 1.0f;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;
    createInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    return createInfo;
}

VulkanSampler::VulkanSampler(std::shared_ptr<VulkanDevice> device, VkSampler sampler)
    : VulkanObject(std::move(device), sampler)
{
}

VulkanSampler::~VulkanSampler()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroySampler(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VulkanSamplerBuilder::VulkanSamplerBuilder() : createInfo_(GetDefaultSamplerCreateInfo()) {}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetCreateFlags(const VkSamplerCreateFlags& flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetFilters(const VkFilter& magFilter, const VkFilter& minFilter)
{
    createInfo_.magFilter = magFilter;
    createInfo_.minFilter = minFilter;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetMipmapMode(const VkSamplerMipmapMode& mipmapMode)
{
    createInfo_.mipmapMode = mipmapMode;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetAddressModes(const VkSamplerAddressMode& addressModeU,
                                                            const VkSamplerAddressMode& addressModeV,
                                                            const VkSamplerAddressMode& addressModeW)
{
    createInfo_.addressModeU = addressModeU;
    createInfo_.addressModeV = addressModeV;
    createInfo_.addressModeW = addressModeW;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetMipmapLodBias(const float mipLodBias)
{
    createInfo_.mipLodBias = mipLodBias;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::EnableAnisotropy(const bool isEnabled)
{
    createInfo_.anisotropyEnable = isEnabled;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetMaxAnisotropy(const float maxAnisotropy)
{
    createInfo_.maxAnisotropy = maxAnisotropy;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::EnableComparing(const bool isEnabled)
{
    createInfo_.compareEnable = isEnabled;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetCompareOp(const VkCompareOp& compareOp)
{
    createInfo_.compareOp = compareOp;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetMipmapLodRange(const float minLod, const float maxLod)
{
    createInfo_.minLod = minLod;
    createInfo_.maxLod = maxLod;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::SetBorderColor(const VkBorderColor& borderColor)
{
    createInfo_.borderColor = borderColor;
    return *this;
}

VulkanSamplerBuilder& VulkanSamplerBuilder::EnableUnnormalizedCoordinates(const bool isEnabled)
{
    createInfo_.unnormalizedCoordinates = isEnabled;
    return *this;
}

std::shared_ptr<VulkanSampler> VulkanSamplerBuilder::Build(std::shared_ptr<VulkanDevice> device) const
{
    VkSampler sampler = VK_NULL_HANDLE;
    if (vkCreateSampler(device->GetHandle(), &createInfo_, nullptr, &sampler) != VK_SUCCESS) {
        std::cerr << "Failed to create sampler!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanSampler>(std::move(device), sampler);
}
} // namespace common::vulkan_wrapper
