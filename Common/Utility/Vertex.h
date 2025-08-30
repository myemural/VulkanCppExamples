/**
 * @file    Vertex.h
 * @brief   This file contains helper classes for storing vertex attributes.
 * @author  Mustafa Yemural (myemural)
 * @date    17.08.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */

#pragma once

namespace common::utility
{
/**
  * @brief Data holder template class for vertex attributes.
  * @tparam T Type of the attribute.
  * @tparam Loc Location of the attribute in the shader code.
  */
template<typename T, int Loc>
struct Attribute
{
    using Type = T;
    static constexpr std::uint32_t Location = Loc;
    static constexpr size_t Size = sizeof(T);
    static constexpr size_t Count = sizeof(T) / sizeof(std::remove_all_extents_t<T>);

    Type data;
};

// Vertex Attribute Types
struct Vec2
{
    float X, Y;
};

struct Vec3
{
    float X, Y, Z;
};

struct Vec4
{
    float X, Y, Z, W;
};

struct Color3
{
    float R, G, B;
};

struct Color4
{
    float R, G, B, A;
};
} // common::utility
