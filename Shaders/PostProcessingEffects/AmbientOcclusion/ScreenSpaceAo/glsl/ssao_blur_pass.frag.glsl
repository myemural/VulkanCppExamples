#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out float outOcclusion;

layout(set = 0, binding = 0) uniform sampler2D uOcclusion;

void main()
{
    // Box blur with the size of the noise texture to remove the tiling pattern
    const int blurRadius = 2;

    vec2 texelSize = 1.0 / vec2(textureSize(uOcclusion, 0));
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(uOcclusion, 0));

    float result = 0.0;
    for (int x = -blurRadius; x < blurRadius; ++x) {
        for (int y = -blurRadius; y < blurRadius; ++y) {
            result += texture(uOcclusion, uv + vec2(x, y) * texelSize).r;
        }
    }

    outOcclusion = result / float(4 * blurRadius * blurRadius);
}
