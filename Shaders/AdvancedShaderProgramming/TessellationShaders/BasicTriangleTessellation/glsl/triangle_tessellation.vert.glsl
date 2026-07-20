#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

vec2 positions[3] = vec2[](
        vec2(0.0, -0.75),  // Top
        vec2(0.75, 0.75),  // Right-bottom
        vec2(-0.75, 0.75)  // Left-bottom
);

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
