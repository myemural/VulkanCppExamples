/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanDevice.h"

#include <algorithm>

#include "VulkanPhysicalDevice.h"
#include "VulkanQueue.h"
#include "VulkanCommandPool.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanShaderModule.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDeviceMemory.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

namespace common::vulkan_wrapper
{

inline VkDeviceCreateInfo GetDefaultDeviceCreateInfo()
{
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = 0;
    createInfo.pQueueCreateInfos = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.pEnabledFeatures = nullptr;
    return createInfo;
}

inline VkDeviceQueueCreateInfo GetDefaultQueueCreateInfo()
{
    VkDeviceQueueCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueFamilyIndex = 0;
    createInfo.queueCount = 0;
    createInfo.pQueuePriorities = nullptr;
    return createInfo;
}

VulkanDevice::VulkanDevice(std::shared_ptr<VulkanPhysicalDevice> physicalDevice, VkDevice const device)
        : VulkanObject(std::move(physicalDevice), device)
{}

VulkanDevice::~VulkanDevice()
{
    if (handle_ != VK_NULL_HANDLE) {
        vkDestroyDevice(handle_, nullptr);
    }
}

std::shared_ptr<VulkanQueue> VulkanDevice::CreateQueue(const std::uint32_t queueFamilyIndex, const std::uint32_t queueIndex)
{
    auto device = shared_from_this();

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device->GetHandle(), queueFamilyIndex, queueIndex, &queue);
    return std::make_shared<VulkanQueue>(device, queue);
}

std::shared_ptr<VulkanSemaphore> VulkanDevice::CreateSemaphore()
{
    auto device = shared_from_this();

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore = VK_NULL_HANDLE;
    if (vkCreateSemaphore(device->GetHandle(), &semaphoreCreateInfo, nullptr, &semaphore) != VK_SUCCESS) {
        std::cout << "Failed to create semaphore!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanSemaphore>(device, semaphore);
}

std::shared_ptr<VulkanFence> VulkanDevice::CreateFence(const VkFenceCreateFlags& flags)
{
    auto device = shared_from_this();

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = flags;

    VkFence fence = VK_NULL_HANDLE;
    if (vkCreateFence(device->GetHandle(), &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
        std::cout << "Failed to create fence!" << std::endl;
        return nullptr;
    }

    // Add to list
    return std::make_shared<VulkanFence>(device, fence);
}

std::shared_ptr<VulkanCommandPool> VulkanDevice::CreateCommandPool(const std::uint32_t queueFamilyIndex, const VkCommandPoolCreateFlags& flags)
{
    auto device = shared_from_this();

    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool cmdPool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device->GetHandle(), &createInfo, nullptr, &cmdPool) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanCommandPool>(device, cmdPool);
}

std::shared_ptr<VulkanDescriptorPool> VulkanDevice::CreateDescriptorPool(const std::uint32_t maxSets,
    const std::vector<VkDescriptorPoolSize>& poolSizes, const VkDescriptorPoolCreateFlags &flags)
{
    auto device = shared_from_this();

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = flags;
    descriptorPoolCreateInfo.maxSets = maxSets;
    descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data();

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(device->GetHandle(), &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        std::cout << "Failed to create descriptor pool!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanDescriptorPool>(device, descriptorPool);
}

std::shared_ptr<VulkanDescriptorSetLayout> VulkanDevice::CreateDescriptorSetLayout(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings, const VkDescriptorSetLayoutCreateFlags &flags)
{
    auto device = shared_from_this();

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pNext = nullptr;
    descriptorSetLayoutCreateInfo.flags = flags;
    descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
    descriptorSetLayoutCreateInfo.pBindings = bindings.empty() ? nullptr : bindings.data();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device->GetHandle(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cout << "Failed to create descriptor set layout!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanDescriptorSetLayout>(device, descriptorSetLayout);
}

std::shared_ptr<VulkanSwapChain> VulkanDevice::CreateSwapChain(const std::shared_ptr<VulkanSurface>& surface,
                                                               const std::function<void(VulkanSwapChainBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanSwapChainBuilder builder;
    builderFunc(builder);

    return builder.Build(device, surface);
}

std::shared_ptr<VulkanRenderPass> VulkanDevice::CreateRenderPass(
    const std::function<void(VulkanRenderPassBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanRenderPassBuilder builder;
    builderFunc(builder);

    return builder.Build(device);
}

std::shared_ptr<VulkanFramebuffer> VulkanDevice::CreateFramebuffer(const std::shared_ptr<VulkanRenderPass>& renderPass,
                                                                   const std::vector<std::shared_ptr<VulkanImageView>>& attachments,
                                                                   const std::function<void(VulkanFramebufferBuilder &)>
                                                                   &builderFunc)
{
    const auto device = shared_from_this();

    VulkanFramebufferBuilder builder;
    builderFunc(builder);

    return builder.Build(device, renderPass, attachments);
}

std::shared_ptr<VulkanImage> VulkanDevice::CreateImage(const std::function<void(VulkanImageBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanImageBuilder builder;
    builderFunc(builder);

    return builder.Build(device);
}

std::shared_ptr<VulkanImageView> VulkanDevice::CreateImageView(const std::shared_ptr<VulkanImage>& image,
                                                               const std::function<void(VulkanImageViewBuilder&)> &
                                                               builderFunc)
{
    const auto device = shared_from_this();

    VulkanImageViewBuilder builder;
    builderFunc(builder);

    return builder.Build(device, image);
}

std::shared_ptr<VulkanShaderModule> VulkanDevice::CreateShaderModule(const std::vector<std::uint32_t>& moduleCode)
{
    auto device = shared_from_this();

    VkShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = moduleCode.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = moduleCode.data();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device->GetHandle(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        std::cout << "Failed to create shader module!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanShaderModule>(device, shaderModule);
}

std::shared_ptr<VulkanPipelineLayout> VulkanDevice::CreatePipelineLayout(
    const std::vector<std::shared_ptr<VulkanDescriptorSetLayout> > &descSetLayouts,
    const std::vector<VkPushConstantRange> &pushConstantRanges,
    const VkPipelineLayoutCreateFlags &createFlags)
{
    auto device = shared_from_this();

    VkPipelineLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = createFlags;

    createInfo.setLayoutCount = descSetLayouts.size();

    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts(descSetLayouts.size());
    for (uint32_t i = 0; i < descSetLayouts.size(); i++) {
        vkDescriptorSetLayouts[i] = descSetLayouts[i]->GetHandle();
    }
    createInfo.pSetLayouts = vkDescriptorSetLayouts.empty() ? nullptr : vkDescriptorSetLayouts.data();

    createInfo.pushConstantRangeCount = pushConstantRanges.size();
    createInfo.pPushConstantRanges = pushConstantRanges.empty() ? nullptr : pushConstantRanges.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    if (vkCreatePipelineLayout(device->GetHandle(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanPipelineLayout>(device, pipelineLayout);
}

std::shared_ptr<VulkanPipeline> VulkanDevice::CreateGraphicsPipeline(std::shared_ptr<VulkanPipelineLayout> layout,
    std::shared_ptr<VulkanRenderPass> renderPass, const std::function<void(VulkanGraphicsPipelineBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanGraphicsPipelineBuilder graphicsPipelineBuilder;
    builderFunc(graphicsPipelineBuilder);

    return graphicsPipelineBuilder.Build(device, layout, renderPass);
}

std::shared_ptr<VulkanBuffer> VulkanDevice::CreateBuffer(const std::function<void(VulkanBufferBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanBufferBuilder builder;
    builderFunc(builder);

    return builder.Build(device);
}

std::shared_ptr<VulkanDeviceMemory> VulkanDevice::AllocateMemory(const VkDeviceSize &size,
    const std::uint32_t memoryTypeIndex)
{
    auto device = shared_from_this();

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    if (vkAllocateMemory(device->GetHandle(), &allocateInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate memory!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanDeviceMemory>(device, deviceMemory);
}

void VulkanDevice::UpdateDescriptorSets(const std::vector<VkWriteDescriptorSet> &writeDescriptorSets,
                                        const std::vector<VkCopyDescriptorSet> &copyDescriptorSets) const
{
    vkUpdateDescriptorSets(handle_, writeDescriptorSets.size(),
                           writeDescriptorSets.empty() ? nullptr : writeDescriptorSets.data(),
                           copyDescriptorSets.size(),
                           copyDescriptorSets.empty() ? nullptr : copyDescriptorSets.data());
}

std::shared_ptr<VulkanSampler> VulkanDevice::CreateSampler(
    const std::function<void(VulkanSamplerBuilder &)> &builderFunc)
{
    const auto device = shared_from_this();

    VulkanSamplerBuilder builder;
    builderFunc(builder);

    return builder.Build(device);
}

VulkanDeviceBuilder::VulkanDeviceBuilder()
        : createInfo(GetDefaultDeviceCreateInfo())
{}

VulkanDeviceBuilder & VulkanDeviceBuilder::AddQueueInfo(
    const std::function<void(VkDeviceQueueCreateInfo &)> &setterFunc)
{
    VkDeviceQueueCreateInfo queueCreateInfo = GetDefaultQueueCreateInfo();
    setterFunc(queueCreateInfo);
    queueCreateInfos_.push_back(queueCreateInfo);
    return *this;
}

VulkanDeviceBuilder & VulkanDeviceBuilder::AddLayer(const std::string &layerName)
{
    layers_.emplace_back(layerName);
    return *this;
}

VulkanDeviceBuilder & VulkanDeviceBuilder::AddLayers(const std::vector<std::string> &layerNames)
{
    layers_.insert(layers_.end(), layerNames.begin(), layerNames.end());
    return *this;
}

VulkanDeviceBuilder & VulkanDeviceBuilder::AddExtension(const std::string &extensionName)
{
    extensions_.emplace_back(extensionName);
    return *this;
}

VulkanDeviceBuilder & VulkanDeviceBuilder::AddExtensions(const std::vector<std::string> &extensionNames)
{
    extensions_.insert(extensions_.end(), extensionNames.begin(), extensionNames.end());
    return *this;
}

VulkanDeviceBuilder & VulkanDeviceBuilder::SetDeviceFeatures(const VkPhysicalDeviceFeatures &features)
{
    deviceFeatures_ = features;
    return *this;
}

std::shared_ptr<VulkanDevice> VulkanDeviceBuilder::Build(std::shared_ptr<VulkanPhysicalDevice> physicalDevice)
{
    if (!queueCreateInfos_.empty()) {
        createInfo.queueCreateInfoCount = queueCreateInfos_.size();
        createInfo.pQueueCreateInfos = queueCreateInfos_.data();
    }

    std::vector<const char*> layersStr;
    if (!layers_.empty()) {

        std::ranges::transform(layers_, std::back_inserter(layersStr),
                           [](const std::string& s) { return s.c_str(); });
        createInfo.enabledLayerCount = layersStr.size();
        createInfo.ppEnabledLayerNames = layersStr.data();
    }

    std::vector<const char*> extensionsStr;
    if (!extensions_.empty()) {
        std::ranges::transform(extensions_, std::back_inserter(extensionsStr),
                           [](const std::string& s) { return s.c_str(); });
        createInfo.enabledExtensionCount = extensionsStr.size();
        createInfo.ppEnabledExtensionNames = extensionsStr.data();
    }

    if (deviceFeatures_.has_value()) {
        createInfo.pEnabledFeatures = &deviceFeatures_.value();
    }

    VkDevice device = VK_NULL_HANDLE;
    if (vkCreateDevice(physicalDevice->GetHandle(), &createInfo, nullptr, &device) != VK_SUCCESS) {
        std::cout << "Failed to create logical device!" << std::endl;
        return nullptr;
    }


    return std::make_shared<VulkanDevice>(physicalDevice, device);
}
} // common::vulkan_wrapper