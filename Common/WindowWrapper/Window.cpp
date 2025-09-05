/**
* Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "Window.h"

#include <iostream>
#include <utility>

namespace common::window_wrapper
{

Window::Window(std::string  windowName)
    : windowName_{std::move(windowName)}, window_{nullptr}
{
    glfwInit();
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::Init(const uint32_t windowWidth, const uint32_t windowHeight, const bool isResizable, const uint32_t sampleCount)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, static_cast<int>(sampleCount));

    windowWidth_ = windowWidth;
    windowHeight_ = windowHeight;

    window_ = glfwCreateWindow(static_cast<int>(windowWidth), static_cast<int>(windowHeight), windowName_.c_str(),
                               nullptr, nullptr);

    if(!window_)
    {
        std::cout << "Failed to create a window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);

    return true;
}

std::vector<std::string> Window::GetVulkanInstanceExtensions() {
    std::vector<std::string> extensions;
    uint32_t count;
    const char** surfaceExtensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t idx = 0; idx < count; idx++) {
        extensions.emplace_back(surfaceExtensions[idx]);
    }
    return extensions;
}

VkSurfaceKHR Window::CreateVulkanSurface(VkInstance instance) const
{
    VkSurfaceKHR surface;
    if (const auto test = glfwCreateWindowSurface(instance, window_, nullptr, &surface); test != VK_SUCCESS) {
        std::cout << "Failed to create window surface!" << '\n';
        return VK_NULL_HANDLE;
    }

    return surface;
}

void Window::SetWindowResizeCallback(const GLFWframebuffersizefun callback) const
{
    glfwSetFramebufferSizeCallback(window_, callback);
}

void Window::SetMouseCallback(const GLFWcursorposfun callback) const
{
    glfwSetCursorPosCallback(window_, callback);
}

bool Window::CheckWindowCloseFlag() const
{
    return static_cast<bool>(glfwWindowShouldClose(window_));
}

void Window::OnUpdate() const
{
    glfwSwapBuffers(window_);
    glfwPollEvents();
}
} // namespace common::window_wrapper