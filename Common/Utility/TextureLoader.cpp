/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "TextureLoader.h"

#include <stdexcept>

namespace common::utility
{

std::string TextureLoader::basePath_;

void TextureLoader::SetBasePath(const std::string &path)
{
    basePath_ = path;
}

TextureHandler TextureLoader::Load(const std::string &path)
{
    int width, height, channels;

    const std::string fileFullPath = basePath_ + path;
    // stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(fileFullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + fileFullPath);
    }

    return {
        data,
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height),
        static_cast<std::uint32_t>(channels)
    };
}

} // namespace common::utility