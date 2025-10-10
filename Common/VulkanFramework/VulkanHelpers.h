/**
 * @file    VulkanHelpers.h
 * @brief   This file contains some helper functions and classes using in Vulkan application implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    17.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <algorithm>

#include <vulkan/vulkan_core.h>

#include "Vertex.h"

namespace common::vulkan_framework
{

template<typename T>
constexpr VkFormat GetVkFormat()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<float>()
{
    return VK_FORMAT_R32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<common::utility::Vec2>()
{
    return VK_FORMAT_R32G32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<common::utility::Vec3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<common::utility::Color3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<common::utility::Color4>()
{
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<float[4]>()
{
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

template<typename Vertex>
static constexpr VkVertexInputBindingDescription
GenerateBindingDescription(const std::uint32_t bindingIndex,
                           const VkVertexInputRate& inputRate = VK_VERTEX_INPUT_RATE_VERTEX)
{
    VkVertexInputBindingDescription desc{};
    desc.binding = bindingIndex;
    desc.stride = sizeof(Vertex);
    desc.inputRate = inputRate;
    return desc;
}

template<typename AttrType>
VkVertexInputAttributeDescription GenerateAttributeDescriptionInternal(const uint32_t bindingIndex,
                                                                       const uint32_t offset)
{
    VkVertexInputAttributeDescription description{};
    description.location = AttrType::Location;
    description.binding = bindingIndex;
    description.format = GetVkFormat<typename AttrType::Type>();
    description.offset = offset;
    return description;
}

#define GenerateAttributeDescription(VertexStruct, Attribute, BindingIndex)                                            \
    GenerateAttributeDescriptionInternal<decltype(VertexStruct::Attribute)>(BindingIndex,                              \
                                                                            offsetof(VertexStruct, Attribute))

constexpr common::utility::Vec4
CalculateUVRect(const VkRect2D& r, const uint32_t atlasWidth, const uint32_t atlasHeight)
{
    const float u0 = static_cast<float>(r.offset.x) / static_cast<float>(atlasWidth);
    const float v0 = static_cast<float>(r.offset.y) / static_cast<float>(atlasHeight);
    const float u1 = static_cast<float>(r.offset.x + r.extent.width) / static_cast<float>(atlasWidth);
    const float v1 = static_cast<float>(r.offset.y + r.extent.height) / static_cast<float>(atlasHeight);

    return {u0, v0, (u1 - u0), (v1 - v0)};
}

inline VkRect2D GetAnimatedScissorRect(const float timeSeconds, const float viewportWidth, const float viewportHeight)
{
    constexpr float scissorWidth = 250.0f;
    constexpr float scissorHeight = 200.0f;
    constexpr float speed = 1.0f; // rad/sec
    const float orbitRadiusX = (viewportWidth - scissorWidth) * 0.45f;
    const float orbitRadiusY = (viewportHeight - scissorHeight) * 0.45f;
    const float centerX = viewportWidth * 0.5f + orbitRadiusX * std::cos(timeSeconds * speed);
    const float centerY = viewportHeight * 0.5f + orbitRadiusY * std::sin(timeSeconds * speed);

    VkRect2D rect{};
    rect.offset.x = static_cast<int32_t>(std::clamp(centerX - scissorWidth * 0.5f, 0.0f, viewportWidth - scissorWidth));
    rect.offset.y =
            static_cast<int32_t>(std::clamp(centerY - scissorHeight * 0.5f, 0.0f, viewportHeight - scissorHeight));
    rect.extent.width = static_cast<std::uint32_t>(scissorWidth);
    rect.extent.height = static_cast<std::uint32_t>(scissorHeight);

    return rect;
}
} // namespace common::vulkan_framework
