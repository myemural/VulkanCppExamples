#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;
layout(set = 0, binding = 1) uniform sampler2D gPosition;
layout(set = 0, binding = 2) uniform sampler2D uDofImage; // rgb = blurred color, a = near field coverage

layout(push_constant) uniform CompositePushConstants {
    float exposure;             // Manual exposure in EV
    float focusDistance;        // Focus distance in meters (view-space units)
    float cocScale;             // CoC radius (in half-res pixels) of an object at infinity
    float maxCocRadius;         // Maximum CoC radius in half-res pixels
    uint isDofEnabled;          // 0: DoF disabled, 1: DoF enabled
    uint debugView;             // 0: Final image, 1: CoC view
} pc;

const uint DEBUG_VIEW_FINAL = 0U;
const uint DEBUG_VIEW_COC = 1U;

// Pixels without geometry (cleared G-Buffer) are treated as very far away
const float kBackgroundDepth = 10000.0;

// ACES Filmic (Narkowicz fit)
// Reference: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tonemapAces(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

float linearDepth(vec4 positionView)
{
    // gPosition keeps the view-space position, w = 0 means there is no geometry
    return positionView.w > 0.0 ? max(-positionView.z, 1e-3) : kBackgroundDepth;
}

// Thin lens CoC: c = (f^2 / (N * (5 - f))) * (z - S) / z
// The constant part is calculated on CPU (cocScale), so the CoC is "cocScale * (1 - S / z)"
float calculateCoc(float depth)
{
    float coc = pc.cocScale * (1.0 - pc.focusDistance / depth);
    return clamp(coc, -pc.maxCocRadius, pc.maxCocRadius);
}

void main()
{
    vec2 resolution = vec2(textureSize(gLightPassOutput, 0));
    vec2 uv = gl_FragCoord.xy / resolution;

    vec3 sceneColor = texture(gLightPassOutput, uv).rgb;
    float coc = calculateCoc(linearDepth(texelFetch(gPosition, ivec2(gl_FragCoord.xy), 0)));

    if (pc.debugView == DEBUG_VIEW_COC) {
        // Grayscale scene, Far Field: Orange, Near Field: Blue (intensity shows the CoC size)
        vec3 gray= vec3(dot(tonemapAces(sceneColor * exp2(pc.exposure)), vec3(0.2126, 0.7152, 0.722)));
        vec3 cocColor = coc > 0.0 ? vec3(1.0, 0.45, 0.45) : vec3(0.05, 0.4, 1.0);
        outColor = vec4(mix(gray * 0.3, cocColor, clamp(abs(coc) / pc.maxCocRadius, 0.0, 1.0)), 1.0);
        return;
    }

    vec3 hdrColor = sceneColor;
    if (pc.isDofEnabled != 0U) {
        vec4 dof = texture(uDofImage, uv);

        // Pixel's own blur amount (far and near field), transition is in [1, 2] half-res pixels CoC radius
        float ownAlpha = smoothstep(1.0, 2.0, abs(coc));

        // Union of the own blur amount and the near field which spreads over this pixel
        float alpha = ownAlpha + dof.a - ownAlpha * dof.a;
        hdrColor = mix(sceneColor, dof.rgb, alpha);
    }

    // Manual exposure control
    hdrColor *= exp2(pc.exposure);

    // Apply ACES tonemapping (HDR to LDR)
    outColor = vec4(tonemapAces(hdrColor), 1.0);
}
