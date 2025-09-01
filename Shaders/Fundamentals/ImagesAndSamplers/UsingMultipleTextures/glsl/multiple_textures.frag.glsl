#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in int samplerIdx;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler[2];

void main()
{
    vec4 color1 = texture(uCombinedSampler[0], fragUV); // First texture
    vec4 color2 = texture(uCombinedSampler[1], fragUV); // Second texture

    if (samplerIdx == 0) {
        outColor = color1;
    } else if  (samplerIdx == 1) {
        outColor = color2;
    } else {
        outColor = mix(color1, color2, 0.4); // %60 first, %40 second
    }
}