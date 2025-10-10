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

layout(set = 0, binding = 0) uniform sampler2D uOffscreenTex;

void main()
{
    float border = 0.01; // Border width
    if((fragUV.x < border) || (fragUV.x > 1.0 - border) || (fragUV.y < border) || (fragUV.y > 1.0 - border)) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Border color
    } else {
        outColor = texture(uOffscreenTex, fragUV);
    }

}