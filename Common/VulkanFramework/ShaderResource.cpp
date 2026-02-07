/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ShaderResource.h"

#include "ShaderLoader.h"

namespace common::vulkan_framework
{
ShaderResource::ShaderResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device) : device_{device} {}

void ShaderResource::CreateShaders(const ShaderModulesCreateInfo& createInfo)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    for (const auto& [name, asset]: createInfo.modules) {
        const auto shaderModule = devicePtr->CreateShaderModule(asset.data);
        if (!shaderModule) {
            throw std::runtime_error("Failed to create vertex shader module!");
        }
        shaderModules_[name] = shaderModule;
    }
}

void ShaderResource::DeleteShaderModule(const std::string& moduleName) { shaderModules_.erase(moduleName); }
} // namespace common::vulkan_framework
