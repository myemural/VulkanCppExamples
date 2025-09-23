/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    02.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationImagesAndSamplers.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::images_and_samplers::simple_blending
{
class VulkanApplication final : public base::ApplicationImagesAndSamplers
{
public:
    explicit VulkanApplication(common::utility::ParameterServer &&params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void Cleanup() noexcept override;

    void CreatePipeline();

    void UpdateDescriptorSets();

    void CreateLeafTextureImage();

    void CreateLeafTextureImageView();

    void CreateSampler();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffer();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    PushConstantData pushConstantData_[4] = {};

    // Create infos
    std::vector<common::vulkan_framework::BufferResourceCreateInfo> bufferCreateInfos_;
    common::vulkan_framework::ShaderModulesCreateInfo shaderModuleCreateInfo_;
    common::vulkan_framework::DescriptorResourceCreateInfo descriptorSetCreateInfo_;
    common::vulkan_framework::DescriptorUpdateInfo descriptorSetUpdateInfo_;

    // Texture resource
    common::utility::TextureHandler leafTextureHandler_{};
    std::shared_ptr<common::vulkan_wrapper::VulkanImage> leafTexImage_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> leafDeviceMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImageView> leafTexImageView_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSampler> sampler_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;
};
} // namespace examples::fundamentals::images_and_samplers::simple_blending
