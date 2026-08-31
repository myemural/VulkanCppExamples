#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8) in;

struct BvhNode
{
    vec3 boundsMin;
    uint leftFirst;     // For Inner node: Left child index, For Leaf: First triangle index
    vec3 boundsMax;
    uint triangleCount; // 0 marks an inner node
};

struct Triangle
{
    vec4 v0; // xyz = Position, w = Owning Object ID (bit pattern)
    vec4 v1;
    vec4 v2;
    vec4 n0;
    vec4 n1;
    vec4 n2;
};

struct MeshMaterialData
{
    vec4 albedoColor; // rgb = Albedo, a = Emission Strength
    float roughness;
    float metallic;
    float reflectivity;
    float pad0_;
};

layout(set = 0, binding = 0, rgba16f) uniform writeonly image2D outRadiance;
layout(std430, set = 0, binding = 1) readonly buffer BvhNodes { BvhNode bvhNodes[]; };
layout(std430, set = 0, binding = 2) readonly buffer Triangles { Triangle triangles[]; };
layout(std430, set = 0, binding = 3) readonly buffer MaterialData { MeshMaterialData materialData[]; };

layout(std140, set = 0, binding = 4) uniform LightBlock
{
    vec4 lightPosition; // xyz = Area Light Center, w = Half Size
    vec4 lightColor;    // xyz = Light Color, w = Intensity
} light;

layout(set = 0, binding = 5) uniform samplerCube skybox;

layout(push_constant) uniform RayTracePushConstants {
    mat4 invViewProj;
    vec4 cameraPosition;
    uvec4 frameInfo; // xy = Resolution, z = Frame Index
} pc;

struct HitInfo
{
    float t;
    vec3 normal;
    uint objectId;
};

const float kEpsilon = 1e-4;
const float kRayOffset = 1e-3;
const float kInfinitiy = 1e30;
const float kSkyAmbient = 0.06;

vec3 safeInverseDirection(vec3 direction)
{
    vec3 magnitude = max(abs(direction), vec3(1e-8));
    vec3 signs = mix(vec3(1.0), vec3(-1.0), lessThan(direction, vec3(0.0)));
    return signs / magnitude;
}

bool intersectAABB(vec3 origin, vec3 invDirection, vec3 boundsMin, vec3 boundsMax, float tMax)
{
    vec3 t0 = (boundsMin - origin) * invDirection;
    vec3 t1 = (boundsMax - origin) * invDirection;
    vec3 tNear = min(t0, t1);
    vec3 tFar = max(t0, t1);

    float tClosest = max(max(tNear.x, tNear.y), max(tNear.z, 0.0));
    float tFarthest = min(min(tFar.x, tFar.y), min(tFar.z, tMax));

    return tClosest <= tFarthest;
}

// Moller-Trumbore, double-sided so the winding of the primitives doesn't matter
bool intersectTriangle(vec3 origin, vec3 direction, uint index, out float t, out vec2 barycentric)
{
    vec3 edge1 = triangles[index].v1.xyz - triangles[index].v0.xyz;
    vec3 edge2 = triangles[index].v2.xyz - triangles[index].v0.xyz;

    vec3 pv = cross(direction, edge2);
    float determinant = dot(edge1, pv);
    if (abs(determinant) < 1e-9) {
        return false; // Ray is parallel to the triangle
    }

    float invDeterminant = 1.0 / determinant;
    vec3 tv = origin - triangles[index].v0.xyz;
    float u = dot(tv, pv) * invDeterminant;
    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 qv = cross(tv, edge1);
    float v = dot(direction, qv) * invDeterminant;
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    t = dot(edge2, qv) * invDeterminant;
    barycentric = vec2(u, v);

    return t > kEpsilon;
}

// Stops at the first occluder, used for the shadow rays
bool traceAnyHit(vec3 origin, vec3 direction, float tMax)
{
    vec3 invDirection = safeInverseDirection(direction);
    uint stack[32];
    int stackPointer = 0;
    stack[stackPointer++] = 0U;

    while (stackPointer > 0) {
        BvhNode node = bvhNodes[stack[--stackPointer]];
        if (!intersectAABB(origin, invDirection, node.boundsMin, node.boundsMax, tMax)) {
            continue;
        }

        if (node.triangleCount > 0U) {
            // Check for intersection with triangles in the leaf node
            for (uint i = 0; i < node.triangleCount; ++i) {
                float t;
                vec2 barycentric;
                if (intersectTriangle(origin, direction, node.leftFirst + i, t, barycentric) && t < tMax) {
                    return true; // Found an occluder
                }
            }
        } else if (stackPointer < 30) {
            // Push child nodes onto the stack
            stack[stackPointer++] = node.leftFirst;
            stack[stackPointer++] = node.leftFirst + 1U;
        }
    }

    return false; // No occluder found
}

bool traceClosestHit(vec3 origin, vec3 direction, float tMax, out HitInfo hit)
{
    vec3 invDirection = safeInverseDirection(direction);
    uint stack[32];
    int stackPointer = 0;
    stack[stackPointer++] = 0U;

    hit.t = tMax;
    hit.normal = vec3(0.0, 1.0, 0.0);
    hit.objectId = 0U;

    bool found = false;

    while (stackPointer > 0) {
        BvhNode node = bvhNodes[stack[--stackPointer]];
        if (!intersectAABB(origin, invDirection, node.boundsMin, node.boundsMax, hit.t)) {
            continue;
        }

        if (node.triangleCount > 0U) {
            // Check for intersection with triangles in the leaf node
            for (uint i = 0; i < node.triangleCount; ++i) {
                uint index = node.leftFirst + i;
                float t;
                vec2 barycentric;
                if (intersectTriangle(origin, direction, index, t, barycentric) && t < hit.t) {
                    hit.t = t;
                    hit.objectId = floatBitsToUint(triangles[index].v0.w);
                    hit.normal = normalize(triangles[index].n0.xyz * (1.0 - barycentric.x - barycentric.y) +
                                           triangles[index].n1.xyz * barycentric.x +
                                           triangles[index].n2.xyz * barycentric.y);
                    found = true;
                }
            }
        } else if (stackPointer < 30) {
            // Push child nodes onto the stack
            stack[stackPointer++] = node.leftFirst;
            stack[stackPointer++] = node.leftFirst + 1U;
        }
    }

    return found; // Return whether a hit was found
}

vec3 sampleSky(vec3 direction)
{
    return texture(skybox, vec3(direction.x, direction.y, -direction.z)).rgb;
}

// Fixed sample pattern on the area light, this keeps the soft shadows stable while the camera moves
vec3 directLighting(vec3 position, vec3 normal, vec3 viewDirection, MeshMaterialData material, int sampleCount, float softness)
{
    const vec2 offsets[4] = vec2[4](vec2(-0.7, -0.7), vec2(0.7, -0.7), vec2(-0.7, 0.7), vec2(0.7, 0.7));
    float shininess = mix(320.0, 8.0, material.roughness);
    vec3 result = vec3(0.0);

    for (int i = 0; i < sampleCount; ++i) {
        vec3 lightPoint = light.lightPosition.xyz + vec3(offsets[i].x, 0.0, offsets[i].y) * light.lightPosition.w * softness;
        vec3 toLight = lightPoint - position;
        float distanceToLight = length(toLight);
        vec3 lightDirection = toLight / distanceToLight;

        float NdotL = dot(normal, lightDirection);
        if (NdotL <= 0.0) {
            continue;
        }

        if (traceAnyHit(position + normal * kRayOffset, lightDirection, distanceToLight - kRayOffset)) {
            continue;
        }

        vec3 halfVector = normalize(lightDirection + viewDirection);
        float specular = pow(max(dot(normal, halfVector), 0.0), shininess) * (1.0 - material.roughness);
        float attenuation = light.lightColor.a / (distanceToLight * distanceToLight);

        result += (material.albedoColor.rgb + vec3(specular)) * NdotL * attenuation;
    }

    return result * light.lightColor.rgb / float(sampleCount);
}

vec3 shadeSurface(vec3 position, vec3 normal, vec3 viewDirection, MeshMaterialData material, int lightSamples, float softness)
{
    vec3 emission = material.albedoColor.rgb * material.albedoColor.a;
    vec3 ambient = material.albedoColor.rgb * sampleSky(normal) * kSkyAmbient;

    return emission + ambient + directLighting(position, normal, viewDirection, material, lightSamples, softness);
}

void main()
{
    uvec2 pixel = gl_GlobalInvocationID.xy;
    if (pixel.x >= pc.frameInfo.x || pixel.y >= pc.frameInfo.y) {
        return;
    }

    // Primary ray is generated by unprojecting the pixel through the inverse view-proj matrix
    vec2 ndc = ((vec2(pixel) + 0.5) / vec2(pc.frameInfo.xy)) * 2.0 - 1.0;
    vec4 nearPoint = pc.invViewProj * vec4(ndc, 0.0, 1.0);
    vec4 farPoint = pc.invViewProj * vec4(ndc, 1.0, 1.0);
    vec3 origin = pc.cameraPosition.xyz;
    vec3 direction = normalize(farPoint.xyz / farPoint.w - nearPoint.xyz / nearPoint.w);

    vec3 color;
    HitInfo hit;

    if (traceClosestHit(origin, direction, kInfinitiy, hit)) {
        MeshMaterialData material = materialData[hit.objectId];
        vec3 position = origin + direction * hit.t;
        vec3 normal = faceforward(hit.normal, direction, hit.normal);
        vec3 viewDirection = -direction;

        color = shadeSurface(position, normal, viewDirection, material, 4, 1.0);

        // A single specular bounce is enough to get mirrors and the skybox reflection
        float reflectance = clamp(max(material.reflectivity, material.metallic) * (1.0 - material.roughness), 0.0, 1.0);

        if (reflectance > 0.01) {
            vec3 bounceOrigin = position + normal * kRayOffset;
            vec3 bounceDirection = reflect(direction, normal);
            vec3 reflectedColor;

            HitInfo bounceHit;
            if (traceClosestHit(bounceOrigin, bounceDirection, kInfinitiy, bounceHit)) {
                MeshMaterialData bounceMaterial = materialData[bounceHit.objectId];
                vec3 bouncePosition = bounceOrigin + bounceDirection * bounceHit.t;
                vec3 bounceNormal = faceforward(bounceHit.normal, bounceDirection, bounceHit.normal);
                reflectedColor = shadeSurface(bouncePosition, bounceNormal, -bounceDirection, bounceMaterial, 1, 0.0);
            } else {
                reflectedColor = sampleSky(bounceDirection);
            }

            // Schlick style blend, metals tint the reflection with their own albedo
            float fresnel = reflectance + (1.0 - reflectance) * pow(1.0 - max(dot(normal, viewDirection), 0.0), 5.0);
            vec3 tint = mix(vec3(1.0), material.albedoColor.rgb, material.metallic);
            color = mix(color, reflectedColor + tint, fresnel);
        }
    } else {
        color = sampleSky(direction);
    }

    // Reinhard tone mapping
    color = color / (color + vec3(1.0));

    imageStore(outRadiance, ivec2(pixel), vec4(color, 1.0));
}
