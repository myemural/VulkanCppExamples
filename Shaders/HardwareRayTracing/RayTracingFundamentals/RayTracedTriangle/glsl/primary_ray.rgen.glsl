#version 460
#extension GL_EXT_ray_tracing : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) rayPayloadEXT vec3 payloadColor;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2D outRadiance;
layout(set = 0, binding = 2) uniform SceneUbo
{
    mat4 viewInverse;
    mat4 projInverse;
    vec4 missColor;
} scene;

const float kRayTMin = 0.001;
const float kRayTMax = 10000.0;

void main()
{
    // Pixel center in normalized device coordinates ([-1, 1], y is down because of the flipped projection)
    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 ndc = pixelCenter / vec2(gl_LaunchSizeEXT.xy) * 2.0 - 1.0;

    // Camera position and the direction through the pixel in world space
    const vec4 origin = scene.viewInverse * vec4(0.0, 0.0, 0.0, 1.0);
    const vec4 target = scene.projInverse * vec4(ndc, 1.0, 1.0);
    const vec4 direction = scene.viewInverse * vec4(normalize(target.xyz / target.w), 0.0);

    payloadColor = vec3(0.0);

    // SBT record offset = 0, SBT record stride = 0, miss index = 0, payload location = 0
    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, origin.xyz, kRayTMin, direction.xyz, kRayTMax, 0);

    imageStore(outRadiance, ivec2(gl_LaunchIDEXT.xy), vec4(payloadColor, 1.0));
}
