/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication and ApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    08.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationData.h"
#include "ApplicationDrawing3D.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"
#include "TextureLoader.h"

namespace examples::fundamentals::drawing_3d::drawing_cube
{
class VulkanApplication final : public base::ApplicationDrawing3D
{
public:
    explicit VulkanApplication(common::utility::ParameterServer &&params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void Cleanup() noexcept override;

    void InitResources(const VkFormat &depthImageFormat);

    void CreateRenderPass(const VkFormat &depthImageFormat);

    void CreatePipeline();

    void UpdateDescriptorSets();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t indexCount);

    void CalculateAndSetMvp();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;
    MvpData mvpUbObject{glm::mat4(1.0), glm::mat4(1.0), glm::mat4(1.0)};

    // Create infos
    std::vector<common::vulkan_framework::BufferResourceCreateInfo> bufferCreateInfos_;
    std::vector<common::vulkan_framework::ImageResourceCreateInfo> imageResourceCreateInfos_;
    std::vector<common::vulkan_framework::SamplerResourceCreateInfo> samplerResourceCreateInfos_;
    common::vulkan_framework::ShaderModulesCreateInfo shaderModuleCreateInfo_;
    common::vulkan_framework::DescriptorResourceCreateInfo descriptorSetCreateInfo_;
    common::vulkan_framework::DescriptorUpdateInfo descriptorSetUpdateInfo_;

    // Texture resource
    common::utility::TextureHandler crateTextureHandler_{};

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffersPresent_;
    std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> cmdBufferTransfer_;
};
} // namespace examples::fundamentals::drawing_3d::drawing_cube
