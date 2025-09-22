/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    23.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include "ApplicationDescriptorSets.h"
#include "VulkanDescriptorSet.h"
#include "VulkanPipeline.h"

namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
{
class VulkanApplication final : public base::ApplicationDescriptorSets
{
public:
    explicit VulkanApplication(common::utility::ParameterServer &&params);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreateDescriptorSetLayout();

    void CreateDescriptorPool();

    void CreateDescriptorSet();

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t vertexCount);

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;

    std::vector<common::vulkan_framework::BufferResourceCreateInfo> bufferCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_;

    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSetLayout> descriptorSetLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorPool> descriptorPool_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSet> descriptorSet_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffers_;
};
} // namespace examples::fundamentals::descriptor_sets::changing_color_with_ub
