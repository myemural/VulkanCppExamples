#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uSourceImage;

layout(push_constant) uniform BloomPassPushConstants {
    vec2 srcTexelSize;        // 1.0 / source image size
    vec2 dstTexelSize;        // 1.0 / target mip size
    uint useKarisAverage;     // Only for the first downsample pass (firefly suppression)
    float filterRadius;       // Only for upsample passes (tent filter radius in source texels)
    float anamorphicStretch;  // Only for upsample passes (horizontal multiplier of the tent filter)
} pc;

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;
    vec2 t = pc.srcTexelSize * pc.filterRadius * vec2(pc.anamorphicStretch, 1.0);

    // 3x3 tent filter
    vec3 result = texture(uSourceImage, uv).rgb * 4.0;
    result += (texture(uSourceImage, uv + vec2(0.0, t.y)).rgb + texture(uSourceImage, uv - vec2(0.0, t.y)).rgb +
               texture(uSourceImage, uv + vec2(t.x, 0.0)).rgb + texture(uSourceImage, uv - vec2(t.x, 0.0)).rgb) * 2.0;
    result += texture(uSourceImage, uv + t).rgb + texture(uSourceImage, uv - t).rgb +
              texture(uSourceImage, uv + vec2(t.x, -t.y)).rgb + texture(uSourceImage, uv + vec2(-t.x, t.y)).rgb;
    result *= 1.0 / 16.0;

    outColor = vec4(result, 1.0);
}
