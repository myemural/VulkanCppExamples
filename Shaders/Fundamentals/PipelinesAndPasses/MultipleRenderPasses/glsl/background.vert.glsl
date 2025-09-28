#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main()
{
    fragUV = inUV;
    gl_Position = vec4(inPosition, 1.0);
}