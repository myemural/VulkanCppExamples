/**
 * @file    Main.cpp
 * @brief   This example gets and prints information about devices which placed in the system via Vulkan.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include <iostream>
#include <iomanip>

#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"

using namespace common::vulkan_wrapper;

int main()
{
    constexpr auto applicationName = EXAMPLE_APPLICATION_NAME;

    // Create a Vulkan instance
    const auto instance = VulkanInstanceBuilder()
            .SetApplicationInfo([=](auto &info) {
                info.pApplicationName = applicationName;
                info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                info.pEngineName = "DefaultEngine";
                info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                info.apiVersion = VK_API_VERSION_1_0;
            })
            .AddLayer("VK_LAYER_KHRONOS_validation")
            .Build();

    // Get physical devices on your system by filtering them with graphics queue support
    const auto physicalDevices = VulkanPhysicalDeviceSelector()
            .FilterByQueueTypes(VK_QUEUE_GRAPHICS_BIT)
            .Select(instance);

    // Print all device info
    for (const auto &device: physicalDevices) {
        VkPhysicalDeviceProperties props = device->GetProperties();

        // Print device general info
        std::cout << "*********************************************************" << "\n";
        std::cout << "*Device Name: " << std::setw(42) << std::setfill(' ') << std::left
                << props.deviceName << "*" << "\n";
        std::cout << "*********************************************************" << "\n";
        std::cout << "Vulkan API Version: "
                << VK_VERSION_MAJOR(props.apiVersion) << "."
                << VK_VERSION_MINOR(props.apiVersion) << "."
                << VK_VERSION_PATCH(props.apiVersion) << "\n";

        std::cout << "Driver Version: " << props.driverVersion << "\n";
        std::cout << "Vendor ID: " << props.vendorID << "\n";
        std::cout << "Device ID: " << props.deviceID << "\n";

        std::cout << "Device Type: ";
        switch (props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                std::cout << "Integrated GPU\n";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                std::cout << "Discrete GPU\n";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                std::cout << "Virtual GPU\n";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                std::cout << "CPU\n";
                break;
            default:
                std::cout << "Unknown\n";
                break;
        }

        std::cout << "*********************************************************" << '\n';

        // Print some device limits
        const auto &limits = props.limits;
        std::cout << "************ Device Limits ************" << '\n';
        std::cout << "Max Image Dimension 2D: " << limits.maxImageDimension2D << "\n";
        std::cout << "Max Uniform Buffer Range: " << limits.maxUniformBufferRange << "\n";
        std::cout << "Max Vertex Input Attributes: " << limits.maxVertexInputAttributes << "\n";
        std::cout << "Max Vertex Input Bindings: " << limits.maxVertexInputBindings << "\n";
        std::cout << "Max Push Constants Size: " << limits.maxPushConstantsSize << "\n";

        // Print queue family properties
        std::vector<VkQueueFamilyProperties> queueFamilies = device->GetQueueFamilyProperties();

        std::cout << "************ Queue Families ************" << '\n';
        for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
            const auto &q = queueFamilies[i];
            std::cout << "Queue Family " << i << ":\n";
            std::cout << "->Queue Count: " << q.queueCount << "\n";
            std::cout << "->Flags: ";
            if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                std::cout << "Graphics ";
            }
            if (q.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                std::cout << "Compute ";
            }
            if (q.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                std::cout << "Transfer ";
            }
            if (q.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
                std::cout << "SparseBinding ";
            }
            std::cout << "\n";
        }

        std::cout << "*********************************************************" << '\n';
    }

    return 0;
}
