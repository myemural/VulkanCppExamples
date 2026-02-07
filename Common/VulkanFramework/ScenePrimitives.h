/**
 * @file    ScenePrimitives.h
 * @brief   Contains helper functions for primitives that used in SceneManager.
 * @author  Mustafa Yemural (myemural)
 * @date    12.12.2025
 *
 * Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "GltfModelHandler.h"

namespace common::vulkan_framework
{

// For cubes
std::vector<glm::vec3> CreateCubePositions(float size);

std::vector<glm::vec2> CreateCubeUVs();

std::vector<glm::vec3> CreateCubeNormals();

std::vector<glm::vec4> CreateCubeTangents();

std::vector<std::uint16_t> CreateCubeIndices();

inline constexpr auto kCubeMeshName = "builtin_cube";
utility::GltfMesh CreateCubeMesh(float size);

// For spheres
std::vector<glm::vec3> CreateSpherePositions(float size, std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec2> CreateSphereUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec3> CreateSphereNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec4> CreateSphereTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<std::uint16_t> CreateSphereIndices(std::uint32_t stackCount, std::uint32_t sectorCount);

inline constexpr auto kSphereMeshName = "builtin_sphere";
utility::GltfMesh CreateSphereMesh(std::uint32_t stackCount, std::uint32_t sectorCount);

// For cones
std::vector<glm::vec3> CreateConePositions(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec2> CreateConeUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec3> CreateConeNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec4> CreateConeTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<uint16_t> CreateConeIndices(std::uint32_t stackCount, std::uint32_t sectorCount);

inline constexpr auto kConeMeshName = "builtin_cone";
utility::GltfMesh CreateConeMesh(std::uint32_t stackCount, std::uint32_t sectorCount);

// For cylinders
std::vector<glm::vec3> CreateCylinderPositions(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec2> CreateCylinderUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec3> CreateCylinderNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<glm::vec4> CreateCylinderTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

std::vector<uint16_t> CreateCylinderIndices(std::uint32_t stackCount, std::uint32_t sectorCount);

inline constexpr auto kCylinderMeshName = "builtin_cylinder";
utility::GltfMesh CreateCylinderMesh(std::uint32_t stackCount, std::uint32_t sectorCount);

// For planes
std::vector<glm::vec3> CreatePlanePositions(float size);

std::vector<glm::vec2> CreatePlaneUVs();

std::vector<glm::vec3> CreatePlaneNormals();

std::vector<glm::vec4> CreatePlaneTangents();

std::vector<std::uint16_t> CreatePlaneIndices();

inline constexpr auto kPlaneMeshName = "builtin_plane";
utility::GltfMesh CreatePlaneMesh(float size);

enum class BuiltinMeshType
{
    CUBE,
    SPHERE,
    CONE,
    CYLINDER,
    PLANE
};

std::string GetBuiltinMeshName(const BuiltinMeshType& builtinMeshType);

} // namespace common::vulkan_framework
