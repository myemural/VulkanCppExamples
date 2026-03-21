/**
 * @file    ApplicationData.h
 * @brief   This header file keeps user-provided application data (vertices, indices etc.).
 * @author  Mustafa Yemural (myemural)
 * @date    15.02.2026
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <glm/glm.hpp>

#include "MathUtils.h"

namespace examples::real_time_lighting::lighting_architectures::clustered_forward_unlimited
{

#define NUM_OBJECTS 500
#define MAX_LIGHT_COUNT 150
#define TILE_SIZE_X 16
#define TILE_SIZE_Y 16
#define Z_SLICE_COUNT 16

inline const std::vector enabledMaterialComponents{common::scene::MaterialComponent::DIFFUSE_COLOR_VEC4,
                                                   common::scene::MaterialComponent::DIFFUSE_MAP_TEXTURE,
                                                   common::scene::MaterialComponent::NORMAL_MAP_TEXTURE};

struct alignas(16) PointLightData
{
    glm::vec4 lightPositionIntensity; // xyz = Light Position (View-Space), w = Light Intensity
    glm::vec4 lightColorRadius;       // xyz = Light Color, w = Radius
};

struct alignas(16) ClusterHeader
{
    uint32_t offset;
    uint32_t count;
};

struct ComputePipelinePushConstants
{
    glm::mat4 proj;                // Projection Matrix
    glm::vec4 screenSizeAndPlanes; // x = Width, y = Height, z = Near Plane, w = Far Plane
    std::uint32_t lightCount;      // Total light count in scene
};

struct ForwardPipelinePushConstants
{
    glm::mat4 view;
    glm::mat4 projection;
    std::uint32_t tilesX;
    std::uint32_t objectId;
    float nearPlane;
    float farPlane;
};

// Scene object and light position vectors
inline const std::vector<glm::vec3> modelPositions = common::utility::GenerateRandomPositions(
        NUM_OBJECTS, glm::vec3(-25.0f, -20.0f, -30.0f), glm::vec3(25.0f, 20.0f, -2.0f), 3.0f);

} // namespace examples::real_time_lighting::lighting_architectures::clustered_forward_unlimited
