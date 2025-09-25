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
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::images_and_samplers::using_multiple_textures
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

private:
    void CreateResources();

    void InitResources();

    void CreatePipeline();

    void UpdateDescriptorSets() const;

    void CreateTextureImages();

    void CreateTextureImageViews();

    void CreateSampler();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CopyStagingBuffers();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    PushConstantData pushConstantData_[3] = {};

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
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffersPresent_;
};
} // namespace examples::fundamentals::images_and_samplers::using_multiple_textures
