/**
 * @file    ShaderLoader.h
 * @brief   Contains shader asset loader class implementation.
 * @author  Mustafa Yemural (myemural)
 * @date    07.02.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <filesystem>

#include "AssetLoader.h"
#include "CoreDefines.h"
#include "ShaderAsset.h"

namespace common::asset_manager
{

class COMMON_API ShaderLoader : public AssetLoader<ShaderAsset>
{
public:
    /**
     * @param basePath Base path of the shaders.
     * @param shaderType Shaders type: GLSL, HLSL or Slang.
     */
    explicit ShaderLoader(const std::filesystem::path& basePath, std::string shaderType);

    /**
     * @brief Loads shader from file and returns it as an asset.
     * @param relativePath Relative path of the shader file.
     * @return Shader asset.
     */
    std::unique_ptr<ShaderAsset> Load(const std::string& relativePath) override;

private:
    [[nodiscard]] std::filesystem::path GenerateBasePath(const std::filesystem::path& rootPath) const;

    std::filesystem::path basePath_;
    std::string shaderType_;
};

} // namespace common::asset_manager
