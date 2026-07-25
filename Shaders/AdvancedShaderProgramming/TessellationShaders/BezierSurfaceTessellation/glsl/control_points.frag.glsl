#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform ControlPointsPushConstants {
    mat4 view;
    mat4 proj;
    vec4 pointColor;
    float pointSize;
} pc;

void main()
{
    outColor = pc.pointColor;
}
