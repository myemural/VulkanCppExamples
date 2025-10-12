/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    03.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationImagesAndSamplers.h"
#include "TextureLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::fundamentals::images_and_samplers::texture_atlases
{
class VulkanApplication final : public base::ApplicationImagesAndSamplers
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void CreateResources();

    void InitResources();

    void CreatePipeline();

    void UpdateDescriptorSets() const;

    void CreateQuadTextureImage();

    void CreateQuadTextureImageView();

    void CreateSampler();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffer();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    PushConstantData pushConstantData_[4] = {};

    // Texture resource
    common::utility::TextureHandler atlasTextureHandler_{};
    std::shared_ptr<common::vulkan_wrapper::VulkanImage> quadTextureImage_;
    std::shared_ptr<common::vulkan_wrapper::VulkanDeviceMemory> textureDeviceMemory_;
    std::shared_ptr<common::vulkan_wrapper::VulkanImageView> quadTextureImageView_;
    std::shared_ptr<common::vulkan_wrapper::VulkanSampler> sampler_;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;
};
} // namespace examples::fundamentals::images_and_samplers::texture_atlases
