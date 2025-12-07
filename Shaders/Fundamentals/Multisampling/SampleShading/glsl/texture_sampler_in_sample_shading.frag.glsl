#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler;

void main()
{
    if (gl_SampleID == 0) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        outColor = texture(uCombinedSampler, fragUV);
    }
}