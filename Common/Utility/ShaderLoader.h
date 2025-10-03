/**
 * @file    ShaderLoader.h
 * @brief   This file contains utility class for loading shaders from file.
 * @author  Mustafa Yemural (myemural)
 * @date    16.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include <vector>

namespace common::utility
{
enum class ShaderBaseType
{
    GLSL,
    HLSL
};

class ShaderLoader
{
public:
    /**
     * @param rootPath Root path.
     * @param shaderBaseType Specifies which shading language (GLSL, HLSL etc.) the .spv file was converted from.
     */
    ShaderLoader(const std::string& rootPath, const ShaderBaseType& shaderBaseType);

    ~ShaderLoader() = default;

    /**
     * @brief Loads .spv file from local into the vector.
     * @param fileName Only contains file name of the .spv file ("example.spv" etc.)
     * @return Returns .spv file as vector of bytes.
     */
    [[nodiscard]] std::vector<std::uint32_t> LoadSpirV(const std::string& fileName) const;

private:
    [[nodiscard]] std::string GenerateBasePath(const std::string& rootPath) const;

    std::string basePath_;
    ShaderBaseType shaderType_;
};
} // namespace common::utility
