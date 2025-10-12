#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants {
    mat4 mvpMatrix;
} pc;

void main()
{
    gl_Position = pc.mvpMatrix * vec4(inPosition, 1.0);
}