/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "ShaderLoader.h"

#include <fstream>

namespace common::asset_manager
{
namespace
{
    constexpr auto kGlslSpirvOutPath = "glsl/spirv/";
    constexpr auto kHlslSpirvOutPath = "hlsl/spirv/";
    constexpr auto kSlangSpirvOutPath = "slang/spirv/";
} // namespace

ShaderLoader::ShaderLoader(const std::filesystem::path& basePath, std::string shaderType)
    : shaderType_(std::move(shaderType))
{
    basePath_ = GenerateBasePath(basePath);
}

std::unique_ptr<ShaderAsset> ShaderLoader::Load(const std::string& relativePath)
{
    const std::filesystem::path fullPath{basePath_ / relativePath};

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

    std::vector<std::uint32_t> output(fileSizeInBytes / sizeof(std::uint32_t));

    std::ifstream file(fullPath, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("SPIRV file couldn't be opened: " + fullPath.string());
    }

    file.read(reinterpret_cast<char*>(output.data()), static_cast<std::streamsize>(fileSizeInBytes));

    if (!file || file.gcount() != static_cast<std::streamsize>(fileSizeInBytes)) {
        throw std::runtime_error("SPIRV file couldn't be read fully: " + fullPath.string());
    }

    return std::make_unique<ShaderAsset>(std::move(output));
}

std::filesystem::path ShaderLoader::GenerateBasePath(const std::filesystem::path& rootPath) const
{
    if (shaderType_ == "GLSL") {
        return rootPath / kGlslSpirvOutPath;
    } else if (shaderType_ == "HLSL") {
        return rootPath / kHlslSpirvOutPath;
    } else if (shaderType_ == "SLANG") {
        return rootPath / kSlangSpirvOutPath;
    }

    throw std::runtime_error("Shader type is wrong!");
}

} // namespace common::asset_manager
