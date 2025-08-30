#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

void main()
{
    fragUV = inUV;
    gl_Position = vec4(inPosition, 0.0, 1.0);
}