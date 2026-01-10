/**
 * @file    VulkanApplication.h
 * @brief   This file contains VulkanApplication class declaration.
 * @author  Mustafa Yemural (myemural)
 * @date    14.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBasicLighting.h"
#include "MaterialManager.h"
#include "PerspectiveCamera.h"
#include "SceneManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "Window.h"

namespace examples::real_time_lighting::basic_lighting::blinn_phong
{
class VulkanApplication final : public base::ApplicationBasicLighting
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

    ~VulkanApplication() override = default;

protected:
    bool Init() override;

    void DrawFrame() override;

    void PreUpdate() override;

private:
    void CreateInitialResources() const;

    void BuildScene();

    void UpdateDescriptorSets() const;

    void InitInputSystem();

    void CreateRenderPass();

    void CreatePipelines();

    void CreateCommandBuffers();

    void RecordPresentCommandBuffers(std::uint32_t currentImageIndex);

    void UpdateSceneTransforms() const;

    void ProcessInput() const;

    // Pipelines
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> scenePipeline_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> lightPipeline_;

    // Command buffers
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffersPresent_;

    // Mouse related values
    bool firstMouseTriggered_ = true;
    float lastX_ = 0.0f;
    float lastY_ = 0.0f;

    // Camera
    std::shared_ptr<common::utility::PerspectiveCamera> camera_ = nullptr;

    // Scene manager
    std::unique_ptr<common::vulkan_framework::SceneManager> scene_;
    std::unique_ptr<common::vulkan_framework::MaterialManager> materialManager_;
};
} // namespace examples::real_time_lighting::basic_lighting::blinn_phong
