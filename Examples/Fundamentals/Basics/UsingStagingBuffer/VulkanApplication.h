/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    22.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBasics.h"
#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "VulkanShaderModule.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanBuffer.h"
#include "VulkanDeviceMemory.h"
#include "Window.h"

namespace examples::fundamentals::basics::using_staging_buffer
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderBaseType::GLSL;
inline constexpr auto kVertexShaderFileName = "triangle.vert.spv";
inline constexpr auto kFragmentShaderFileName = "triangle.frag.spv";
inline constexpr auto kVertexShaderHash = "vert_main";
inline constexpr auto kFragmentShaderHash = "frag_main";

class VulkanApplication final : public base::ApplicationBasics
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreateVertexBuffer(std::uint64_t dataSize);

    void CreateStagingBuffer(std::uint64_t dataSize);

    void FillStagingBuffer(const void* data, std::uint64_t dataSize) const;

    void CreateShaderModules();

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffer(std::uint64_t dataSize);

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;
    std::shared_ptr<common::vulkan_wrapper::VulkanBuffer> vertexBuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> vertexBufferMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanBuffer> stagingBuffer_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> stagingBufferMemory_;
    std::unordered_map<std::string, std::shared_ptr<common::vulkan_wrapper::VulkanShaderModule>> shaderModules_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;

};
} // namespace examples::fundamentals::basics::using_staging_buffer