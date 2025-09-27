#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler;

void main()
{
    outColor = texture(uCombinedSampler, fragUV);
}