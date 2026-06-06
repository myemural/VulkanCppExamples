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

// HDR Equirectangular Texture
layout(set = 0, binding = 0) uniform sampler2D equirectangularEnvironment;

const float PI = 3.14159265359;

void main()
{
    vec3 dir = normalize(directionVector);
    dir.y = -dir.y; // Axis fix

    // Spherical UV conversion
    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = asin(clamp(dir.y, -1.0, 1.0)) / PI + 0.5;

    vec2 uv = vec2(u, v);

    outColor = texture(equirectangularEnvironment, uv);
}