#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform MeshPushConstants {
    vec4 shapeColor; // xyz = Color for control point or Bézier curve
    float controlPointSize;
    float tessLevel;
} pc;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    gl_PointSize = pc.controlPointSize;
}
