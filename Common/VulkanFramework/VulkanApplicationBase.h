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

#include <memory>

#include "ParameterServer.h"
#include "VulkanInstance.h"

namespace common::vulkan_framework
{
class VulkanApplicationBase
{
public:
    virtual ~VulkanApplicationBase() = default;

    /**
     * @param params Contains application parameters.
     */
    explicit VulkanApplicationBase(utility::ParameterServer params);

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

    /**
     * @brief Returns std::string parameter from parameter server.
     * @param key Key name of the parameter.
     * @return Returns value of the parameter.
     */
    [[nodiscard]] std::string GetParamStr(const std::string& key) const;

    /**
     * @brief Returns std::uint32_t parameter from parameter server.
     * @param key Key name of the parameter.
     * @return Returns value of the parameter.
     */
    [[nodiscard]] std::uint32_t GetParamU32(const std::string& key) const;

    /**
     * @brief Returns float parameter from parameter server.
     * @param key Key name of the parameter.
     * @return Returns value of the parameter.
     */
    [[nodiscard]] float GetParamFloat(const std::string& key) const;

    utility::ParameterServer params_;
    std::shared_ptr<vulkan_wrapper::VulkanInstance> instance_;

private:
    bool CreateInstance();
};
} // common::vulkan_framework
