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

const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

float luminance(vec3 color)
{
    return dot(color, kLumaRec709);
}

// Karis average: Bright samples get smaller weights, so a single very bright pixel can't dominate the whole block
float karisWeight(vec3 color)
{
    return 1.0 / (1.0 + luminance(color));
}

void main()
{
    vec2 uv = gl_FragCoord.xy * pc.dstTexelSize;
    vec2 t = pc.srcTexelSize;

    // 13 bilinear taps (offsets in source texels)
    vec3 a = texture(uSourceImage, uv + t * vec2(-2.0,  2.0)).rgb;
    vec3 b = texture(uSourceImage, uv + t * vec2( 0.0,  2.0)).rgb;
    vec3 c = texture(uSourceImage, uv + t * vec2( 2.0,  2.0)).rgb;
    vec3 d = texture(uSourceImage, uv + t * vec2(-2.0,  0.0)).rgb;
    vec3 e = texture(uSourceImage, uv).rgb;
    vec3 f = texture(uSourceImage, uv + t * vec2( 2.0,  0.0)).rgb;
    vec3 g = texture(uSourceImage, uv + t * vec2(-2.0, -2.0)).rgb;
    vec3 h = texture(uSourceImage, uv + t * vec2( 0.0, -2.0)).rgb;
    vec3 i = texture(uSourceImage, uv + t * vec2( 2.0, -2.0)).rgb;
    vec3 j = texture(uSourceImage, uv + t * vec2(-1.0,  1.0)).rgb;
    vec3 k = texture(uSourceImage, uv + t * vec2( 1.0,  1.0)).rgb;
    vec3 l = texture(uSourceImage, uv + t * vec2(-1.0, -1.0)).rgb;
    vec3 m = texture(uSourceImage, uv + t * vec2( 1.0, -1.0)).rgb;

    // The filter is a weighted use of 5 overlapping 2x2 boxes: 4 outer boxes (0.125 each) and the center box (0.5)
    vec3 boxes[5] = vec3[](
            (a + b + d + e) * 0.25,
            (b + c + e + f) * 0.25,
            (d + e + g + h) * 0.25,
            (e + f + h + i) * 0.25,
            (j + k + l + m) * 0.25
    );
    float weights[5] = float[](0.125, 0.125, 0.125, 0.125, 0.5);

    if (pc.useKarisAverage != 0U) {
        for (int n = 0; n < 5; ++n) {
            weights[n] *= karisWeight(boxes[n]);
        }
    }

    vec3 result = vec3(0.0);
    float weightSum = 0.0;
    for (int n = 0; n < 5; ++n) {
        result += boxes[n] * weights[n];
        weightSum += weights[n];
    }

    // Normalization is only needed with Karis average (weights sum to 1.0 otherwise)
    result /= weightSum;

    outColor = vec4(max(result, vec3(0.0)), 1.0);
}
