/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "VulkanQueue.h"

#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanSemaphore.h"
#include "VulkanSwapChain.h"

namespace common::vulkan_wrapper
{
VulkanQueue::VulkanQueue(std::shared_ptr<VulkanDevice> device, VkQueue queue) : VulkanObject{std::move(device), queue}
{
}

void VulkanQueue::Submit(const std::vector<std::shared_ptr<VulkanCommandBuffer>>& cmdBuffers,
                         const std::vector<std::shared_ptr<VulkanSemaphore>>& waitSemaphores,
                         const std::vector<std::shared_ptr<VulkanSemaphore>>& signalSemaphores,
                         const std::shared_ptr<VulkanFence>& fence,
                         const std::vector<VkPipelineStageFlags>& waitStages) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    std::vector<VkCommandBuffer> vkCmdBuffers(cmdBuffers.size());
    for (size_t i = 0; i < cmdBuffers.size(); i++) {
        vkCmdBuffers[i] = cmdBuffers[i]->GetHandle();
    }
    submitInfo.commandBufferCount = vkCmdBuffers.size();
    submitInfo.pCommandBuffers = vkCmdBuffers.empty() ? nullptr : vkCmdBuffers.data();

    std::vector<VkSemaphore> vkWaitSemaphores(waitSemaphores.size());
    for (size_t i = 0; i < waitSemaphores.size(); i++) {
        vkWaitSemaphores[i] = waitSemaphores[i]->GetHandle();
    }
    submitInfo.waitSemaphoreCount = vkWaitSemaphores.size();
    submitInfo.pWaitSemaphores = vkWaitSemaphores.empty() ? nullptr : vkWaitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.empty() ? nullptr : waitStages.data();

    std::vector<VkSemaphore> vkSignalSemaphores(signalSemaphores.size());
    for (size_t i = 0; i < signalSemaphores.size(); i++) {
        vkSignalSemaphores[i] = signalSemaphores[i]->GetHandle();
    }
    submitInfo.signalSemaphoreCount = vkSignalSemaphores.size();
    submitInfo.pSignalSemaphores = vkSignalSemaphores.empty() ? nullptr : vkSignalSemaphores.data();

    VkFence queueSubmitFence = fence != nullptr ? fence->GetHandle() : VK_NULL_HANDLE;
    if (vkQueueSubmit(handle_, 1, &submitInfo, queueSubmitFence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit command buffer to queue!");
    }
}

void VulkanQueue::Present(const std::vector<std::shared_ptr<VulkanSwapChain>>& swapChains,
                          const std::vector<std::uint32_t>& swapChainImageIndices,
                          const std::vector<std::shared_ptr<VulkanSemaphore>>& waitSemaphores)
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    std::vector<VkSemaphore> vkWaitSemaphores(waitSemaphores.size());
    for (size_t i = 0; i < waitSemaphores.size(); i++) {
        vkWaitSemaphores[i] = waitSemaphores[i]->GetHandle();
    }
    presentInfo.waitSemaphoreCount = vkWaitSemaphores.size();
    presentInfo.pWaitSemaphores = vkWaitSemaphores.empty() ? nullptr : vkWaitSemaphores.data();

    std::vector<VkSwapchainKHR> vkSwapChains(swapChains.size());
    for (size_t i = 0; i < swapChains.size(); i++) {
        vkSwapChains[i] = swapChains[i]->GetHandle();
    }
    presentInfo.swapchainCount = vkSwapChains.size();
    presentInfo.pSwapchains = vkSwapChains.empty() ? nullptr : vkSwapChains.data();

    presentInfo.pImageIndices = swapChainImageIndices.data();
    presentInfo.pResults = nullptr; /// TODO: Advanced queue handling will be added later

    presentResult_ = vkQueuePresentKHR(handle_, &presentInfo);
    if (presentResult_ != VK_SUCCESS && presentResult_ != VK_SUBOPTIMAL_KHR &&
        presentResult_ != VK_ERROR_OUT_OF_DATE_KHR) {
        throw std::runtime_error("Failed to present queue!");
    }
}

void VulkanQueue::WaitIdle() const
{
    if (vkQueueWaitIdle(handle_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to wait queue!");
    }
}

VkResult VulkanQueue::GetPresentResult() const { return presentResult_; }
} // namespace common::vulkan_wrapper
