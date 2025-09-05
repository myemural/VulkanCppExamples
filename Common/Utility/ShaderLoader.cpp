/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ShaderLoader.h"

#include <fstream>
#include <filesystem>

namespace common::utility
{
namespace
{
    constexpr auto kGlslSpirvOutPath = "glsl/spirv/";
    constexpr auto kHlslSpirvOutPath = "hlsl/spirv/";
}

ShaderLoader::ShaderLoader(const std::string &rootPath, const ShaderBaseType &shaderBaseType)
    : shaderType_{shaderBaseType}
{
    basePath_ = GenerateBasePath(rootPath);
}

std::vector<std::uint32_t> ShaderLoader::LoadSpirV(const std::string &fileName) const
{
    const std::filesystem::path fullPath{basePath_ + fileName};

    if (!std::filesystem::exists(fullPath)) {
        throw std::runtime_error("SPIRV binary file not found: " + fullPath.string());
    }

    const uintmax_t fileSizeInBytes = std::filesystem::file_size(fullPath);
    if (fileSizeInBytes % sizeof(uint32_t) != 0) {
        throw std::runtime_error("SPIRV file is not dividable by 4: " + fullPath.string());
    }

    if (fileSizeInBytes > std::numeric_limits<std::streamsize>::max()) {
        throw std::runtime_error("SPIRV file is too big: " + fullPath.string());
    }

    std::vector<uint32_t> output(fileSizeInBytes / sizeof(uint32_t));

    std::ifstream file(fullPath, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("SPIRV file couldn't be opened: " + fullPath.string());
    }

    file.read(reinterpret_cast<char *>(output.data()), static_cast<std::streamsize>(fileSizeInBytes));

    if (!file || file.gcount() != static_cast<std::streamsize>(fileSizeInBytes)) {
        throw std::runtime_error("SPIRV file couldn't be read fully: " + fullPath.string());
    }

    return output;
}

std::string ShaderLoader::GenerateBasePath(const std::string &rootPath) const
{
    std::string intermediatePath;

    switch (shaderType_) {
        case ShaderBaseType::GLSL:
            intermediatePath = kGlslSpirvOutPath;
            break;
        case ShaderBaseType::HLSL:
            intermediatePath = kHlslSpirvOutPath;
            break;
        default:
            throw std::runtime_error("Shader type is a wrong value!");
    }

    return rootPath + intermediatePath;
}
} // namespace common::utility
