/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanFramebuffer.h"

#include "VulkanDevice.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"

namespace common::vulkan_wrapper
{
inline VkFramebufferCreateInfo GetDefaultFramebufferCreateInfo()
{
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = nullptr;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments = nullptr;
    createInfo.width = 0;
    createInfo.height = 0;
    createInfo.layers = 1;
    return createInfo;
}

VulkanFramebuffer::VulkanFramebuffer(std::shared_ptr<VulkanDevice> device, VkFramebuffer framebuffer)
    : VulkanObject(std::move(device), framebuffer)
{
}

VulkanFramebuffer::~VulkanFramebuffer()
{
    if (handle_ != VK_NULL_HANDLE) {
        if (const auto device = GetParent()) {
            vkDestroyFramebuffer(device->GetHandle(), handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }
}

VulkanFramebufferBuilder::VulkanFramebufferBuilder()
    : createInfo_(GetDefaultFramebufferCreateInfo())
{
}

VulkanFramebufferBuilder &VulkanFramebufferBuilder::SetCreateFlags(const VkFramebufferCreateFlags &flags)
{
    createInfo_.flags = flags;
    return *this;
}

VulkanFramebufferBuilder &VulkanFramebufferBuilder::SetDimensions(const std::uint32_t width, const std::uint32_t height,
                                                                  const std::uint32_t layers)
{
    createInfo_.width = width;
    createInfo_.height = height;
    createInfo_.layers = layers;
    return *this;
}

std::shared_ptr<VulkanFramebuffer> VulkanFramebufferBuilder::Build(std::shared_ptr<VulkanDevice> device,
                                                                   const std::shared_ptr<VulkanRenderPass> &renderPass,
                                                                   const std::vector<std::shared_ptr<VulkanImageView> >
                                                                   &attachments)
{
    createInfo_.renderPass = renderPass->GetHandle();
    createInfo_.attachmentCount = static_cast<uint32_t>(attachments.size());
    std::vector<VkImageView> attachmentViews(attachments.size());
    for (size_t i = 0; i < attachments.size(); i++) {
        attachmentViews[i] = attachments[i]->GetHandle();
    }
    createInfo_.pAttachments = attachmentViews.data();

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    if (vkCreateFramebuffer(device->GetHandle(), &createInfo_, nullptr, &framebuffer) != VK_SUCCESS) {
        std::cout << "Failed to create framebuffer!" << std::endl;
        return nullptr;
    }

    return std::make_shared<VulkanFramebuffer>(std::move(device), framebuffer);
}
} // common::vulkan_wrapper
