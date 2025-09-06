/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanCommandPool.h"

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

namespace common::vulkan_wrapper
{

inline VkCommandPoolCreateInfo GetDefaultCommandPoolCreateInfo()
{
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueFamilyIndex = 0;
    return createInfo;
}

VulkanCommandPool::VulkanCommandPool(std::shared_ptr<VulkanDevice> device, VkCommandPool cmdPool)
        : VulkanObject(std::move(device), cmdPool)
{}

VulkanCommandPool::~VulkanCommandPool()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyCommandPool(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

std::vector<std::shared_ptr<VulkanCommandBuffer> > VulkanCommandPool::CreateCommandBuffers(const std::uint32_t count,
    const VkCommandBufferLevel &level)
{
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = handle_;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = count;

    std::vector<VkCommandBuffer> cmdBuffers(count);
    const auto device = GetParent();
    if (!device || vkAllocateCommandBuffers(device->GetHandle(), &allocateInfo, cmdBuffers.data()) != VK_SUCCESS) {
        std::cout << "Failed to allocate command buffers!" << std::endl;
        return {};
    }

    std::vector<std::shared_ptr<VulkanCommandBuffer> > commandBuffers;
    for (auto &cmdBuffer: cmdBuffers) {
        auto vulkanCmdBuffer = std::make_shared<VulkanCommandBuffer>(shared_from_this(), cmdBuffer);
        commandBuffers.push_back(vulkanCmdBuffer);
    }

    return commandBuffers;
}

bool VulkanCommandPool::ResetCommandPool(const VkCommandPoolResetFlags &resetFlags) const
{
    const auto device = GetParent();
    if (!device || vkResetCommandPool(device->GetHandle(), handle_, resetFlags) != VK_SUCCESS) {
        std::cerr << "Failed to reset command pool!" << std::endl;
        return false;
    }

    return true;
}

VulkanCommandPoolBuilder::VulkanCommandPoolBuilder()
    : createInfo_(GetDefaultCommandPoolCreateInfo())
{
}

VulkanCommandPoolBuilder &VulkanCommandPoolBuilder::SetCreateFlags(const VkCommandPoolCreateFlags &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanCommandPoolBuilder &VulkanCommandPoolBuilder::SetQueueFamilyIndex(const std::uint32_t familyIndex)
{
    createInfo_.queueFamilyIndex = familyIndex;
    return *this;
}

std::shared_ptr<VulkanCommandPool> VulkanCommandPoolBuilder::Build(std::shared_ptr<VulkanDevice> device) const
{
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device->GetHandle(), &createInfo_, nullptr, &cmdPool) != VK_SUCCESS) {
        std::cerr << "Failed to create command pool!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanCommandPool>(std::move(device), cmdPool);
}
} // common::vulkan_wrapper