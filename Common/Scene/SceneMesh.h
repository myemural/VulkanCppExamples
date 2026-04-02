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
#include <vector>

#include <glm/glm.hpp>

#include "CoreDefines.h"

namespace common::scene
{

struct COMMON_API Mesh
{
    std::string name;
    std::uint32_t vertexCount = UINT32_MAX;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;
    std::vector<glm::vec2> texCoords0;
    std::vector<glm::vec2> texCoords1;
    std::vector<glm::vec4> colors0;
    std::vector<glm::vec4> colors1;
    std::vector<std::uint16_t> indices;
};

} // namespace common::scene
