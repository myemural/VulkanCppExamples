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
std::uint32_t TextureHandler::GetByteSize() const
{
    switch (Format) {
        case TextureChannelFormat::RGB:
            return Width * Height * 3;
        case TextureChannelFormat::RGBA:
        default:
            return Width * Height * 4;
    }
}

void TextureHandler::Clear() noexcept
{
    stbi_image_free(Data);
    Data = nullptr;
    Width = UINT32_MAX;
    Height = UINT32_MAX;
    Channels = UINT32_MAX;
}

TextureLoader::TextureLoader(const std::string& basePath) : basePath_{basePath} {}

TextureHandler TextureLoader::Load(const std::string& path, const TextureChannelFormat& channelFormat) const
{
    int width, height, channels;

    const std::string fileFullPath = basePath_ + path;

    int desiredChannels;
    switch (channelFormat) {
        case TextureChannelFormat::RGB:
            desiredChannels = STBI_rgb;
            break;
        case TextureChannelFormat::RGBA:
        default:
            desiredChannels = STBI_rgb_alpha;
    }

    unsigned char* data = stbi_load(fileFullPath.c_str(), &width, &height, &channels, desiredChannels);
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + fileFullPath);
    }

    return {data, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height),
            static_cast<std::uint32_t>(channels), channelFormat};
}

} // namespace common::utility
