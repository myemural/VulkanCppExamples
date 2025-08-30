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
#include <vector>

#include "stb_image.h"

namespace common::utility
{

struct TextureHandler
{
    unsigned char* Data = nullptr;
    std::uint32_t Width = UINT32_MAX;
    std::uint32_t Height = UINT32_MAX;
    std::uint32_t Channels = UINT32_MAX;

    [[nodiscard]] std::uint32_t GetByteSize() const
    {
        return Width * Height * (Channels + 1);
    }

    void Clear()
    {
        stbi_image_free(Data);
        Data = nullptr;
        Width = UINT32_MAX;
        Height = UINT32_MAX;
        Channels = UINT32_MAX;
    }
};

class TextureLoader
{
public:
    static void SetBasePath(const std::string& path);

    static TextureHandler Load(const std::string& path);

private:
    static std::string basePath_;
};
} // namespace common::utility