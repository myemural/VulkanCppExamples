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

namespace common::window_wrapper
{

class Window {
public:
    /**
     * @param windowName Title of the window.
     */
    explicit Window(std::string  windowName);

    ~Window();

    /**
     * @brief Initializes window object.
     * @param windowWidth Width of the window.
     * @param windowHeight Height of the window.
     * @param isResizable Specifies window is resizable or not.
     * @param sampleCount Multisampling sample count.
     * @return
     */
    bool Init(std::uint32_t windowWidth, std::uint32_t windowHeight, bool isResizable, std::uint32_t sampleCount);

    /**
     * @return Returns GLFW handler of the window.
     */
    [[nodiscard]] GLFWwindow* GetGLFWWindow() const { return window_; }

    /**
     * @return Returns Vulkan instance extension list that required for the window.
     */
    [[nodiscard]] static std::vector<std::string> GetVulkanInstanceExtensions();

    /**
     * @return Returns title of the window.
     */
    [[nodiscard]] std::string GetWindowTitle() const { return windowName_; }

    /**
     * @return Returns width of the window.
     */
    [[nodiscard]] std::uint32_t GetWindowWidth() const { return windowWidth_; }

    /**
     * @return Returns height of the window.
     */
    [[nodiscard]] std::uint32_t GetWindowHeight() const { return windowHeight_; }

    /**
     * @brief Creates and returns a Vulkan surface which related to window object.
     * @param instance Vulkan instance.
     * @return Returns Surface object.
     */
    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) const;

    /**
     * @brief Sets a callback function for resizing window event.
     * @param callback Callback function pointer that calling when window is resized.
     */
    void SetWindowResizeCallback(GLFWframebuffersizefun callback) const;

    /**
     * @brief Sets a callback function for mouse events.
     * @param callback Callback function pointer that calling when mouse event is occurred.
     */
    void SetMouseCallback(GLFWcursorposfun callback) const;

    /**
     * @brief Checks window close event has occurred or not.
     * @return Returns true if close event has occurred, otherwise false.
     */
    [[nodiscard]] bool CheckWindowCloseFlag() const ;

    /**
     * @brief Window related update function that will call in every frame.
     */
    void OnUpdate() const ;
private:
    std::string windowName_;
    std::uint32_t windowWidth_ = 0;
    std::uint32_t windowHeight_ = 0;
    GLFWwindow *window_;
};

} // namespace common::window_wrapper