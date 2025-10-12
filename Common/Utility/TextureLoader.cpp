/**
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#include "TextureLoader.h"

#include <stdexcept>

#include "stb_image.h"

namespace common::utility
{

TextureLoader::TextureLoader(const std::string& basePath) : basePath_{basePath} {}

TextureHandler TextureLoader::Load(const std::string& path, const TextureChannelFormat& channelFormat) const
{
    int width, height, channels;

    const std::string fileFullPath = basePath_ + path;

    int desiredChannels;
    size_t actualChannelCount = SIZE_MAX;
    switch (channelFormat) {
        case TextureChannelFormat::RGB:
            desiredChannels = STBI_rgb;
            actualChannelCount = 3;
            break;
        case TextureChannelFormat::RGBA:
            actualChannelCount = 4;
        default:
            desiredChannels = STBI_rgb_alpha;
    }

    unsigned char* data = stbi_load(fileFullPath.c_str(), &width, &height, &channels, desiredChannels);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + fileFullPath);
    }

    const size_t dataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * actualChannelCount;
    const std::vector imageData(data, data + dataSize);

    return {imageData, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height),
            static_cast<std::uint32_t>(channels), channelFormat};
}

} // namespace common::utility
