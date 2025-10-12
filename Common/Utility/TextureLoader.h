/**
 * @file    TextureLoader.h
 * @brief   This file contains utility class for loading texture data from file.
 * @author  Mustafa Yemural (myemural)
 * @date    29.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <string>

#include "TextureHandler.h"

namespace common::utility
{

class TextureLoader
{
public:
    /**
     * @param basePath Base path of the texture.
     */
    explicit TextureLoader(const std::string& basePath);

    ~TextureLoader() = default;

    /**
     * @brief Loads the texture from file.
     * @param path Relative path to the base path of the texture file.
     * @param channelFormat Specifies channel format (RGB, RGBA etc.) of the texture file.
     * @return Returns texture handler object that holds real texture data and its info.
     */
    [[nodiscard]] TextureHandler Load(const std::string& path,
                                      const TextureChannelFormat& channelFormat = TextureChannelFormat::RGBA) const;

private:
    std::string basePath_;
};
} // namespace common::utility
