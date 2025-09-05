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

namespace examples::fundamentals::descriptor_sets::basic_push_constants
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderBaseType::GLSL;
inline constexpr auto kVertexShaderFileName = "push_constants.vert.spv";
inline constexpr auto kFragmentShaderFileName = "push_constants.frag.spv";
inline constexpr auto kVertexShaderHash = "vert_main";
inline constexpr auto kFragmentShaderHash = "frag_main";
inline constexpr auto kVertexBufferKey = "mainVertexBuffer";
inline constexpr auto kIndexBufferKey = "mainIndexBuffer";

class VulkanApplication final : public base::ApplicationDescriptorSets
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t currentImageIndex, std::uint32_t indexCount);

    ApplicationSettings settings_;
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
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;
};
} // namespace examples::fundamentals::descriptor_sets::basic_push_constants