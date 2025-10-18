#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec2 fragUV;

layout(push_constant) uniform PushConstants {
    mat4 mvpMatrix;
} pc;

void main()
{
    gl_Position = pc.mvpMatrix * vec4(inPosition, 1.0);

    int vertexIndex = gl_VertexIndex % 4;

    // Manual texture coordinates for quad
    switch (vertexIndex) {
        case 0:
            fragUV = vec2(0.0, 0.0);
            break;
        case 1:
            fragUV = vec2(0.0, 1.0);
            break;
        case 2:
            fragUV = vec2(1.0, 0.0);
            break;
        case 3:
            fragUV = vec2(1.0, 1.0);
            break;
    }
}