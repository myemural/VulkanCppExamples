/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ShaderResource.h"

namespace common::vulkan_framework
{
ShaderResource::ShaderResource(const std::shared_ptr<vulkan_wrapper::VulkanDevice>& device) : device_{device} {}

void ShaderResource::CreateShaders(const ShaderModulesCreateInfo& createInfo)
{
    const auto devicePtr = device_.lock();
    if (!devicePtr) {
        throw std::runtime_error("Device object not found!");
    }

    const std::string basePath = createInfo.BasePath;
    const utility::ShaderBaseType shaderType = createInfo.ShaderType;

    for (const auto& [name, fileName]: createInfo.Modules) {
        const utility::ShaderLoader shaderLoader{basePath, shaderType};
        const auto shaderCode = shaderLoader.LoadSpirV(fileName);
        const auto shaderModule = devicePtr->CreateShaderModule(shaderCode);
        if (!shaderModule) {
            throw std::runtime_error("Failed to create vertex shader module!");
        }
        shaderModules_[name] = shaderModule;
    }
}
} // namespace common::vulkan_framework
