/**
 * @file    SceneMesh.h
 * @brief   Mesh data structure for built-in mesh. This is temporary solution, it can be changed later.
 * @author  Mustafa Yemural (myemural)
 * @date    02.04.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "CoreDefines.h"
#include "SceneConfig.h"

namespace common::scene
{

enum class ComponentType
{
    SIGNED_BYTE,
    UNSIGNED_BYTE,
    SIGNED_SHORT,
    UNSIGNED_SHORT,
    UNSIGNED_INT,
    FLOAT
};

enum class DataType
{
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4
};

struct COMMON_API BufferView
{
    const unsigned char* data = nullptr;
    size_t bufferLength = SIZE_MAX;
    size_t byteOffset = SIZE_MAX;
    size_t byteLength = SIZE_MAX;
    size_t byteStride = SIZE_MAX;
};

struct COMMON_API Accessor
{
    BufferView bufferView{};
    size_t byteOffset = SIZE_MAX;
    ComponentType componentType = ComponentType::FLOAT;
    DataType type = DataType::VEC4;
    size_t count = SIZE_MAX;
};

struct COMMON_API MeshPrimitive
{
    std::string name;
    std::unordered_map<AttributeType, Accessor> attributes;
    Accessor indices{};
};

} // namespace common::scene
