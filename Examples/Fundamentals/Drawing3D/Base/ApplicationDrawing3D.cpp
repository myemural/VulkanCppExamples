/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ApplicationDrawing3D.h"

#include <thread>

#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "VulkanCommandBuffer.h"
#include "AppCommonConfig.h"

namespace examples::fundamentals::drawing_3d::base
{
using namespace common::vulkan_wrapper;
using namespace common::vulkan_framework;
using namespace common::window_wrapper;

void ApplicationDrawing3D::SetWindow(const std::shared_ptr<Window> &window)
{
    window_ = window;
}

void ApplicationDrawing3D::Update()
{
    window_->OnUpdate();
    std::this_thread::sleep_for(std::chrono::milliseconds(params_.Get<long long>(VulkanParams::RenderLoopMs)));
}

bool ApplicationDrawing3D::ShouldClose()
{
    return window_->CheckWindowCloseFlag();
}

void ApplicationDrawing3D::CreateDefaultSurface()
{
    const auto vulkanSurface = window_->CreateVulkanSurface(instance_->GetHandle());

    if (!vulkanSurface) {
        throw std::runtime_error("Failed to get Vulkan surface from window system!");
    }

    surface_ = std::make_shared<VulkanSurface>(instance_, vulkanSurface);
}

void ApplicationDrawing3D::SelectDefaultPhysicalDevice()
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

void ApplicationDrawing3D::CreateDefaultLogicalDevice()
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

void ApplicationDrawing3D::CreateDefaultQueue()
{
    queue_ = device_->CreateQueue(currentQueueFamilyIndex_, 0);
}

void ApplicationDrawing3D::CreateDefaultSwapChain()
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

void ApplicationDrawing3D::CreateDefaultFramebuffers(const std::shared_ptr<VulkanImageView> &depthImageView)
{
    const auto windowWidth = window_->GetWindowWidth();
    const auto windowHeight = window_->GetWindowHeight();

    for (const auto &swapImage: swapChainImageViews_) {
        auto framebuffer = device_->CreateFramebuffer(renderPass_, {swapImage, depthImageView}, [&](auto &builder) {
            builder.SetDimensions(windowWidth, windowHeight);
        });

        if (!framebuffer) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        framebuffers_.push_back(framebuffer);
    }
}

void ApplicationDrawing3D::CreateDefaultCommandPool()
{
    cmdPool_ = device_->CreateCommandPool(currentQueueFamilyIndex_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    if (!cmdPool_) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void ApplicationDrawing3D::CreateDefaultSyncObjects(const std::uint32_t maxFramesInFlight)
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

void ApplicationDrawing3D::CreateBuffers(const std::vector<BufferCreateInfo> &bufferCreateInfos)
{
    for (const auto &[name, bufferSize, usage, memoryProps]: bufferCreateInfos) {
        buffers_[name] = std::make_unique<BufferResource>(physicalDevice_, device_);
        buffers_[name]->CreateBuffer(bufferSize, usage);
        buffers_[name]->AllocateBufferMemory(memoryProps);
    }
}

void ApplicationDrawing3D::SetBuffer(const std::string &name, const void *data, const std::uint64_t dataSize)
{
    buffers_[name]->MapMemory();
    buffers_[name]->FlushData(data, dataSize);
    buffers_[name]->UnmapMemory();
}

void ApplicationDrawing3D::CreateImages(const std::vector<ImageResourceCreateInfo> &imageCreateInfos)
{
    for (const auto &createInfo: imageCreateInfos) {
        images_[createInfo.Name] = std::make_unique<ImageResource>(physicalDevice_, device_);
        images_[createInfo.Name]->CreateImage(createInfo);
    }
}

void ApplicationDrawing3D::CreateSamplers(const std::vector<SamplerResourceCreateInfo> &samplerCreateInfos)
{
    for (const auto &createInfo: samplerCreateInfos) {
        samplers_[createInfo.Name] = std::make_unique<SamplerResource>(device_);
        samplers_[createInfo.Name]->CreateSampler(createInfo);
    }
}

void ApplicationDrawing3D::SetImageFromBuffer(const std::string &name,
                                              const std::shared_ptr<VulkanBuffer> &stagingBuffer,
                                              const VkExtent3D &dimensions)
{
    images_[name]->ChangeImageLayout(cmdPool_, queue_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    /// TODO: It is constant for now
    const VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = dimensions
    };
    images_[name]->CopyDataFromBuffer(cmdPool_, queue_, stagingBuffer, copyRegion);
    images_[name]->ChangeImageLayout(cmdPool_, queue_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void ApplicationDrawing3D::CreateShaderModules(const ShaderModulesCreateInfo &modulesInfo)
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

void ApplicationDrawing3D::CreateDescriptorSets(const DescriptorSetCreateInfo &descriptorSetInfo)
{
    descriptorRegistry_ = std::make_unique<DescriptorRegistry>(device_);
    descriptorRegistry_->CreatePool(descriptorSetInfo.MaxSets, descriptorSetInfo.PoolSizes);

    for (const auto &[name, bindings]: descriptorSetInfo.Layouts) {
        descriptorRegistry_->AddLayout(name, bindings);
        descriptorRegistry_->CreateDescriptorSet(name);
    }

    descriptorUpdater_ = std::make_unique<DescriptorUpdater>(device_, *descriptorRegistry_);
}

void ApplicationDrawing3D::UpdateDescriptorSet(const DescriptorSetUpdateInfo &descriptorSetUpdateInfo) const
{
    for (const auto &bufferUpdateInfo: descriptorSetUpdateInfo.BufferWriteRequests) {
        descriptorUpdater_->AddBufferUpdate(bufferUpdateInfo);
    }

    for (const auto &imageUpdateInfo: descriptorSetUpdateInfo.ImageWriteRequests) {
        descriptorUpdater_->AddImageUpdate(imageUpdateInfo);
    }

    for (const auto &texelUpdateInfo: descriptorSetUpdateInfo.TexelBufferWriteRequests) {
        descriptorUpdater_->AddTexelBufferUpdate(texelUpdateInfo);
    }

    for (const auto &copyInfo: descriptorSetUpdateInfo.CopySetRequests) {
        descriptorUpdater_->AddCopyRequest(copyInfo);
    }

    descriptorUpdater_->ApplyUpdates();
}
} // namespace examples::fundamentals::drawing_3d::base
