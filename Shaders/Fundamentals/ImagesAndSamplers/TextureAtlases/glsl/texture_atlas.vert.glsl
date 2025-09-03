#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;

layout(push_constant) uniform Push {
    vec4 uvRect; // u0,v0,du,dv
    vec2 offset;
} pc;

layout(location = 0) out vec2 fragUV;

void main()
{
    vec2 pos = inPosition + pc.offset;
    gl_Position = vec4(pos, 0.0, 1.0);
    fragUV = pc.uvRect.xy + inUV * pc.uvRect.zw;
}