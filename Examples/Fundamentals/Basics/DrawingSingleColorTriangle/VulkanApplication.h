/**
 * @file    VulkanApplication.cpp
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBasics.h"

#include "VulkanShaderModule.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineLayout.h"
#include "ParameterServer.h"
#include "Window.h"

namespace examples::fundamentals::basics::drawing_single_color_triangle
{
class VulkanApplication final : public base::ApplicationBasics
{
public:
    explicit VulkanApplication(common::utility::ParameterServer&& params);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreateShaderModules();

    void CreatePipeline();

    void CreateCommandBuffers();

    void RecordCommandBuffers();

    common::utility::ParameterServer params_;
    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;

    std::unordered_map<std::string, std::shared_ptr<common::vulkan_wrapper::VulkanShaderModule>> shaderModules_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipelineLayout> pipelineLayout_;
    std::shared_ptr<common::vulkan_wrapper::VulkanPipeline> pipeline_;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;
};

} // namespace examples::fundamentals::basics::drawing_single_color_triangle
