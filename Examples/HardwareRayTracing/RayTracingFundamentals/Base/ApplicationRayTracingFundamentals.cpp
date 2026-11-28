/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationRayTracingFundamentals.h"

#include "AppCommonConfig.h"
#include "TimeUtils.h"
#include "VulkanInstance.h"

namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::base
{
using namespace common::utility;
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationRayTracingFundamentals::SetWindow(const std::shared_ptr<Window>& window) { window_ = window; }

bool ApplicationRayTracingFundamentals::Init()
{
    try {
        currentWindowWidth_ = GetParamU32(WindowParams::Width);
        currentWindowHeight_ = GetParamU32(WindowParams::Height);

        CreateDefaultSurface();
        SelectDefaultPhysicalDevice();
        QueryRayTracingProperties();
        CreateDefaultLogicalDevice();
        CreateDefaultQueue();
        CreateDefaultSwapChain();
        CreateDefaultCommandPool();
        CreateDefaultSyncObjects();

        resources_ = std::make_unique<ResourceManager>(physicalDevice_, device_);

        depthImageFormat_ = physicalDevice_->FindSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

void ApplicationRayTracingFundamentals::PreUpdate()
{
    // Calculate delta time
    const double currentFrame = GetCurrentTime();
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    Window::PollEvents();

    // FPS calculation
    fpsTimer_ += deltaTime_;
    frameCounter_++;

    if (fpsTimer_ >= 1.0) // Print every second
    {
        const int fps = static_cast<int>(static_cast<double>(frameCounter_) / fpsTimer_);
        const auto newWindowTitle = GetParamStr(WindowParams::Title) + std::string(" - FPS: ") + std::to_string(fps);
        window_->SetWindowTitle(newWindowTitle);

        frameCounter_ = 0;
        fpsTimer_ = 0.0;
    }
}

void ApplicationRayTracingFundamentals::PostUpdate() { window_->SwapBuffers(); }

bool ApplicationRayTracingFundamentals::ShouldClose() { return window_->CheckWindowCloseFlag(); }

void ApplicationRayTracingFundamentals::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationRayTracingFundamentals::SelectDefaultPhysicalDevice()
{
    // Ray tracing extensions depend on Vulkan 1.2 core features:
    // - Buffer device address
    // - Descriptor indexing
    // - SPIR-V 1.4
    const auto physicalDevices = VulkanPhysicalDeviceSelector()
                                         .FilterByQueueTypes(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)
                                         .FilterBySurfaceSupport(surface_)
                                         .FilterByMinApiVersion(VK_API_VERSION_1_2)
                                         .FilterByExtensionSupport(kRequiredDeviceExtensions)
                                         .Select(instance_);

    if (physicalDevices.empty()) {
        throw std::runtime_error("No physical devices found that support hardware-accelerated ray tracing!");
    }

    // Select first physical device (discrete GPUs are listed first)
    physicalDevice_ = physicalDevices[0];

    currentQueueFamilyIndex_ = physicalDevice_->GetSurfaceSupportedQueueFamilyIndex(surface_->GetHandle());
    if (currentQueueFamilyIndex_ == UINT32_MAX) {
        throw std::runtime_error("Failed to find suitable graphics queue!");
    }
}

void ApplicationRayTracingFundamentals::QueryRayTracingProperties()
{
    accelerationStructureProperties_ = {};
    accelerationStructureProperties_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
    accelerationStructureProperties_.pNext = nullptr;

    rayTracingPipelineProperties_ = {};
    rayTracingPipelineProperties_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    rayTracingPipelineProperties_.pNext = &accelerationStructureProperties_;

    const auto properties = physicalDevice_->GetProperties2(&rayTracingPipelineProperties_);

    // Break the chain, so the stored structures don't keep pointers to each other
    rayTracingPipelineProperties_.pNext = nullptr;

    std::cout << "Selected GPU: " << properties.properties.deviceName
              << " (Max ray recursion depth: " << rayTracingPipelineProperties_.maxRayRecursionDepth << ")" << '\n';
}

void ApplicationRayTracingFundamentals::CreateDefaultLogicalDevice()
{
    std::vector queuePriorities = {1.0f};

    // Query supported features first, so a meaningful error can be given instead of a device creation failure
    VkPhysicalDeviceRayQueryFeaturesKHR supportedRayQueryFeatures{};
    supportedRayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR supportedRayTracingPipelineFeatures{};
    supportedRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    supportedRayTracingPipelineFeatures.pNext = &supportedRayQueryFeatures;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR supportedAccelerationStructureFeatures{};
    supportedAccelerationStructureFeatures.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    supportedAccelerationStructureFeatures.pNext = &supportedRayTracingPipelineFeatures;

    VkPhysicalDeviceVulkan12Features supportedVulkan12Features{};
    supportedVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    supportedVulkan12Features.pNext = &supportedAccelerationStructureFeatures;

    const auto supportedFeatures = physicalDevice_->GetSupportedFeatures2(&supportedVulkan12Features);

    std::string missingFeatures;
    const auto checkFeature = [&](const VkBool32 isSupported, const char* featureName) {
        if (!isSupported) {
            missingFeatures += std::string(" ") + featureName;
        }
    };

    checkFeature(supportedVulkan12Features.bufferDeviceAddress, "bufferDeviceAddress");
    checkFeature(supportedVulkan12Features.scalarBlockLayout, "scalarBlockLayout");
    checkFeature(supportedVulkan12Features.shaderSampledImageArrayNonUniformIndexing,
                 "shaderSampledImageArrayNonUniformIndexing");
    checkFeature(supportedVulkan12Features.runtimeDescriptorArray, "runtimeDescriptorArray");
    checkFeature(supportedVulkan12Features.descriptorBindingVariableDescriptorCount,
                 "descriptorBindingVariableDescriptorCount");
    checkFeature(supportedVulkan12Features.descriptorBindingPartiallyBound, "descriptorBindingPartiallyBound");
    checkFeature(supportedAccelerationStructureFeatures.accelerationStructure, "accelerationStructure");
    checkFeature(supportedRayTracingPipelineFeatures.rayTracingPipeline, "rayTracingPipeline");
    checkFeature(supportedRayQueryFeatures.rayQuery, "rayQuery");
    checkFeature(supportedFeatures.features.shaderInt64, "shaderInt64");

    if (!missingFeatures.empty()) {
        throw std::runtime_error("Selected GPU doesn't support required features:" + missingFeatures);
    }

    // Enable required features
    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.pNext = nullptr;
    vulkan12Features.bufferDeviceAddress = VK_TRUE;
    vulkan12Features.scalarBlockLayout = VK_TRUE;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vulkan12Features.runtimeDescriptorArray = VK_TRUE;
    vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
    rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    rayQueryFeatures.pNext = &vulkan12Features;
    rayQueryFeatures.rayQuery = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineFeatures.pNext = &rayQueryFeatures;
    rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
    accelerationStructureFeatures.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.shaderInt64 = VK_TRUE; // For buffer reference (device address) arithmetic in shaders

    device_ = physicalDevice_->CreateDevice([&](auto& builder) {
        builder.AddExtendingStructure(&accelerationStructureFeatures)
                .AddExtensions(kRequiredDeviceExtensions)
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

void ApplicationRayTracingFundamentals::CreateDefaultQueue()
{
    queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0);
}

void ApplicationRayTracingFundamentals::CreateDefaultSwapChain()
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

void
ApplicationRayTracingFundamentals::CreateDefaultFramebuffers(const std::shared_ptr<VulkanImageView>& depthImageView)
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

void ApplicationRayTracingFundamentals::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationRayTracingFundamentals::CreateDefaultSyncObjects()
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

void ApplicationRayTracingFundamentals::CreateVulkanResources(const ResourceDescriptor& resourceCreateInfo) const
{
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

void ApplicationRayTracingFundamentals::ExecuteOneTimeCommands(
        const std::function<void(const std::shared_ptr<VulkanCommandBuffer>&)>& recordFunc) const
{
    const auto cmdBuffers = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    if (cmdBuffers.empty() || !cmdBuffers.front()) {
        throw std::runtime_error("Failed to create one-time command buffer!");
    }

    const auto& cmdBuffer = cmdBuffers.front();

    if (!cmdBuffer->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording one-time command buffer!");
    }

    recordFunc(cmdBuffer);

    if (!cmdBuffer->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording one-time command buffer!");
    }

    queue_->Submit({cmdBuffer});
    queue_->WaitIdle();
}
} // namespace examples::hardware_ray_tracing::ray_tracing_fundamentals::base
