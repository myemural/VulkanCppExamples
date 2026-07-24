#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform MeshPushConstants {
    vec4 shapeColor; // xyz = Color for control point or Bézier curve
    float controlPointSize;
    float tessLevel;
} pc;

void main()
{
    outColor = vec4(pc.shapeColor.xyz, 1.0);
}