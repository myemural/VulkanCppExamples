#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;
layout(location = 1) in flat int fragViewportIndex;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler;

void main()
{
    vec4 texColor = texture(uCombinedSampler, fragUV);
    float blendFactor = 0.4;

    if (fragViewportIndex == 0) { // Top-left
        outColor = mix(texColor, vec4(0.3, 0.0, 0.0, 1.0), blendFactor);
    } else if (fragViewportIndex == 1) { // Top-right
        outColor = mix(texColor, vec4(0.0, 0.3, 0.0, 1.0), blendFactor);
    } else if (fragViewportIndex == 2) { // Bottom-left
        outColor = mix(texColor, vec4(0.0, 0.0, 0.3, 1.0), blendFactor);
    } else { // Bottom-right
        outColor = mix(texColor, vec4(0.3, 0.3, 0.0, 1.0), blendFactor);
    }
}