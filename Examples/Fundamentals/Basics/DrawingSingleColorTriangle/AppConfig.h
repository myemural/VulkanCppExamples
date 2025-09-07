/**
 * @file    AppConfig.h
 * @brief   This header file keeps key names for user-provided configs.
 * @author  Mustafa Yemural (myemural)
 * @date    7.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

namespace examples::fundamentals::basics::drawing_single_color_triangle
{

// Parameter server key names
namespace WindowParams
{
    constexpr auto Width = "Window.Width";
    constexpr auto Height = "Window.Height";
    constexpr auto Title = "Window.Title";
    constexpr auto Resizable = "Window.Resizable";
    constexpr auto SampleCount = "Window.SampleCount";
}

namespace VulkanParams
{
    constexpr auto AppCreateConfig = "Vulkan.AppCreateConfig";
}

namespace ProjectParams
{
    constexpr auto MaxFramesInFlight = "Project.MaxFramesInFlight";
    constexpr auto VertexCount = "Project.VertexCount";
    constexpr auto BaseShaderType = "Project.BaseShaderType";
    constexpr auto MainVertexShaderFile = "Project.MainVertexShaderFile";
    constexpr auto MainFragmentShaderFile = "Project.MainFragmentShaderFile";
    constexpr auto MainVertexShaderKey = "Project.MainVertexShaderKey";
    constexpr auto MainFragmentShaderKey = "Project.MainFragmentShaderKey";
    constexpr auto ClearColor = "Project.ClearColor";
} // namespace ProjectParams

} // namespace examples::fundamentals::basics::drawing_single_color_triangle
