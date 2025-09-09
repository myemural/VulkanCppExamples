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

    glfwSetWindowUserPointer(window_, this);

    glfwSetKeyCallback(window_, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (const auto self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
            self->CallKeyCallback(key, scancode, action, mods);
        }

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwSetCursorPosCallback(window_, [](GLFWwindow* window, double x, double y) {
        if (const auto self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
            self->CallMouseCallback(x, y);
        }
    });

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

void Window::SetKeyCallback(const std::function<void(int, int, int, int)> &callback)
{
    keyCallback_ = callback;
}

void Window::CallKeyCallback(int key, int scancode, int action, int mods) const
{
    if (keyCallback_) {
        keyCallback_(key, scancode, action, mods);
    }
}

void Window::SetMouseCallback(const std::function<void(double, double)>& callback)
{
    mouseCallback_ = callback;
}

void Window::CallMouseCallback(double x, double y) const
{
    if (mouseCallback_) {
        mouseCallback_(x, y);
    }
}

void Window::DisableCursor() const
{
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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