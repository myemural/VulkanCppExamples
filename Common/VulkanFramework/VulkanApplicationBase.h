/**
 * @file    VulkanApplicationBase.h
 * @brief   This file contains base class of the all Vulkan applications.
 * @author  Mustafa Yemural (myemural)
 * @date    26.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include <vulkan/vulkan_core.h>

namespace common::vulkan_wrapper
{
class VulkanInstance;
}

namespace common::vulkan_framework
{

struct ApplicationCreateConfig
{
    std::string ApplicationName;
    std::uint32_t VulkanApiVersion = VK_API_VERSION_1_0;
    std::uint32_t ApplicationVersion = VK_MAKE_VERSION(1, 0, 0);
    std::string EngineName = "DefaultEngine";
    std::uint32_t EngineVersion = VK_MAKE_VERSION(1, 0, 0);
    std::vector<std::string> InstanceLayers;
    std::vector<std::string> InstanceExtensions;
    long long RenderLoopMs = 8LL;
};

class VulkanApplicationBase {
public:
    virtual ~VulkanApplicationBase() = default;

    /**
     * @param cfg Contains creation related configs for the application.
     */
    explicit VulkanApplicationBase(ApplicationCreateConfig  cfg);

    /**
     * @brief This function executes init, render loop and cleanup steps of the application.
     * @return Returns true if application ran successfully, otherwise false.
     */
    bool Run();

protected:
    /**
     * @brief Contains application initialization related codes.
     * @return Returns true if init success, otherwise false.
     */
    virtual bool Init() = 0;

    /**
     * @brief Contains the codes which executes every step of the render loop.
     */
    virtual void DrawFrame() = 0;

    /**
     * @brief Contains the codes which is not related to render but executes every end of the render loop.
     */
    virtual void Update() = 0;

    /**
     * @brief Contains the codes which executes completion after render loop.
     */
    virtual void Cleanup() = 0;

    /**
     * @brief Called in the render loop condition and checks application is done or not.
     * @return Returns true if application close state is satisfied, otherwise false.
     */
    virtual bool ShouldClose() = 0;

    std::shared_ptr<vulkan_wrapper::VulkanInstance> instance_;
    ApplicationCreateConfig applicationConfig_;

private:
    bool CreateInstance();
};

} // common::vulkan_framework