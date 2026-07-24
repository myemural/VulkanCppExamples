#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(isolines, equal_spacing, cw) in;

vec3 cubicBezier(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t)
{
    float u = 1.0 - t;
    return u * u * u * p0 + 3.0 * u * u * t * p1 + 3.0 * u * t * t * p2 + t * t * t * p3;
}

void main()
{
    float t = gl_TessCoord.x;
    vec3 pos = cubicBezier(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz,
                           gl_in[3].gl_Position.xyz, t);
    gl_Position = vec4(pos, 1.0);
}
