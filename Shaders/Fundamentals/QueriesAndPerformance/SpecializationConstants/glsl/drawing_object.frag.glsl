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

// 0 = Flat color (blue)
// 1 = Texture
// 2 = Mixture of color (blue) and texture
layout(constant_id = 0) const int MODE = 0;

layout(set = 0, binding = 1) uniform sampler2D uCombinedSampler;

void main()
{
    if (MODE == 0) {
        outColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
    else if (MODE == 1) {
        outColor = texture(uCombinedSampler, fragUV);
    }
    else if (MODE == 2) {
        vec4 textureColor = texture(uCombinedSampler, fragUV);
        outColor = mix(vec4(0.0, 0.0, 1.0, 1.0), textureColor, 0.5);
    }

}