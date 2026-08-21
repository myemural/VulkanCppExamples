#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec2 fragOffset;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in float fragIntensity;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D screenTex;

void main()
{
    const float radiusSqr = dot(fragOffset, fragOffset);

    // Cut the quad down to a disc
    if (radiusSqr > 1.0) {
        discard;
    }

    const float halo = exp(-4.5 * radiusSqr); // Wide, soft falloff
    const float core = pow(max(0.0, 1.0 - radiusSqr), 6.0); // Tight, bright center

    const vec3 color = fragColor * (halo * 0.75 + core * 0.9) * fragIntensity;

    outColor = vec4(color, halo);
}
