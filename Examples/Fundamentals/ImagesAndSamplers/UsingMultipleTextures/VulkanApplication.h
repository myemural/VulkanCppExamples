/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    01.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationImagesAndSamplers.h"
#include "ApplicationData.h"
#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::images_and_samplers::using_multiple_textures
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderBaseType::GLSL;
inline constexpr auto kVertexShaderFileName = "multiple_textures.vert.spv";
inline constexpr auto kFragmentShaderFileName = "multiple_textures.frag.spv";
inline constexpr auto kVertexShaderHash = "vertMain";
inline constexpr auto kFragmentShaderHash = "fragMain";
inline constexpr auto kVertexBufferKey = "mainVertexBuffer";
inline constexpr auto kIndexBufferKey = "mainIndexBuffer";
inline constexpr auto kBricksTexStagingBufferKey = "bricksTexStagingBuffer";
inline constexpr auto kWallTexStagingBufferKey = "wallTexStagingBuffer";

inline constexpr auto kMainDescSetLayoutKey = "mainDescSetLayout";

inline constexpr auto kTextureBricksPath = "Textures/bricks.jpg";
inline constexpr auto kTextureWallPath = "Textures/wall.jpg";

class VulkanApplication final : public base::ApplicationImagesAndSamplers
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void Cleanup() override;

    void CreatePipeline();

    void UpdateDescriptorSets();

    void CreateTextureImages();

    void CreateTextureImageViews();

    void CreateSampler();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffers();

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;
    PushConstantData pushConstantData_[3] = {};

    // Create infos
    std::vector<base::BufferCreateInfo> bufferCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_;
    base::DescriptorSetCreateInfo descriptorSetCreateInfo_;
    base::DescriptorSetUpdateInfo descriptorSetUpdateInfo_;

    // Texture resource
    common::utility::TextureHandler bricksTextureHandler_{};
    common::utility::TextureHandler wallTextureHandler_{};
    std::shared_ptr<common::vulkan_wrapper::VulkanImage> bricksTexImage_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> bricksTexDeviceMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImageView> bricksTexImageView_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImage> wallTexImage_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> wallTexDeviceMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImageView> wallTexImageView_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSampler> sampler_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;
};
} // namespace examples::fundamentals::images_and_samplers::using_multiple_textures