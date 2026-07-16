#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) out vec2 fragUv;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const float SCALE = 4.0;

void main()
{
    vec3 basePos = gl_in[0].gl_Position.xyz;

    vec3 cameraToPoint = normalize(basePos - pc.cameraPosition.xyz);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(cameraToPoint, up));

    float width = SCALE * 0.35;
    float height = SCALE;

    // Bottom Left
    vec3 pos = basePos - right * width;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    fragUv = vec2(0.0, 1.0);
    EmitVertex();

    // Top Left
    pos = basePos - right * width + up * height;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    fragUv = vec2(0.0, 0.0);
    EmitVertex();

    // Bottom Right
    pos = basePos + right * width;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    fragUv = vec2(1.0, 1.0);
    EmitVertex();

    // Top Right
    pos = basePos + right * width + up * height;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    fragUv = vec2(1.0, 0.0);
    EmitVertex();

    EndPrimitive();
}