#version 450

layout(location = 0) in vec2 inPosition;

layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
} ubo;

void main()
{
    vec4 pos = vec4(inPosition, 0.0, 1.0);
    gl_Position = ubo.model * pos;
}
