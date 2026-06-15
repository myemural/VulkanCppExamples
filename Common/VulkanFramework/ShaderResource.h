/**
 * @file    ShaderResource.h
 * @brief   This class contains a registry about all shader modules.
 * @author  Mustafa Yemural (myemural)
 * @date    23.09.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <map>
#include <string>

#include "CoreDefines.h"
#include "ShaderAsset.h"
#include "VulkanDevice.h"

namespace common::vulkan_framework
{
struct COMMON_API ShaderModulesCreateInfo
{
    struct Module
    {
        std::string name;
        asset_manager::ShaderAsset asset;
    };

    std::vector<Module> modules;
};

class COMMON_API ShaderResource
{
public:
    /**
     * @param device Refers VulkanDevice object.
     */
    explicit ShaderResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device);

    /**
     * @brief Creates a shader module resource from given information.
     * @param createInfo Shader module create information.
     */
    void CreateShaders(const ShaderModulesCreateInfo& createInfo);

    /**
     * @brief Returns the shader module resource.
     * @param name Name of the shader module resource.
     * @return Returns the shader module resource.
     */
    [[nodiscard]] std::shared_ptr<vulkan_wrapper::VulkanShaderModule> GetShaderModule(const std::string& name)
    {
        if (!shaderModules_.contains(name)) {
            throw std::runtime_error("Shader module \"" + name + "\" not found!");
        }

        return shaderModules_[name];
    }

    /**
     * @brief Deletes shader module from the resource.
     * @param moduleName Shader module name.
     */
    void DeleteShaderModule(const std::string& moduleName);

private:
    std::weak_ptr<vulkan_wrapper::VulkanDevice> device_;

    std::map<std::string, std::shared_ptr<vulkan_wrapper::VulkanShaderModule>> shaderModules_;
};
} // namespace common::vulkan_framework
