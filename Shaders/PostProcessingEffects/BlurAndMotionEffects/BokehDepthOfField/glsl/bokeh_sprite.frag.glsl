#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 inLocalPosition;
layout(location = 1) flat in vec3 inColor;
layout(location = 2) flat in float inRadius;
layout(location = 3) flat in float inCoc;

layout(set = 0, binding = 1) uniform sampler2D uGatherSourceImage; // rgb = Color, a = Signed CoC

layout(push_constant) uniform BokehPassPushConstants {
    vec2 texelSize;   // 1.0 / half-res image size
    uint bladeCount;  // Aperture blade count (0: Circular)
    float bladeAngle; // Rotation of the aperture polygon in radians
} pc;

const float PI = 3.14159265359;

// Signed distance (in pixels, positive inside) to the edge of the aperture shape
float apertureEdgeDistance(vec2 position, float radius)
{
    float distanceToCenter = length(position);
    if (pc.bladeCount < 3U) {
        return radius - distanceToCenter;
    }

    // Regular polygon: Find the angle relative to the center of the nearest edge
    float segmentAngle = 2.0 * PI / float(pc.bladeCount);
    float angle = atan(position.y, position.x) - pc.bladeAngle;
    float angleToEdgeCenter = mod(angle, segmentAngle) - 0.5 * segmentAngle;

    // Distance from the polygon center to the edge (inradius) minus distance of the point along the edge normal
    return radius * cos(0.5 * segmentAngle) - distanceToCenter * cos(angleToEdgeCenter);
}

void main()
{
    // Anti-alised aperture shape
    float shapeMask = clamp(apertureEdgeDistance(inLocalPosition, inRadius) + 0.5, 0.0, 1.0);

    // Occlusion: The sprite is hidden where the target pixel is in front of it (smaller signed CoC)
    // So, far field bokeh doesn't cover in-focus objects, but near field bokeh covers everything behind it
    float targetCoc = texelFetch(uGatherSourceImage, ivec2(gl_FragCoord.xy), 0).a;
    float margin = max(1.0, 0.25 * inRadius);
    float visibility = clamp((targetCoc - inCoc) / margin + 1.0, 0.0, 1.0);

    outColor = vec4(inColor * shapeMask * visibility, 1.0);
}
