/**
 * @file    VulkanApplication.cpp
 * @brief   This file contains VulkanApplication and VulkanApplicationSettings implementations.
 * @author  Mustafa Yemural (myemural)
 * @date    11.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <memory>

#include "ApplicationBasics.h"

namespace examples::fundamentals::basics::clear_screen_with_color
{
// Application constants
inline constexpr std::uint32_t kMaxFramesInFlight = 2;

// User customizable settings
struct ApplicationSettings
{
    VkClearColorValue ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

class VulkanApplication final : public base::ApplicationBasics
{
public:
    VulkanApplication(const common::vulkan_framework::ApplicationCreateConfig &config, const ApplicationSettings& settings);

protected:
    bool Init() override;

    void DrawFrame() override;

    void CreateCommandBuffers();

    void RecordCommandBuffers();

    ApplicationSettings settings_;
    std::uint32_t currentIndex_ = 0;
    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer>> cmdBuffers_;
};

} // namespace examples::fundamentals::basics::clear_screen_with_color