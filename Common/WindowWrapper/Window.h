/**
 * @file    Window.h
 * @brief   This file contains wrapper class for GLFW window handler.
 * @author  Mustafa Yemural (myemural)
 * @date    15.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"

class Window {
public:
    explicit Window(std::string  windowName);

    ~Window();

    bool Init(std::uint32_t windowWidth, std::uint32_t windowHeight, bool isResizable, std::uint32_t sampleCount);

    [[nodiscard]] GLFWwindow* GetGLFWWindow() const { return window_; }

    [[nodiscard]] static std::vector<std::string> GetVulkanInstanceExtensions();

    std::string GetWindowTitle() const { return windowName_; }

    std::uint32_t GetWindowWidth() const { return windowWidth_; }

    std::uint32_t GetWindowHeight() const { return windowHeight_; }

    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) const;

    void SetWindowResizeCallback(auto callback) const
    {
        glfwSetFramebufferSizeCallback(window_, callback);
    }

    void SetMouseCallback(auto callback) const
    {
        glfwSetCursorPosCallback(window_, callback);
    }

    [[nodiscard]] bool CheckWindowCloseFlag() const ;

    void OnUpdate() const ;
private:
    std::string windowName_;
    std::uint32_t windowWidth_ = 0;
    std::uint32_t windowHeight_ = 0;
    GLFWwindow *window_;
};
