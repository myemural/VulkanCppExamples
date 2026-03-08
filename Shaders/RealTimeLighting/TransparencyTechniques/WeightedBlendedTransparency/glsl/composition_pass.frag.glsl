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

layout(set = 0, binding = 0) uniform sampler2D accumTex;
layout(set = 0, binding = 1) uniform sampler2D revealTex;

void main()
{
    // Get values from samplers
    vec4 accum = texture(accumTex, fragUv);
    float reveal = texture(revealTex, fragUv).r;

    // Calculate color and alpha
    vec3 backgroundColor = vec3(0.0); /// TODO: Can be taken from CPU or opaque pass.
    vec3 color = accum.rgb / max(accum.a, 0.0001);
    float alpha = 1.0 - reveal;
    vec3 finalColor = color * alpha + backgroundColor * (1.0 - alpha);
    outColor = vec4(finalColor, 1.0);
}