/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationPipelinesAndPasses.h"

#include "AppCommonConfig.h"
#include "TimeUtils.h"
#include "VulkanInstance.h"

namespace examples::fundamentals::pipelines_and_passes::base
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationPipelinesAndPasses::SetWindow(const std::shared_ptr<Window>& window) { window_ = window; }

void ApplicationPipelinesAndPasses::PreUpdate()
{
    // Calculate delta time
    const double currentFrame = GetCurrentTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    Window::PollEvents();
}

void ApplicationPipelinesAndPasses::PostUpdate() { window_->SwapBuffers(); }

bool ApplicationPipelinesAndPasses::ShouldClose() { return window_->CheckWindowCloseFlag(); }

void ApplicationPipelinesAndPasses::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationPipelinesAndPasses::SelectDefaultPhysicalDevice()
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

void ApplicationPipelinesAndPasses::CreateDefaultLogicalDevice()
{
    std::vector queuePriorities = {1.0f};

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;
    deviceFeatures.shaderStorageImageReadWithoutFormat = VK_TRUE;

    device_ = physicalDevice_->CreateDevice([&](auto& builder) {
        builder.AddLayer("VK_LAYER_KHRONOS_validation")
                .AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
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

void ApplicationPipelinesAndPasses::CreateDefaultQueue() { queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0); }

void ApplicationPipelinesAndPasses::CreateDefaultSwapChain()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    auto surfaceFormat = physicalDevice_->GetSurfaceFormat(surface_->GetHandle(), VK_FORMAT_B8G8R8A8_SRGB,
                                                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    auto surfaceCapabilities = physicalDevice_->GetSurfaceCapabilities(surface_->GetHandle());

    if (!surfaceFormat.has_value() || !surfaceCapabilities.has_value()) {
        throw std::runtime_error("Failed to get surface format or capabilities!");
    }

    swapChain_ = device_->CreateSwapChain(surface_, [&](auto& builder) {
        builder.SetMinImageCount(surfaceCapabilities.value().minImageCount + 1)
                .SetImageFormat(surfaceFormat->format)
                .SetImageColorSpace(surfaceFormat->colorSpace)
                .SetImageExtent(windowWidth, windowHeight)
                .SetPreTransformFlagBits(surfaceCapabilities.value().currentTransform);
    });

    if (!swapChain_) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    swapChainImageViews_ = swapChain_->GetSwapChainImageViews();

    if (swapChainImageViews_.empty()) {
        throw std::runtime_error("Failed to get swap chain image views!");
    }
}

void ApplicationPipelinesAndPasses::CreateDefaultFramebuffers(const std::shared_ptr<VulkanImageView>& depthImageView)
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage, depthImageView}, [&](auto& builder) {
            builder.SetDimensions(windowWidth, windowHeight);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void ApplicationPipelinesAndPasses::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationPipelinesAndPasses::CreateDefaultSyncObjects()
{
    swapImagesFences_.resize(swapChainImageViews_.size(), nullptr);

    for (size_t i = 0; i < swapChainImageViews_.size(); ++i) {
        renderFinishedSemaphores_.emplace_back(device_->CreateSemaphore());
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphores_.emplace_back(device_->CreateSemaphore());
        inFlightFences_.emplace_back(device_->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT));
    }

    if (imageAvailableSemaphores_.empty() || renderFinishedSemaphores_.empty() || inFlightFences_.empty()) {
        throw std::runtime_error("Failed to create semaphores or fences!");
    }
}

void ApplicationPipelinesAndPasses::CreateVulkanResources(const ResourceDescriptor& resourceCreateInfo)
{
    resources_ = std::make_unique<ResourceManager>(physicalDevice_, device_);

    if (resourceCreateInfo.buffers.has_value()) {
        resources_->CreateBuffers(resourceCreateInfo.buffers.value());
    }

    if (resourceCreateInfo.images.has_value()) {
        resources_->CreateImages(resourceCreateInfo.images.value());
    }

    if (resourceCreateInfo.samplers.has_value()) {
        resources_->CreateSamplers(resourceCreateInfo.samplers.value());
    }

    if (resourceCreateInfo.shaders.has_value()) {
        resources_->CreateShaderModules(resourceCreateInfo.shaders.value());
    }

    if (resourceCreateInfo.descriptors.has_value()) {
        resources_->CreateDescriptorSets(resourceCreateInfo.descriptors.value());
    }
}
} // namespace examples::fundamentals::pipelines_and_passes::base
