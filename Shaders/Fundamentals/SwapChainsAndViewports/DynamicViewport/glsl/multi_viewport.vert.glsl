#version 450
#extension GL_ARB_shader_viewport_layer_array : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

layout(push_constant) uniform PushConstants {
    mat4 mvpMatrix;
    int viewportIndex;
} pc;

void main()
{
    gl_ViewportIndex = pc.viewportIndex;
    fragUV = inUV;
    gl_Position = pc.mvpMatrix * vec4(inPosition, 1.0);
}