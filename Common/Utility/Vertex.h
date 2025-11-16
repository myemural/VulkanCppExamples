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

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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

// Vertex Layouts

struct VertexPos2
{
    Attribute<glm::vec2, 0> Position; // layout(location=0) in vec2 position;
};

struct VertexPos2Color3
{
    Attribute<glm::vec2, 0> Position; // layout(location=0) in vec2 position;
    Attribute<glm::vec3, 1> Color;    // layout(location=1) in vec3 color;
};

struct VertexPos2Uv2
{
    Attribute<glm::vec2, 0> Position; // layout(location=0) in vec2 position;
    Attribute<glm::vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

struct VertexPos3
{
    Attribute<glm::vec3, 0> Position; // layout(location=0) in vec3 position;
};

struct VertexPos3Uv2
{
    Attribute<glm::vec3, 0> Position; // layout(location=0) in vec3 position;
    Attribute<glm::vec2, 1> Uv;       // layout(location=1) in vec2 texCoord;
};

} // namespace common::utility
