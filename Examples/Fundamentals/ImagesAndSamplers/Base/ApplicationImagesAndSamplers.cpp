/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationImagesAndSamplers.h"

#include <thread>

#include "AppCommonConfig.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImage.h"
#include "VulkanInstance.h"

namespace examples::fundamentals::images_and_samplers::base
{
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationImagesAndSamplers::SetWindow(const std::shared_ptr<Window>& window) { window_ = window; }

void ApplicationImagesAndSamplers::PreUpdate() { window_->PollEvents(); }

void ApplicationImagesAndSamplers::PostUpdate() { window_->SwapBuffers(); }

bool ApplicationImagesAndSamplers::ShouldClose() { return window_->CheckWindowCloseFlag(); }

void ApplicationImagesAndSamplers::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationImagesAndSamplers::SelectDefaultPhysicalDevice()
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

void ApplicationImagesAndSamplers::CreateDefaultLogicalDevice()
{
    std::vector queuePriorities = {1.0f};

    device_ = physicalDevice_->CreateDevice([&](auto& builder) {
        builder.AddLayer("VK_LAYER_KHRONOS_validation")
                .AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .AddQueueInfo([&](auto& queueInfo) {
                    queueInfo.queueFamilyIndex = currentQueueFamilyIndex_;
                    queueInfo.queueCount = 1;
                    queueInfo.pQueuePriorities = queuePriorities.data();
                });
    });

    if (!device_) {
        throw std::runtime_error("Failed to create logical device!");
    }
}

void ApplicationImagesAndSamplers::CreateDefaultQueue() { queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0); }

void ApplicationImagesAndSamplers::CreateDefaultSwapChain()
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

void ApplicationImagesAndSamplers::CreateDefaultRenderPass()
{
    VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    renderPass_ = device_->CreateRenderPass([&](auto& builder) {
        builder.AddAttachment([](auto& attachmentCreateInfo) {
                   attachmentCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
                   attachmentCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                   attachmentCreateInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                   attachmentCreateInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                   attachmentCreateInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                   attachmentCreateInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                   attachmentCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                   attachmentCreateInfo.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
               })
                .AddSubpass([&colorAttachmentRef](auto& subpassCreateInfo) {
                    subpassCreateInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpassCreateInfo.colorAttachmentCount = 1;
                    subpassCreateInfo.pColorAttachments = &colorAttachmentRef;
                });
    });

    if (!renderPass_) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void ApplicationImagesAndSamplers::CreateDefaultFramebuffers()
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (const auto& swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(
                renderPass_, {swapImage}, [&](auto& builder) { builder.SetDimensions(windowWidth, windowHeight); });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void ApplicationImagesAndSamplers::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationImagesAndSamplers::CreateDefaultSyncObjects(const std::uint32_t maxFramesInFlight)
{
    swapImagesFences_.resize(swapChainImageViews_.size(), nullptr);

    for (size_t i = 0; i < swapChainImageViews_.size(); ++i) {
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

void ApplicationImagesAndSamplers::CreateBuffers(const std::vector<BufferResourceCreateInfo>& bufferCreateInfos)
{
    for (const auto& createInfo: bufferCreateInfos) {
        buffers_[createInfo.Name] = std::make_unique<BufferResource>(physicalDevice_, device_);
        buffers_[createInfo.Name]->CreateBuffer(createInfo);
    }
}

void ApplicationImagesAndSamplers::SetBuffer(const std::string& name, const void* data, const std::uint64_t dataSize)
{
    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(data, dataSize);
    buffers_[name]->UnmapMemory();
}

void ApplicationImagesAndSamplers::CreateShaderModules(const ShaderModulesCreateInfo& modulesInfo)
{
    shaderResources_ = std::make_unique<ShaderResource>(device_);
    shaderResources_->CreateShaders(modulesInfo);
}

void ApplicationImagesAndSamplers::CreateDescriptorSets(const DescriptorResourceCreateInfo& descriptorSetInfo)
{
    descriptorRegistry_ = std::make_unique<DescriptorRegistry>(device_);
    descriptorRegistry_->CreateDescriptors(descriptorSetInfo);

    descriptorUpdater_ = std::make_unique<DescriptorUpdater>(device_, *descriptorRegistry_);
}

void ApplicationImagesAndSamplers::UpdateDescriptorSet(const DescriptorUpdateInfo& descriptorSetUpdateInfo) const
{
    for (const auto& bufferUpdateInfo: descriptorSetUpdateInfo.BufferWriteRequests) {
        descriptorUpdater_->AddBufferUpdate(bufferUpdateInfo);
    }

    for (const auto& imageUpdateInfo: descriptorSetUpdateInfo.ImageWriteRequests) {
        descriptorUpdater_->AddImageUpdate(imageUpdateInfo);
    }

    for (const auto& texelUpdateInfo: descriptorSetUpdateInfo.TexelBufferWriteRequests) {
        descriptorUpdater_->AddTexelBufferUpdate(texelUpdateInfo);
    }

    for (const auto& copyInfo: descriptorSetUpdateInfo.CopySetRequests) {
        descriptorUpdater_->AddCopyRequest(copyInfo);
    }

    descriptorUpdater_->ApplyUpdates();
}

void ApplicationImagesAndSamplers::ChangeImageLayout(const std::shared_ptr<VulkanImage>& image,
                                                     VkImageLayout oldLayout,
                                                     VkImageLayout newLayout) const
{
    const auto cmdBufferChangeLayout = cmdPool_->CreateCommandBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY).front();

    if (!cmdBufferChangeLayout) {
        throw std::runtime_error("Failed to create command buffer for transition image layout!");
    }

    if (!cmdBufferChangeLayout->BeginCommandBuffer(
                [](auto& beginInfo) { beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; })) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        srcAccessMask = 0;
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    const auto imageMemoryBarrier = image->CreateImageMemoryBarrier(srcAccessMask, dstAccessMask, oldLayout, newLayout);

    cmdBufferChangeLayout->PipelineBarrier(srcStage, dstStage, {imageMemoryBarrier});

    if (!cmdBufferChangeLayout->EndCommandBuffer()) {
        throw std::runtime_error("Failed to end recording command buffer!");
    }

    // Directly submit this command buffer to queue
    queue_->Submit({cmdBufferChangeLayout});
    queue_->WaitIdle();
}
} // namespace examples::fundamentals::images_and_samplers::base
