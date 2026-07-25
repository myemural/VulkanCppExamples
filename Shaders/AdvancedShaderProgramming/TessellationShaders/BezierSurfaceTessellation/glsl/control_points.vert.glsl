#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform ControlPointsPushConstants {
    mat4 view;
    mat4 proj;
    vec4 pointColor;
    float pointSize;
} pc;

void main()
{
    gl_Position = pc.proj * pc.view * vec4(inPosition, 1.0);
    gl_PointSize = pc.pointSize;
}
