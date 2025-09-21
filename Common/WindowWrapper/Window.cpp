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
Window::Window(std::string windowName)
    : windowName_{std::move(windowName)}, window_{nullptr}
{
    glfwInit();
}

Window::~Window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Window::Init(const uint32_t windowWidth, const uint32_t windowHeight, const bool isResizable,
                  const uint32_t sampleCount)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, isResizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, static_cast<int>(sampleCount));

    windowWidth_ = windowWidth;
    windowHeight_ = windowHeight;

    window_ = glfwCreateWindow(static_cast<int>(windowWidth), static_cast<int>(windowHeight), windowName_.c_str(),
                               nullptr, nullptr);

    if (!window_) {
        std::cout << "Failed to create a window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);

    glfwSetWindowUserPointer(window_, this);

    glfwSetKeyCallback(window_, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (const auto self = static_cast<Window *>(glfwGetWindowUserPointer(window))) {
            self->inputDispatcher_.OnKeyEvent(key, action);
        }

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow *window, int button, int action, int mods) {
        if (const auto self = static_cast<Window *>(glfwGetWindowUserPointer(window))) {
            self->inputDispatcher_.OnMouseButtonEvent(button, action);
        }
    });

    glfwSetCursorPosCallback(window_, [](GLFWwindow *window, double x, double y) {
        if (const auto self = static_cast<Window *>(glfwGetWindowUserPointer(window))) {
            self->inputDispatcher_.OnMouseMoveEvent(x, y);
        }
    });

    glfwSetScrollCallback(window_, [](GLFWwindow *window, double xOffset, double yOffset) {
        if (const auto self = static_cast<Window *>(glfwGetWindowUserPointer(window))) {
            self->inputDispatcher_.OnMouseScrollEvent(xOffset, yOffset);
        }
    });

    return true;
}

std::vector<std::string> Window::GetVulkanInstanceExtensions()
{
    std::vector<std::string> extensions;
    uint32_t count;
    const char **surfaceExtensions = glfwGetRequiredInstanceExtensions(&count);
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

void Window::DisableCursor() const
{
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool Window::CheckWindowCloseFlag() const
{
    return static_cast<bool>(glfwWindowShouldClose(window_));
}

void Window::PollEvents() const
{
    glfwPollEvents();
}

void Window::SwapBuffers() const
{
    glfwSwapBuffers(window_);
}

bool Window::IsKeyPressed(const int key) const
{
    return inputDispatcher_.IsKeyPressed(key);
}

bool Window::IsMouseButtonPressed(const int button) const
{
    return inputDispatcher_.IsMouseButtonPressed(button);
}

void Window::OnKey(std::function<void(const KeyEvent &)> callback)
{
    inputDispatcher_.AddKeyListener(std::move(callback));
}

void Window::OnMouseButton(std::function<void(const MouseButtonEvent &)> callback)
{
    inputDispatcher_.AddMouseButtonListener(std::move(callback));
}

void Window::OnMouseMove(std::function<void(const MouseMoveEvent &)> callback)
{
    inputDispatcher_.AddMouseMoveListener(std::move(callback));
}

void Window::OnMouseScroll(std::function<void(const MouseScrollEvent &)> callback)
{
    inputDispatcher_.AddMouseScrollListener(std::move(callback));
}
} // namespace common::window_wrapper
