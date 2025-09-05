/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    31.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationImagesAndSamplers.h"
#include "ShaderLoader.h"
#include "VulkanInstance.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::images_and_samplers::combined_image_sampler
{

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

// Project constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;
inline constexpr auto kCurrentShaderType = common::utility::ShaderBaseType::GLSL;
inline constexpr auto kVertexShaderFileName = "textured_quad.vert.spv";
inline constexpr auto kFragmentShaderFileName = "textured_quad.frag.spv";
inline constexpr auto kVertexShaderHash = "vertMain";
inline constexpr auto kFragmentShaderHash = "fragMain";
inline constexpr auto kVertexBufferKey = "mainVertexBuffer";
inline constexpr auto kIndexBufferKey = "mainIndexBuffer";
inline constexpr auto kTextureStagingBufferKey = "textureStagingBuffer";

inline constexpr auto kMainDescSetLayoutKey = "mainDescSetLayout";

inline constexpr auto kTextureBricksPath = "Textures/bricks.jpg";

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

    void CreateQuadTextureImage();

    void CreateQuadTextureImageView();

    void CreateSampler();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffer();

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;

    // Create infos
    std::vector<base::BufferCreateInfo> bufferCreateInfos_;
    base::ShaderModulesCreateInfo shaderModuleCreateInfo_;
    base::DescriptorSetCreateInfo descriptorSetCreateInfo_;
    base::DescriptorSetUpdateInfo descriptorSetUpdateInfo_;

    // Texture resource
    common::utility::TextureHandler bricksTextureHandler_{};
    std::shared_ptr<common::vulkan_wrapper::VulkanImage> quadTextureImage_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> textureDeviceMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImageView> quadTextureImageView_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSampler> sampler_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;
};
} // namespace examples::fundamentals::images_and_samplers::combined_image_sampler