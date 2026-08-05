#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(std430, set = 0, binding = 0) readonly buffer FrustumCorners { vec4 corners[8]; };

layout(push_constant) uniform LinePushConstants {
    mat4 viewProj; // Debug camera's view * projection matrix
} pc;

layout(location = 0) out vec3 outColor;

// Corner layout
// 0: Near-left-bottom, 1: Near-right-bottom, 2: Near-left-top, 3: Near-right-top,
// 4: Far-left-bottom,  5: Far-right-bottom,  6: Far-left-top,  7: Far-right-top
const int kEdgeIndices[24] = int[24](
        0, 1,  1, 3,  3, 2,  2, 0, // Near plane rectangle
        4, 5,  5, 7,  7, 6,  6, 4, // Far plane rectangle
        0, 4,  1, 5,  2, 6,  3, 7  // Connecting edges
);

void main()
{
    const int cornerIndex = kEdgeIndices[gl_VertexIndex];
    gl_Position = pc.viewProj * vec4(corners[cornerIndex].xyz, 1.0);
    outColor = vec3(1.0, 0.85, 0.1); // Give a constant color to the frustum wireframe
}
