#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;
layout(location = 0) in float lifetime;

void main()
{
    float red = clamp(lifetime * 2.0, 0.0, 1.0);
    vec3 baseColor = vec3(1.0 - red, 0.0, 0.0);
    float alpha = clamp(lifetime, 0.0, 1.0);

    outColor = vec4(baseColor, alpha);
}