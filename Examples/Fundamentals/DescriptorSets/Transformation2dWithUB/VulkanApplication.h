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
#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanDescriptorSet.h"
#include "Window.h"

namespace examples::fundamentals::descriptor_sets::transformation2d_with_ub
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderType::GLSL;
inline constexpr auto kVertexShaderFileName = "transform_2d.vert.spv";
inline constexpr auto kFragmentShaderFileName = "transform_2d.frag.spv";
inline constexpr auto kVertexShaderHash = "vert_main";
inline constexpr auto kFragmentShaderHash = "frag_main";
inline constexpr auto kVertexBufferKey = "mainVertexBuffer";
inline constexpr auto kIndexBufferKey = "mainIndexBuffer";
inline constexpr auto kModelUBKey = "modelUB";

class VulkanApplication final : public base::ApplicationDescriptorSets
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreateDescriptorSetLayout();

    void CreateDescriptorPool();

    void CreateDescriptorSet();

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t indexCount);

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;
    std::vector<base::BufferCreateInfo> bufferCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSetLayout> descriptorSetLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorPool> descriptorPool_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDescriptorSet> descriptorSet_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;
};
} // namespace examples::fundamentals::descriptor_sets::transformation2d_with_ub