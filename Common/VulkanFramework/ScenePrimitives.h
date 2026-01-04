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
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace common::vulkan_framework
{

// For cubes
std::vector<glm::vec3> CreateCubePositions(float size);

std::vector<glm::vec2> CreateCubeUVs();

std::vector<glm::vec3> CreateCubeNormals();

std::vector<glm::vec3> CreateCubeTangents();

std::vector<std::uint16_t> CreateCubeIndices();

// For spheres
std::vector<glm::vec3> CreateSpherePositions(float size, int stackCount, int sectorCount);

std::vector<glm::vec2> CreateSphereUVs(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateSphereNormals(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateSphereTangents(int stackCount, int sectorCount);

std::vector<std::uint16_t> CreateSphereIndices(int stackCount, int sectorCount);

// For cones
std::vector<glm::vec3> CreateConePositions(float height, int stackCount, int sectorCount);

std::vector<glm::vec2> CreateConeUVs(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateConeNormals(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateConeTangents(int stackCount, int sectorCount);

std::vector<uint16_t> CreateConeIndices(int stackCount, int sectorCount);

// For cylinders
std::vector<glm::vec3> CreateCylinderPositions(float height, int stackCount, int sectorCount);

std::vector<glm::vec2> CreateCylinderUVs(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateCylinderNormals(int stackCount, int sectorCount);

std::vector<glm::vec3> CreateCylinderTangents(int stackCount, int sectorCount);

std::vector<uint16_t> CreateCylinderIndices(int stackCount, int sectorCount);

// For planes
std::vector<glm::vec3> CreatePlanePositions(float size);

std::vector<glm::vec2> CreatePlaneUVs();

std::vector<glm::vec3> CreatePlaneNormals();

std::vector<glm::vec3> CreatePlaneTangents();

std::vector<std::uint16_t> CreatePlaneIndices();

} // namespace common::vulkan_framework
