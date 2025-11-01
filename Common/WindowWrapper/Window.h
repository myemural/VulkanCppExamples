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

#include <functional>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "GLFW/glfw3.h"

#include "CoreDefines.h"
#include "InputDispatcher.h"

namespace common::window_wrapper
{

class COMMON_API Window
{
public:
    /**
     * @param windowName Title of the window.
     */
    explicit Window(std::string windowName);

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
     * @brief Disables cursor on the window.
     */
    void DisableCursor() const;

    /**
     * @brief Checks window close event has occurred or not.
     * @return Returns true if close event has occurred, otherwise false.
     */
    [[nodiscard]] bool CheckWindowCloseFlag() const;

    /**
     * @brief Used to polling call of the input events.
     */
    void PollEvents() const;

    /**
     * @brief Used for swapping buffers of the window.
     */
    void SwapBuffers() const;

    /**
     * @brief Checks the state of a keyboard key.
     * @param key Specifies keyboard key code.
     * @return Returns true if keyboard key is pressed, otherwise false.
     */
    [[nodiscard]] bool IsKeyPressed(int key) const;

    /**
     * @brief Checks the state of a mouse button.
     * @param button Specifies mouse button code.
     * @return Returns true if mouse button is pressed, otherwise false.
     */
    [[nodiscard]] bool IsMouseButtonPressed(int button) const;

    /**
     * @brief Assigns a callback to the window that will be invoked when any keyboard key is pressed.
     * @param callback Specifies the callback that will be invoked when any keyboard key is pressed.
     */
    void OnKey(std::function<void(const KeyEvent&)> callback);

    /**
     * @brief Assigns a callback to the window that will be invoked when any mouse button is pressed.
     * @param callback Specifies the callback that will be called when any mouse button is pressed.
     */
    void OnMouseButton(std::function<void(const MouseButtonEvent&)> callback);

    /**
     * @brief Assigns a callback to the window that will be invoked when the mouse moves on the screen.
     * @param callback Specifies the callback to be called when the mouse moves on the screen.
     */
    void OnMouseMove(std::function<void(const MouseMoveEvent&)> callback);

    /**
     * @brief Assigns a callback to the window that will be invoked when the mouse is scrolled.
     * @param callback Specifies the callback to be called when the mouse is scrolled.
     */
    void OnMouseScroll(std::function<void(const MouseScrollEvent&)> callback);

private:
    std::string windowName_;
    std::uint32_t windowWidth_ = 0;
    std::uint32_t windowHeight_ = 0;
    GLFWwindow* window_;
    InputDispatcher inputDispatcher_;
};

} // namespace common::window_wrapper
