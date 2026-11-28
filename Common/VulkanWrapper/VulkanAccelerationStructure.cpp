/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanAccelerationStructure.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
VulkanAccelerationStructure::VulkanAccelerationStructure(std::shared_ptr<VulkanDevice> device,
                                                         VkAccelerationStructureKHR accelerationStructure,
                                                         const VkAccelerationStructureTypeKHR type)
    : VulkanObject(std::move(device), accelerationStructure), type_(type)
{
}

VulkanAccelerationStructure::~VulkanAccelerationStructure()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            const auto pfnDestroyAccelerationStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
                    device->GetDeviceProcAddr("vkDestroyAccelerationStructureKHR"));
            if (pfnDestroyAccelerationStructure != nullptr) {
                pfnDestroyAccelerationStructure(device->GetHandle(), handle_, nullptr);
            }
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VkDeviceAddress VulkanAccelerationStructure::GetDeviceAddress() const
{
    const auto device = GetParent();
    if (!device) {
        throw std::runtime_error("Device is null!");
    }

    const auto pfnGetAccelerationStructureDeviceAddress =
            reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
                    device->GetDeviceProcAddr("vkGetAccelerationStructureDeviceAddressKHR"));
    if (pfnGetAccelerationStructureDeviceAddress == nullptr) {
        throw std::runtime_error("vkGetAccelerationStructureDeviceAddressKHR couldn't be loaded!");
    }

    VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.pNext = nullptr;
    addressInfo.accelerationStructure = handle_;

    return pfnGetAccelerationStructureDeviceAddress(device->GetHandle(), &addressInfo);
}
} // namespace common::vulkan_wrapper
