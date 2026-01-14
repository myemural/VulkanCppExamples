#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 directionVector;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

layout(set = 0, binding = 0) uniform samplerCube skybox;

void main()
{
    vec3 dir = normalize(directionVector);
    dir = vec3(dir.x, dir.y, -dir.z); // For flipping front and back
    outColor = texture(skybox, dir);
}