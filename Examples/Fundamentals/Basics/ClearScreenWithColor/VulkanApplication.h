/**
 * @file    VulkanApplication.cpp
 * @brief   This file contains VulkanApplication class declaration.
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
#include "ParameterServer.h"

namespace examples::fundamentals::basics::clear_screen_with_color
{
class VulkanApplication final : public base::ApplicationBasics
{
public:
    explicit VulkanApplication(common::utility::ParameterServer &&params);

protected:
    bool Init() override;

    void DrawFrame() override;

private:
    void CreateCommandBuffers();

    void RecordCommandBuffers();

    std::uint32_t currentIndex_ = 0;
    std::uint32_t currentWindowWidth_ = UINT32_MAX;
    std::uint32_t currentWindowHeight_ = UINT32_MAX;

    std::vector<std::shared_ptr<common::vulkan_wrapper::VulkanCommandBuffer> > cmdBuffers_;
};
} // namespace examples::fundamentals::basics::clear_screen_with_color
