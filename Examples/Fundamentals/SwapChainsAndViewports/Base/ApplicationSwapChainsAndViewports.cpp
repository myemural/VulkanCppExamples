/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationSwapChainsAndViewports.h"

#include "AppCommonConfig.h"
#include "TimeUtils.h"
#include "VulkanCommandBuffer.h"
#include "VulkanInstance.h"

namespace examples::fundamentals::swap_chains_and_viewports::base
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationSwapChainsAndViewports::SetWindow(const std::shared_ptr<Window>& window) { window_ = window; }

void ApplicationSwapChainsAndViewports::PreUpdate()
{
    // Calculate delta time
    const double currentFrame = GetCurrentTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    window_->PollEvents();
}

void ApplicationSwapChainsAndViewports::PostUpdate() { window_->SwapBuffers(); }

bool ApplicationSwapChainsAndViewports::ShouldClose() { return window_->CheckWindowCloseFlag(); }

void ApplicationSwapChainsAndViewports::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationSwapChainsAndViewports::SelectDefaultPhysicalDevice()
{
    const auto physicalDevices = VulkanPhysicalDeviceSelector()
                                         .FilterByQueueTypes(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)
                                         .FilterBySurfaceSupport(surface_)
                                         .Select(instance_);

    if (physicalDevices.empty()) {
        throw std::runtime_error("No physical devices found!");
    }

    // Select first physical device
    physicalDevice_ = physicalDevices[0];

    currentQueueFamilyIndex_ = physicalDevice_->GetSurfaceSupportedQueueFamilyIndex(surface_->GetHandle());
    if (currentQueueFamilyIndex_ == UINT32_MAX) {
        throw std::runtime_error("Failed to find suitable graphics queue!");
    }
}

void ApplicationSwapChainsAndViewports::CreateDefaultLogicalDevice()
{
    std::vector queuePriorities = {1.0f};

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.multiViewport = VK_TRUE;

    device_ = physicalDevice_->CreateDevice([&](auto& builder) {
        builder.AddLayer("VK_LAYER_KHRONOS_validation")
                .AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .AddExtension("VK_EXT_shader_viewport_index_layer")
                .AddQueueInfo([&](auto& queueInfo) {
                    queueInfo.queueFamilyIndex = currentQueueFamilyIndex_;
                    queueInfo.queueCount = 1;
                    queueInfo.pQueuePriorities = queuePriorities.data();
                })
                .SetDeviceFeatures(deviceFeatures);
    });

    if (!device_) {
        throw std::runtime_error("Failed to create logical device!");
    }
}

void ApplicationSwapChainsAndViewports::CreateDefaultQueue() { queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0); }

void ApplicationSwapChainsAndViewports::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationSwapChainsAndViewports::CreateDefaultSyncObjects(const std::uint32_t swapImageCount, const std::uint32_t maxFramesInFlight)
{
    swapImagesFences_.resize(swapImageCount, nullptr);

    for (size_t i = 0; i < swapImageCount; ++i) {
        renderFinishedSemaphores_.emplace_back(device_->CreateSemaphore());
    }

    for (size_t i = 0; i < maxFramesInFlight; ++i) {
        imageAvailableSemaphores_.emplace_back(device_->CreateSemaphore());
        inFlightFences_.emplace_back(device_->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT));
    }

    if (imageAvailableSemaphores_.empty() || renderFinishedSemaphores_.empty() || inFlightFences_.empty()) {
        throw std::runtime_error("Failed to create semaphores or fences!");
    }
}

void ApplicationSwapChainsAndViewports::CreateVulkanResources(const ResourceDescriptor& resourceCreateInfo)
{
    resources_ = std::make_unique<ResourceManager>(physicalDevice_, device_);

    if (resourceCreateInfo.Buffers.has_value()) {
        resources_->CreateBuffers(resourceCreateInfo.Buffers.value());
    }

    if (resourceCreateInfo.Images.has_value()) {
        resources_->CreateImages(resourceCreateInfo.Images.value());
    }

    if (resourceCreateInfo.Samplers.has_value()) {
        resources_->CreateSamplers(resourceCreateInfo.Samplers.value());
    }

    if (resourceCreateInfo.Shaders.has_value()) {
        resources_->CreateShaderModules(resourceCreateInfo.Shaders.value());
    }

    if (resourceCreateInfo.Descriptors.has_value()) {
        resources_->CreateDescriptorSets(resourceCreateInfo.Descriptors.value());
    }
}
} // namespace examples::fundamentals::swap_chains_and_viewports::base
