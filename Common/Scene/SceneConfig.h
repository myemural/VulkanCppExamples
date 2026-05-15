/**
 * @file    SceneConfig.h
 * @brief   Contains config data structure for the scene.
 * @author  Mustafa Yemural (myemural)
 * @date    28.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "CoreDefines.h"
#include "Material.h"
#include "VulkanCommandPool.h"
#include "VulkanQueue.h"

namespace common::scene
{

enum class AttributeType
{
    POSITION,
    NORMAL,
    TANGENT,
    TEXCOORD,
    COLOR,
    JOINTS,
    WEIGHTS
};

enum class AccessorType
{
    VEC2,
    VEC3,
    VEC4
};

struct COMMON_API SceneConfig
{
    std::vector<std::pair<AttributeType, AccessorType>> attributeLayout;
    std::vector<MaterialComponent> enabledMaterialComponents;
    std::uint32_t primitiveStackCount = 24U;
    std::uint32_t primitiveSectorCount = 24U;
    std::shared_ptr<vulkan_wrapper::VulkanCommandPool> imageTransferCmdPool;
    std::shared_ptr<vulkan_wrapper::VulkanQueue> imageTransferQueue;
};

/**
 * @brief Returns the size of the accessor in bytes.
 * @param accessorType Type of the accessor.
 * @return Returns the size of the accessor in bytes.
 */
inline std::uint32_t GetAccessorSize(const AccessorType accessorType)
{
    switch (accessorType) {
        case AccessorType::VEC2:
            return sizeof(glm::vec2);
        case AccessorType::VEC3:
            return sizeof(glm::vec3);
        case AccessorType::VEC4:
            return sizeof(glm::vec4);
    }
    return UINT32_MAX;
}

/**
 * @brief Converts accessor types into VkFormat.
 * @param accessorType Type of the accessor.
 * @return Returns the appropriate VkFormat.
 */
inline VkFormat ConvertAccessorTypeToFormat(const AccessorType accessorType)
{
    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (accessorType) {
        case AccessorType::VEC2:
            format = VK_FORMAT_R32G32_SFLOAT;
            break;
        case AccessorType::VEC3:
            format = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case AccessorType::VEC4:
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
    }
    return format;
}

} // namespace common::scene
