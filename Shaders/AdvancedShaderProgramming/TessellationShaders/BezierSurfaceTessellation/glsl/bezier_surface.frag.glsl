#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUv;

const vec3 kCorner00 = vec3(0.10, 0.35, 0.90); // u=0, v=0 (blue)
const vec3 kCorner10 = vec3(0.95, 0.25, 0.25); // u=1, v=0 (red)
const vec3 kCorner01 = vec3(0.20, 0.85, 0.35); // u=0, v=1 (green)
const vec3 kCorner11 = vec3(0.95, 0.85, 0.15); // u=1, v=1 (yellow)

void main()
{
    // Bilinear blend of the four corner colors across the surface
    vec3 colorAtV0 = mix(kCorner00, kCorner10, fragUv.x);
    vec3 colorAtV1 = mix(kCorner01, kCorner11, fragUv.x);
    vec3 finalColor = mix(colorAtV0, colorAtV1, fragUv.y);

    outColor = vec4(finalColor, 1.0);
}
