/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationDescriptorSets.h"

#include <thread>

#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "AppCommonConfig.h"

namespace examples::fundamentals::descriptor_sets::base
{
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationDescriptorSets::SetWindow(const std::shared_ptr<Window> &window)
{
    window_ = window;
}

void ApplicationDescriptorSets::PreUpdate()
{
    window_->PollEvents();
}

void ApplicationDescriptorSets::PostUpdate()
{
    window_->SwapBuffers();
}

bool ApplicationDescriptorSets::ShouldClose()
{
    return window_->CheckWindowCloseFlag();
}

void ApplicationDescriptorSets::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationDescriptorSets::SelectDefaultPhysicalDevice()
{
    const auto physicalDevices = VulkanPhysicalDeviceSelector()
            .FilterByQueueTypes(VK_QUEUE_GRAPHICS_BIT)
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

void ApplicationDescriptorSets::CreateDefaultLogicalDevice()
{
    std::vector queuePriorities = {1.0f};

    device_ = physicalDevice_->CreateDevice([&](auto &builder) {
        builder.AddLayer("VK_LAYER_KHRONOS_validation")
                .AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .AddQueueInfo([&](auto &queueInfo) {
                    queueInfo.queueFamilyIndex = currentQueueFamilyIndex_;
                    queueInfo.queueCount = 1;
                    queueInfo.pQueuePriorities = queuePriorities.data();
                });
    });

    if (!device_) {
        throw std::runtime_error("Failed to create logical device!");
    }
}

void ApplicationDescriptorSets::CreateDefaultQueue()
{
    queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0);
}

void ApplicationDescriptorSets::CreateDefaultSwapChain()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    auto surfaceFormat = physicalDevice_->GetSurfaceFormat(surface_->GetHandle(), VK_FORMAT_B8G8R8A8_SRGB,
                                                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    auto surfaceCapabilities = physicalDevice_->GetSurfaceCapabilities(surface_->GetHandle());

    if (!surfaceFormat.has_value() || !surfaceCapabilities.has_value()) {
        throw std::runtime_error("Failed to get surface format or capabilities!");
    }

    swapChain_ = device_->CreateSwapChain(surface_, [&](auto &builder) {
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

void ApplicationDescriptorSets::CreateDefaultRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto &builder) {
        builder.AddAttachment([](auto &attachmentCreateInfo) {
                    attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                    attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                    attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                })
                .AddSubpass([&colorAttachmentRef](auto &subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void ApplicationDescriptorSets::CreateDefaultFramebuffers()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (const auto &swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage}, [&](auto &builder) {
            builder.SetDimensions(windowWidth, windowHeight);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void ApplicationDescriptorSets::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationDescriptorSets::CreateDefaultSyncObjects(const std::uint32_t maxFramesInFlight)
{
    swapImagesFences_.resize(swapChainImageViews_.size(), nullptr);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        imageAvailableSemaphores_.emplace_back(device_->CreateSemaphore());
        renderFinishedSemaphores_.emplace_back(device_->CreateSemaphore());
        inFlightFences_.emplace_back(device_->CreateFence(VK_FENCE_CREATE_SIGNALED_BIT));
    }

    if (imageAvailableSemaphores_.empty() || renderFinishedSemaphores_.empty() || inFlightFences_.empty()) {
        throw std::runtime_error("Failed to create semaphores or fences!");
    }
}

void ApplicationDescriptorSets::CreateBuffers(const std::vector<BufferCreateInfo> &bufferCreateInfos)
{
    for (const auto &[name, bufferSize, usage, memoryProps]: bufferCreateInfos) {
        buffers_[name] = std::make_unique<BufferResource>(physicalDevice_, device_);
        buffers_[name]->CreateBuffer(bufferSize, usage);
        buffers_[name]->AllocateBufferMemory(memoryProps);
    }
}

void ApplicationDescriptorSets::SetBuffer(const std::string &name, const void *data, const std::uint64_t dataSize)
{
    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(data, dataSize);
    buffers_[name]->UnmapMemory();
}

void ApplicationDescriptorSets::CreateShaderModules(const ShaderModulesCreateInfo &modulesInfo)
{
    const std::string basePath = modulesInfo.BasePath;
    const common::utility::ShaderBaseType shaderType = modulesInfo.ShaderType;

    for (const auto &[name, fileName]: modulesInfo.Modules) {
        const common::utility::ShaderLoader shaderLoader{basePath, shaderType};
        const auto shaderCode = shaderLoader.LoadSpirV(fileName);
        const auto shaderModule = device_->CreateShaderModule(shaderCode);
        if (!shaderModule) {
            throw std::runtime_error("Failed to create vertex shader module!");
        }
        shaderModules_[name] = shaderModule;
    }
}
} // namespace examples::fundamentals::descriptor_sets::base
