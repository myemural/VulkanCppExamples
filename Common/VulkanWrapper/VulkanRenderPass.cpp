/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanRenderPass.h"

#include "VulkanDevice.h"

namespace common::vulkan_wrapper
{
inline VkRenderPassCreateInfo GetDefaultRenderPassCreateInfo()
{
    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.attachmentCount = 0;
    info.pAttachments = nullptr;
    info.subpassCount = 0;
    info.pSubpasses = nullptr;
    info.dependencyCount = 0;
    info.pDependencies = nullptr;
    return info;
}

inline VkAttachmentDescription GetDefaultAttachmentDescription()
{
    VkAttachmentDescription description{};
    description.flags = 0;
    description.format = VK_FORMAT_B8G8R8A8_SRGB;
    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return description;
}

inline VkSubpassDescription GetDefaultSubpassDescription()
{
    VkSubpassDescription description{};
    description.flags = 0;
    description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    description.inputAttachmentCount = 0;
    description.pInputAttachments = nullptr;
    description.colorAttachmentCount = 0;
    description.pColorAttachments = nullptr;
    description.pResolveAttachments = nullptr;
    description.pDepthStencilAttachment = nullptr;
    description.preserveAttachmentCount = 0;
    description.pPreserveAttachments = nullptr;
    return description;
}

inline VkSubpassDependency GetDefaultSubpassDependency()
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = 0;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    dependency.srcAccessMask = VK_ACCESS_NONE;
    dependency.dstAccessMask = VK_ACCESS_NONE;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    return dependency;
}

VulkanRenderPass::VulkanRenderPass(std::shared_ptr<VulkanDevice> device, VkRenderPass renderPass)
    : VulkanObject(std::move(device), renderPass)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyRenderPass(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VkExtent2D VulkanRenderPass::GetRenderAreaGranularity() const
{
    VkExtent2D granularity{};
    if (const auto device = GetParent()) {
        vkGetRenderAreaGranularity(device->GetHandle(), handle_, &granularity);
    }

    return granularity;
}

VulkanRenderPassBuilder::VulkanRenderPassBuilder()
    : createInfo_(GetDefaultRenderPassCreateInfo())
{
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::SetCreateFlags(const VkRenderPassCreateFlags &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::AddAttachment(
    const std::function<void(VkAttachmentDescription &)> &setterFunc)
{
    VkAttachmentDescription description = GetDefaultAttachmentDescription();
    setterFunc(description);
    attachments_.push_back(description);
    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::AddSubpass(
    const std::function<void(VkSubpassDescription &)> &setterFunc)
{
    VkSubpassDescription description = GetDefaultSubpassDescription();
    setterFunc(description);
    subpasses_.push_back(description);
    return *this;
}

VulkanRenderPassBuilder &VulkanRenderPassBuilder::AddDependency(
    const std::function<void(VkSubpassDependency &)> &setterFunc)
{
    VkSubpassDependency dependency = GetDefaultSubpassDependency();
    setterFunc(dependency);
    dependencies_.push_back(dependency);
    return *this;
}

std::shared_ptr<VulkanRenderPass> VulkanRenderPassBuilder::Build(std::shared_ptr<VulkanDevice> device)
{
    if (!attachments_.empty()) {
        createInfo_.attachmentCount = attachments_.size();
        createInfo_.pAttachments = attachments_.data();
    }

    if (!subpasses_.empty()) {
        createInfo_.subpassCount = subpasses_.size();
        createInfo_.pSubpasses = subpasses_.data();
    }

    if (!dependencies_.empty()) {
        createInfo_.dependencyCount = dependencies_.size();
        createInfo_.pDependencies = dependencies_.data();
    }

    VkRenderPass renderPass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(device->GetHandle(), &createInfo_, nullptr, &renderPass) != VK_SUCCESS) {
        std::cerr << "Failed to create render pass!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanRenderPass>(std::move(device), renderPass);
}
} // common::vulkan_wrapper
