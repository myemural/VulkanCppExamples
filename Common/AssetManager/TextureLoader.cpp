/**
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "TextureLoader.h"

#include <utility>

#include "stb_image.h"

namespace common::asset_manager
{

TextureLoader::TextureLoader(std::filesystem::path basePath) : basePath_(std::move(basePath)) {}

std::unique_ptr<TextureAsset> TextureLoader::Load(const std::string& relativePath)
{
    int width, height, channels;

    const std::string fileFullPath = (basePath_ / relativePath).string();

    unsigned char* data = stbi_load(fileFullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + fileFullPath);
    }

    const size_t dataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    const std::vector imageData(data, data + dataSize);

    return std::make_unique<TextureAsset>(imageData, width, height, channels);
}

} // namespace common::asset_manager
