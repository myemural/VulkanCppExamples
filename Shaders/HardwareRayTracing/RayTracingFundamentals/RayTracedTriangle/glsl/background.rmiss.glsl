#version 460
#extension GL_EXT_ray_tracing : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) rayPayloadInEXT vec3 payloadColor;

layout(set = 0, binding = 2) uniform SceneUbo
{
    mat4 viewInverse;
    mat4 projInverse;
    vec4 missColor;
} scene;

void main()
{
    payloadColor = scene.missColor.rgb;
}
