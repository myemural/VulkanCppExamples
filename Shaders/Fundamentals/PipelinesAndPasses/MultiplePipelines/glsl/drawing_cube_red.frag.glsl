#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler;

void main()
{
    vec4 texColor = texture(uCombinedSampler, fragUV);
    vec4 targetRed = vec4(1.0, 0.0, 0.0, texColor.a);
    float blend = 0.5;
    outColor = mix(texColor, targetRed, blend);
}