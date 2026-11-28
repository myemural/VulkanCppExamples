/**
 * @file    VulkanAccelerationStructure.h
 * @brief   This file contains wrapper class implementation for VkAccelerationStructureKHR.
 * @author  Mustafa Yemural (myemural)
 * @date    28.11.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <memory>

#include <vulkan/vulkan_core.h>

#include "CoreDefines.h"
#include "VulkanObject.h"

namespace common::vulkan_wrapper
{
class VulkanDevice;

class VulkanAccelerationStructure final : public VulkanObject<VulkanDevice, VkAccelerationStructureKHR>
{
public:
    COMMON_API VulkanAccelerationStructure(std::shared_ptr<VulkanDevice> device,
                                           VkAccelerationStructureKHR accelerationStructure,
                                           VkAccelerationStructureTypeKHR type);

    COMMON_API ~VulkanAccelerationStructure() override;

    [[nodiscard]] COMMON_API VkDeviceAddress GetDeviceAddress() const;

    [[nodiscard]] COMMON_API VkAccelerationStructureTypeKHR GetType() const { return type_; }

private:
    VkAccelerationStructureTypeKHR type_;
};
} // namespace common::vulkan_wrapper
