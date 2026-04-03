/**
 * @file    BuiltinPrimitives.h
 * @brief   Contains classes and helpers which related to built-in primitive submeshes.
 * @author  Mustafa Yemural (myemural)
 * @date    03.04.2026
 *
 * Copyright (c) 2026 Mustafa Yemural - www.mustafayemural.com
 * Released under the MIT License
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "SceneMesh.h"

namespace common::scene
{

enum class BuiltinMeshType
{
    CUBE,
    SPHERE,
    CONE,
    CYLINDER,
    PLANE
};

std::string GetBuiltinMeshName(const BuiltinMeshType& builtinMeshType);

class BuiltinPrimitive
{
public:
    virtual ~BuiltinPrimitive() = default;

    [[nodiscard]] MeshPrimitive GetMeshPrimitive() const;

protected:
    virtual void CreateMeshPrimitive();

    MeshPrimitive mesh_;
    std::vector<glm::vec3> positions_;
    std::vector<glm::vec2> uvs_;
    std::vector<glm::vec3> normals_;
    std::vector<glm::vec4> tangents_;
    std::vector<std::uint16_t> indices_;
};

class CubePrimitive : public BuiltinPrimitive
{
public:
    explicit CubePrimitive(float size);

    static constexpr auto kCubeMeshName = "builtin_cube";

private:
    static std::vector<glm::vec3> CreateCubePositions(float size);

    static std::vector<glm::vec2> CreateCubeUVs();

    static std::vector<glm::vec3> CreateCubeNormals();

    static std::vector<glm::vec4> CreateCubeTangents();

    static std::vector<std::uint16_t> CreateCubeIndices();
};

class SpherePrimitive : public BuiltinPrimitive
{
public:
    explicit SpherePrimitive(float size, std::uint32_t stackCount, std::uint32_t sectorCount);

    static constexpr auto kSphereMeshName = "builtin_sphere";

private:
    static std::vector<glm::vec3>
    CreateSpherePositions(float size, std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec2> CreateSphereUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec3> CreateSphereNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec4> CreateSphereTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<std::uint16_t> CreateSphereIndices(std::uint32_t stackCount, std::uint32_t sectorCount);
};

class ConePrimitive : public BuiltinPrimitive
{
public:
    explicit ConePrimitive(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

    static constexpr auto kConeMeshName = "builtin_cone";

private:
    static std::vector<glm::vec3>
    CreateConePositions(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec2> CreateConeUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec3> CreateConeNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec4> CreateConeTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<std::uint16_t> CreateConeIndices(std::uint32_t stackCount, std::uint32_t sectorCount);
};

class CylinderPrimitive : public BuiltinPrimitive
{
public:
    explicit CylinderPrimitive(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

    static constexpr auto kCylinderMeshName = "builtin_cylinder";

private:
    static std::vector<glm::vec3>
    CreateCylinderPositions(float height, std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec2> CreateCylinderUVs(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec3> CreateCylinderNormals(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<glm::vec4> CreateCylinderTangents(std::uint32_t stackCount, std::uint32_t sectorCount);

    static std::vector<std::uint16_t> CreateCylinderIndices(std::uint32_t stackCount, std::uint32_t sectorCount);
};

class PlanePrimitive : public BuiltinPrimitive
{
public:
    explicit PlanePrimitive(float size);

    static constexpr auto kPlaneMeshName = "builtin_plane";

private:
    static std::vector<glm::vec3> CreatePlanePositions(float size);

    static std::vector<glm::vec2> CreatePlaneUVs();

    static std::vector<glm::vec3> CreatePlaneNormals();

    static std::vector<glm::vec4> CreatePlaneTangents();

    static std::vector<std::uint16_t> CreatePlaneIndices();
};

} // namespace common::scene
