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

/**
 * @brief Returns appropriate Vulkan format for given data type.
 * @tparam T Data type.
 * @return Returns appropriate Vulkan format.
 */
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
constexpr VkFormat GetVkFormat<utility::Vec2>()
{
    return VK_FORMAT_R32G32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<utility::Vec3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<utility::Color3>()
{
    return VK_FORMAT_R32G32B32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<utility::Color4>()
{
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

template<>
constexpr VkFormat GetVkFormat<float[4]>()
{
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

/**
 * @brief Generates and returns input binding description that usable in Vulkan.
 * @tparam Vertex Type of the vertex.
 * @param bindingIndex Binding index of the binding description.
 * @param inputRate Input rate value.
 * @return Returns vertex input binding description that usable in Vulkan.
 */
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

/**
 * @brief Generates and returns vertex input attribute description that usable in Vulkan.
 * @tparam AttrType Type of the attribute.
 * @param bindingIndex Binding index that matches with binding description.
 * @param offset Offset value of the attribute.
 * @return Returns vertex input attribute description that usable in Vulkan.
 */
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

/**
 * @brief Short way to generate attribute descriptions.
 * @param VertexStruct Type of the vertex struct.
 * @param Attribute Attribute name.
 * @param BindingIndex Binding index.
 */
#define GenerateAttributeDescription(VertexStruct, Attribute, BindingIndex)                                            \
    GenerateAttributeDescriptionInternal<decltype(VertexStruct::Attribute)>(BindingIndex,                              \
                                                                            offsetof(VertexStruct, Attribute))

/**
 * @brief Returns the specific position and size values of the region on the atlas image.
 * @param rect 2D rectangle position and size values.
 * @param atlasWidth Atlas image width.
 * @param atlasHeight Atlas image height.
 * @return Returns the position and size values of the region on the atlas image (X, Y, Width, Height).
 */
constexpr utility::Vec4
CalculateUVRect(const VkRect2D& rect, const uint32_t atlasWidth, const uint32_t atlasHeight)
{
    const float u0 = static_cast<float>(rect.offset.x) / static_cast<float>(atlasWidth);
    const float v0 = static_cast<float>(rect.offset.y) / static_cast<float>(atlasHeight);
    const float u1 = static_cast<float>(rect.offset.x + rect.extent.width) / static_cast<float>(atlasWidth);
    const float v1 = static_cast<float>(rect.offset.y + rect.extent.height) / static_cast<float>(atlasHeight);

    return {u0, v0, (u1 - u0), (v1 - v0)};
}

/**
 * @brief Animates and gets the current value of the scissor rectangle.
 * @param time Current time value.
 * @param viewportWidth Width of the viewport.
 * @param viewportHeight Height of the viewport.
 * @return Returns the current value of the scissor rectangle.
 */
inline VkRect2D GetAnimatedScissorRect(const float time, const float viewportWidth, const float viewportHeight)
{
    constexpr float scissorWidth = 250.0f;
    constexpr float scissorHeight = 200.0f;
    constexpr float speed = 1.0f; // rad/sec
    const float orbitRadiusX = (viewportWidth - scissorWidth) * 0.45f;
    const float orbitRadiusY = (viewportHeight - scissorHeight) * 0.45f;
    const float centerX = viewportWidth * 0.5f + orbitRadiusX * cos(time * speed);
    const float centerY = viewportHeight * 0.5f + orbitRadiusY * sin(time * speed);

    VkRect2D rect{};
    rect.offset.x = static_cast<int32_t>(std::clamp(centerX - scissorWidth * 0.5f, 0.0f, viewportWidth - scissorWidth));
    rect.offset.y =
            static_cast<int32_t>(std::clamp(centerY - scissorHeight * 0.5f, 0.0f, viewportHeight - scissorHeight));
    rect.extent.width = static_cast<std::uint32_t>(scissorWidth);
    rect.extent.height = static_cast<std::uint32_t>(scissorHeight);

    return rect;
}
} // namespace common::vulkan_framework
