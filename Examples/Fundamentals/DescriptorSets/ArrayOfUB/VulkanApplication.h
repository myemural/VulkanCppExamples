/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    24.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationDescriptorSets.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanDescriptorSet.h"
#include "Window.h"

namespace examples::fundamentals::descriptor_sets::array_of_ub
{
class VulkanApplication final : public base::ApplicationDescriptorSets
{
public:
    explicit VulkanApplication(common::utility::ParameterServer &&params);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreatePipeline();

    void CreateDescriptorPool();

    void CreateDescriptorSetLayout();

    void CreateDescriptorSet();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t indexCount);

    void UpdateUniformBuffers(float currentTime);

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = 0;
    std::uint32_t currentWindowHeight_ = 0;

    std::vector<base::BufferCreateInfo> bufferCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_;

    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSetLayout> descriptorSetLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorPool> descriptorPool_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSet> descriptorSet_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffers_;
};
} // namespace examples::fundamentals::descriptor_sets::array_of_ub
