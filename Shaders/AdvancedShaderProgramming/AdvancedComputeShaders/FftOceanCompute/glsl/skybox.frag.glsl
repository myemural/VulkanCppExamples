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

layout(set = 0, binding = 4) uniform samplerCube skybox;

const float kExposure = 1.6;

void main()
{
    vec3 dir = normalize(directionVector);
    dir = vec3(dir.x, dir.y, -dir.z); // For flipping front and back

    vec3 color = texture(skybox, dir).rgb;
    color = vec3(1.0) - exp(-color * kExposure);

    outColor = vec4(color, 1.0);
}
